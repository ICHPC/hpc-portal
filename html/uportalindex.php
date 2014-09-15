<?php

require '../uportal/smarty/libs/Smarty.class.php';
require '../uportal/uportal-config/uportal-functions2.inc';
require '../uportal/uportal-config/dspace-functions.inc';

$smarty = new Smarty;

$smarty->caching = false;
$smarty->cache_lifetime = 0;
$smarty->compile_check = true;
$smarty->debugging = false;
$smarty->force_compile = true;

session_start();

$menuitems =  array ();
$smarty->assign( "menulinks", $menuitems ) ;
$smarty->assign('error', null);

if( !empty( $_SESSION['gecos'] ) ) {
    # Set username in template
    $smarty->assign( "gecos", $_SESSION['gecos'] );
}

if( !empty( $_REQUEST['action'] ) ) {
    $action = strtolower( sanify( $_REQUEST['action'] ) );
}
else {
    $action = "default";
}



$processed = 0;
$display_index = 0;

switch( $action ) {

    case 'login':
        # username and password passed to authenticate() unsanified
        $password = $_REQUEST['password'];
        $gecos = authenticate( $_REQUEST['username'], $password  );
        # username now sanified
        $username = sanify( $_REQUEST['username'] );

        if( empty ( $gecos ) ) {
            $smarty->assign('error', "Bad credentials");
            $smarty->display('login.tpl');
            exit;
        }
        else {

            $_SESSION['username'] = $username;
            $_SESSION['gecos'] = sanify( $gecos );

            $smarty->assign( "gecos", $_SESSION['gecos'] );

            check_user_registration( $_SESSION['username'] );

            $uid = get_uid ( $_SESSION['username'] );
            if( empty ( $uid ) || !is_numeric( $uid ) ) {
                fatal_error( "Unable to log in. Too many DB connections, probably. " );
            }
            else {
                $_SESSION['uid'] = $uid;
            }

            $display_index = 1;
            $processed = 1;
        }


    break;
    case 'logout':
        session_destroy();
        $_SESSION = array();
        $smarty->assign( "gecos", null );
        $smarty->display('login.tpl');
        exit;
    break;

    case '':
        if ( empty( $_SESSION['username'] ) ){
            $smarty->display( 'login.tpl' );
        }
        else {
            $display_index=1;
        }
        $processed = 1;
    break;

}


if( empty( $_SESSION['uid'] ) ) {
    $smarty->display('login.tpl');
    exit;
}

# Set up menus

$menuitems =  array (
array( "name" => "Index", "url" => "?action=index" ) ,
array( "name" => "Projects", "url" => "?action=projects"),
array( "name" => "Job list", "url" => "?action=joblist"),
array( "name" => "New job", "url" => "?action=newjob" ),
array( "name" => "Pools", "url" => "?action=pools" ) ,
array( "name" => "Profile", "url" => "?action=profile" ),
array( "name" => "Publish", "url" => "?action=uploader" ),
array( "name" => "Help", "url" => "https://wiki.ch.ic.ac.uk/wiki/index.php?title=Mod:HPC" ),
);

#   if ( is_admin( $_SESSION['username'] ) ) {
        # Admin functions
//      $menuitems[] = array( "name" => "Pools", "url" => "?action=pools" ) ;
#   }

$smarty->assign( "menulinks", $menuitems ) ;


if( $display_index ) {
    $smarty->assign("motd", get_motd( $_SESSION['username'] ) );

    $smarty->assign( "admin_email", get_admin_email() );
    $smarty->assign( "admin_email_subject", empty($UP_config['admin_email_subject']) ? 'portal email' : $UP_config['admin_email_subject'] );
    $smarty->assign( "admin_name", empty($UP_config['admin_name']) ? 'the admin' : $UP_config['admin_name'] );
    $smarty->display( 'index.tpl' );
}

if ($processed ) { exit; }


# At this point, we are logged in.

if (empty( $_REQUEST['subaction'] ) ) { $_REQUEST['subaction']='default'; }

switch( $action ) {
    case 'publish':

        $subaction = strtolower( sanify( $_REQUEST['subaction'] ) );
        switch( $subaction ) {
            case 'publish':
            default:
                $jid = empty($_REQUEST['jid' ]) ? '' : (int) $_REQUEST['jid' ];
                if ( ! $jid || !is_int( $jid ) ) {
                    fatal_error( "Invalid job specified" );
                }
                $app_id = get_app_id_for_job( $jid );
                if ( !check_job_owner( $_SESSION['uid'], $jid ) ) {
                    fatal_error( "You do not own this job" );
                }
                if ( !is_publishable( $app_id, $jid ) || is_published( $jid ) ) {
                    fatal_error( "You cannot publish this job" );
                }

                $profile = get_profile( $_SESSION['uid'] );
print "<!-- ";
print( $_SESSION['username'] );
print_r( $profile );
print "-->";
                if( $profile['pub_dspace'] === "checked" ) {
print "<!-- pub to dspace\n-->";
                    $handle = dspace_publish( $app_id, $jid, $_SESSION['uid'] );
                }
                if( $profile['pub_chempound'] === "checked" ) {
print "<!-- pub to chempound\n-->";
                    $url    = chempound_publish( $app_id, $jid, $_SESSION['uid'] );
                }
                if( $profile['pub_figshare'] === "checked" ) {
print "<!-- pub to figshare\n-->";
                    $handle2 = figshare_publish( $app_id, $jid, $_SESSION['uid'] );
                }
                if( !($profile['pub_dspace'] === "checked") && !($profile['pub_chempound'] === "checked" ) && !( $profile['pub_figshare'] === "checked" ) ) {
                    fatal_error( "You have no publication methods enabled. Fix this in your profile." );
                }
print "<!--";
print "<p>DSPACE: $handle\n";
print "<p>Figshare: $handle2\n";
print "<p>Chempound: $url\n";
print "-->\n";
if(1) {
                if( empty($handle) && empty($handle2) && empty($url) ) {
                    fatal_error( "Unable to publish. Please send the job ID to " . get_admin_email() );
                }
                else {
                    $proto = $UP_config['protocol'];
                    $host  = $_SERVER['HTTP_HOST'];
                    $uri   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');
                    $extra = '?action=joblist';
                    header("Location: $proto://$host$uri/$extra");
                }
}

        }
    break;

    case 'delete':
        $jid = empty($_REQUEST['jid' ]) ? '' : (int) $_REQUEST['jid' ];
        if ( ! $jid || !is_int( $jid ) ) {
            fatal_error( "Invalid job specified" );
        }


        if ( !check_job_owner( $_SESSION['uid'], $jid ) ) {
            fatal_error( "You do not own this job" );
        }


        if( ! delete_job( $jid ) ) {
            fatal_error( "Cannot delete job" );
        }
        else {

            $proto = $UP_config['protocol'];
            $host  = $_SERVER['HTTP_HOST'];
            $uri   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');
            $extra = '?action=joblist';
            header("Location: $proto://$host$uri/$extra");
            exit;
        }

    break;
    case 'figsharepub':
        $jid = empty($_REQUEST['jid' ]) ? '' : (int) $_REQUEST['jid' ];
        if ( ! $jid || !is_int( $jid ) ) {
            fatal_error( "Invalid job specified" );
        }

        if ( !check_job_owner( $_SESSION['uid'], $jid ) ) {
            fatal_error( "You do not own this job" );
        }

        figshare_make_public(  $_SESSION['uid'], $jid );

    break;
    case 'joblist':
        if( !isset( $_SESSION['page'] )) { $_SESSION['page']=0; }
        if( !isset( $_SESSION['items_per_page'] )) { $_SESSION['items_per_page']=10; }

        if( isset($_REQUEST['page']) ) {
            $r_page = sanify( $_REQUEST['page'] );
            $page = $_SESSION['page'];
            switch( $r_page ) {
                case 'prev':
                    $page--; if($page<0) { $page=0; }
                break;
                case 'next':
                    $page++;
                break;
            }
            $_SESSION['page']= $page;
        }
        $page = $_SESSION['page'];
        $items_per_page = $_SESSION['items_per_page'];

        switch( strtolower( sanify( $_REQUEST['subaction'] ) ) ) {
        default:
        #get a big hash containing all job states.

        if( isset($_REQUEST['orderdir']) ) { $orderdir  = $_SESSION['orderdir']   = (int) $_REQUEST['orderdir']; }
        if( isset($_REQUEST['orderby']) )  { $orderby   = $_SESSION['orderby']    = (int) $_REQUEST['orderby']; }
        if( isset($_REQUEST['byproject']) ){ $projectid = $_SESSION['orderby_project_id'] = (int) $_REQUEST['byproject']; }
        if( isset($_REQUEST['filter']) ){ $projectid = $_SESSION['filter'] = sanify( $_REQUEST['filter'] ); }

        $orderdir = ( isset($_SESSION['orderdir']) ? $_SESSION['orderdir'] : '' );
        $orderby  = ( isset($_SESSION['orderby']) ? $_SESSION['orderby'] : '' );
        $projectid = ( isset($_SESSION['orderby_project_id']) ? $_SESSION['orderby_project_id'] : '' ); # -1;
        $filter    = ( isset($_SESSION['filter']) ? $_SESSION['filter'] : '' );

        if ( !isset($projectid) || !is_int(  $projectid )  ) { $projectid = $_SESSION['orderby_project_id'] = -1; }
        # untaint
        if( !isset( $orderby ) ||  !is_int( $orderby  ) ) { $orderby = $_SESSION['orderby']  = 0; }
        if( !isset($orderdir)  ||  !is_int( $orderdir ) ) { $orderdir= $_SESSION['orderdir'] = 0; }

        $projectname = get_project_name( $_SESSION['uid'], $projectid );
        $smarty->assign( "projectname", $projectname );

            if ( $orderdir==0 ) { $orderdir=1;}
            else {$orderdir=0;}

        $job_list=array();
        $page++;
        while( sizeof($job_list) == 0 && $page>0 ) {
            $page--;
            $_SESSION['page']=$page;
            $job_list = new_get_job_list( $_SESSION['username'] , $orderby, $items_per_page, $orderdir, $projectid, $page * $items_per_page, $filter );
        }

        $smarty->assign( "job_list", $job_list );
        $smarty->assign( "orderdir", $orderdir );
        $smarty->assign( "orderby", $orderby );
        $smarty->assign( "byproject", $projectid );
        $smarty->assign( "defaultfilter", $filter );
        $smarty->assign( "defaultprojectid", $projectid );
        if( $projectid!=-1 ) {
            $smarty->assign( "defaultprojectname" , $projectname );
        }
        else {
            $smarty->assign( "defaultprojectname" , "-- All --" );
        }

        if( $page==0 ) { $smarty->assign( "suppress_prev", 1 ); };
        if( sizeof($job_list) < $items_per_page ) { $smarty->assign( "suppress_next", 1 ); }

        $b=get_projects( $_SESSION['uid'] );
        $b['description'][] = "-- All --" ;
        $b['project_id'][] = -1 ;
        if ( !empty ( $b ) ) {
            $smarty->assign( "projects", $b['description'] );
            $smarty->assign( "project_idx", $b['project_id'] );
        }
        $smarty->display('joblist.tpl');
        }

    break;

    case 'newjob':
        $subaction = strtolower( sanify( $_REQUEST['subaction'] ) );

        $pool_id = !isset($_REQUEST['pool' ]) ? '' : (int) $_REQUEST['pool' ];
        # we check $pool_id for emptiness below
        if ( $pool_id && !is_int( $pool_id ) ) {
            fatal_error( "Invalid pool specified $pool_id" );
        }
        if( $pool_id && !has_access_to_pool( $pool_id, $_SESSION['uid'] ) ) {
                fatal_error( "You may not access this pool" );
        }
        if( isset ( $_REQUEST['application'] ) ) {
            $app_id = (int) $_REQUEST['application'];
            if ( ! $app_id || !is_int( $app_id ) ) {
                fatal_error( "Invalid application specified" );
            }
            if(  !app_in_pool( $pool_id, $app_id ) ) {
                    fatal_error( "You may not access this application" );
            }

        }
        else {
            $app_id=-1;
        }
        if( isset( $_REQUEST['project'] ) ) {
            $project = (int) $_REQUEST['project'];
            if ( ! $project || !is_int( $project ) ) {
                fatal_error( "Invalid project specified" );
            }
            if( $project!=-1 &&  !owns_project( $_SESSION['uid'], $project ) ) {
                    fatal_error( "You may not access this project" );
            }

        }
        else {
            $project=-1;
        }



        switch( $subaction ) {
            case 'appinput':
                if (!isset( $pool_id ) ||  !isset($app_id) ) {
                    fatal_error( "You may not access this pool" );
                }

                $inf = app_input_file_description( $app_id );
                $smarty->assign( "app_input", $inf );
                $smarty->assign( "app", $app_id );
                $smarty->assign( "pool", $pool_id );
                $smarty->assign( "project", $project );
                $smarty->display( 'appinput.tpl');
            break;
            case 'selectapp':
                // display list of apps and projects that the user can seelct from

                if (!isset( $pool_id ) ||  !has_access_to_pool( $pool_id, $_SESSION['uid'] ) ) {
                    fatal_error("You may not access this pool");
                }

                $a=app_list( $pool_id );
                $b=get_projects( $_SESSION['uid'] );
                if( !empty ( $a ) ) {
                    $default_app_idx = isset($_SESSION['new_job_application']) ? $_SESSION['new_job_application'] : '';
                    $smarty->assign( "apps", $a['description'] );
                    $smarty->assign( "app_idx", $a['app_id'] );
                    $smarty->assign( "default_app_idx", $default_app_idx );
                }
                if( !empty ( $b ) ) {
                    $default_project_idx = isset($_SESSION['new_job_project']) ? $_SESSION['new_job_project'] : '';
                    $smarty->assign( "projects", $b['description'] );
                    $smarty->assign( "project_idx", $b['project_id'] );
                    $smarty->assign( "default_project_idx", $default_project_idx );
                }
                $smarty->assign( "pool", $pool_id );
                $smarty->display( 'selectapp.tpl');
            break;

            # get input files and start job
            case 'uploadinput':
                if ( $app_id == -1 ) {
                    fatal_error( "No application specified" );
                }

                # RECORD THE POOL , APPLICATION and PROJECT in the session for next time
                $_SESSION['new_job_pool']        = $pool_id;
                $_SESSION['new_job_application'] = $app_id;
                $_SESSION['new_job_project']     = $project;

                $description = sanify ( $_REQUEST['description'] );

                $success =  manage_job_invocation( $_SESSION['uid'], $app_id, $description, $project ) ;
                if ( $success=="" ) {
                    $smarty->display('staycalm.tpl' );
                }
                else {
                    fatal_error(  "Job start failed. ". $success  );
                }

            break;

            case 'selectpool':
            default:
                $default_pool_idx = ( isset($_SESSION['new_job_pool']) ? $_SESSION['new_job_pool'] : 0 );
                $pools = get_available_pools( $_SESSION['uid'] );
                $smarty->assign( "pools", $pools['description'] );
                $smarty->assign( "pool_idx", $pools['index'] );
                $smarty->assign( "default_pool_idx", $default_pool_idx );
                $smarty->display( 'selectpool.tpl');
            break;

        }


    break;


    case 'status':
        $time = date('l dS \of F Y h:i:s A');
        $smarty->assign( "time", $time );
        $smarty->assign( "status" , pool_status( $pool_id ) );
        $smarty->display('status.tpl');
    break;

    case 'inputdownload':
        $jid = !isset($_REQUEST['jid' ]) ? '' : (int) $_REQUEST['jid' ];
        if ( ! $jid || !is_int( $jid ) ) {
            fatal_error( "Invalid job specified" );
        }


        if ( !check_job_owner( $_SESSION['uid'], $jid ) ) {
            fatal_error( "You do not own this job" );
        }

        $idx = !isset($_REQUEST['inputfile' ]) ? '' : (int) $_REQUEST['inputfile' ];
        if ( !is_int( $idx ) ) {
            fatal_error(  "File not specified" );
        }

        switch( strtolower( sanify( $_REQUEST['subaction'] ) ) ) {
            case 'download':
                if ($idx==-1 ) {
                  return_job_tar_file(  $jid , "input");
                    exit;
                }
                else {
                  if( ! return_job_input_file( $jid, $idx ) ) {
                        fatal_error( "File does not exist" );
                    }
                }
            break;
            case 'preview':
                preview_file( $jid, $idx, $_SESSION['uid'] );
            break;
            default:
                # unhandled!
                fatal_error( "Action not specified" );
            break;
        }

    break;

    case 'outputdownload':
        $jid = !isset($_REQUEST['jid' ]) ? '' : (int) $_REQUEST['jid' ];
        if ( ! $jid || !is_int( $jid ) ) {
            fatal_error( "Invalid job specified" );
        }

        if ( !check_job_owner( $_SESSION['uid'], $jid ) ) {
            fatal_error( "You do not own this job");
        }

        $idx = !isset($_REQUEST['outputfile' ]) ? '' : (int) $_REQUEST['outputfile' ];
        if ( !is_int( $idx ) ) {
            fatal_error(  "File not specified" );
        }

        switch( strtolower( sanify( $_REQUEST['subaction'] ) ) ) {
            case 'download':
                if ($idx==-1 ) {
                  return_job_tar_file(  $jid );
                    exit;
                }
                else {
                  if( ! return_job_output_file( $jid, $idx, $_SESSION['uid'] ) ) {
                        fatal_error(  "File does not exist" );
                    }
                }
            break;
            case 'preview':
                    $out = preview_file( $jid, $idx, $_SESSION['uid'] );

                    $smarty->assign("content", $out[1] );
                    $smarty->display( "preview-" . $out[0] .".tpl" );
            break;
            default:
                # unhandled!
                fatal_error( "Action not specified" );
            break;
        }



    break;

    case 'projects':

        switch( strtolower( sanify( $_REQUEST['subaction'] ) ) ) {
        case 'add':
            $name = sanify( $_REQUEST['name'] );
            if ( ! $name ){
                fatal_error( 'Invalid name for project' );
            }
            add_project( $_SESSION['uid'], $name );
        break;

        case 'delete':
            $project_id = !isset($_REQUEST['project_id']) ? '' : (int) $_REQUEST['project_id'];
            if ( ! $project_id || !is_int( $project_id ) ) {
                fatal_error( "Invalid project specified" );
            }
            if ( ! is_project_empty( $project_id ) ) {
                fatal_error( "Project contains undeleted jobs" );
            }
            delete_project( $_SESSION['uid'] , $project_id );
        break;

        default:
        }

            $b = get_projects( $_SESSION['uid'] );
            if ( !empty( $b ) ) {
                $smarty->assign( "projects", $b['description'] );
                $smarty->assign( "project_idx", $b['project_id'] );
            }
            else {
                $smart->assign( "projects", '');
            }
            $smarty->display( "projects.tpl" );
    break;


    case 'acl':
        $pool_id = !isset($_REQUEST['pool' ]) ? '' : (int) $_REQUEST['pool' ];
        if ( ! $pool_id || !is_int( $pool_id ) ) {
            fatal_error( "Invalid pool specified" );
        }

        if( !owns_pool( $_SESSION['username'], $pool_id ) ) {
            fatal_error( "You do not own this pool" );
        }


        switch( strtolower( sanify( $_REQUEST['subaction'] ) ) ) {
            case 'set':
            break;
            default:
            $a = get_pool_acl( $pool_id );

            $smarty->assign( "acl", $a );
            $smarty->assign( "pool", $pool_id );
            $smarty->display( "acl.tpl" );
            exit;
        }
    break;

    case 'pools':
        switch( strtolower( sanify( $_REQUEST['subaction'] ) ) ) {

        case 'add':
            $error = "";
            if ( empty( $_REQUEST['user'] ) ) {
                fatal_error(  "User not specified" );
            }
            if ( empty( $_REQUEST['host'] ) ) {
                fatal_error(  "Host not specified" );
            }
            if ( empty( $_REQUEST['path'] ) ) {
                fatal_error(  "Path not specified" );
            }
            if ( empty( $_REQUEST['description'] ) ) {
                fatal_error(  "Description not specified" );
            }

            add_pool( $_SESSION['username'], sanify( $_REQUEST['user'] ), sanify( $_REQUEST['host'] ), sanify( $_REQUEST['path'] ), sanify( $_REQUEST['description'] ) );

            display_pools();


        break;

        case 'list':
            $pool_id = !isset($_REQUEST['pool' ]) ? '' : (int) $_REQUEST['pool' ];
            if ( ! $pool_id || !is_int( $pool_id ) ) {
                fatal_error( "Invalid pool specified" );
            }

            if( has_access_to_pool(  $pool_id, $_SESSION['uid'] ) ) {
                $d = get_pool_applications( $pool_id );


                if( owns_pool( $_SESSION['username'], $pool_id ) ) {
                    $smarty->assign( "mypool", true );
                }
                $smarty->assign( "apps", $d );
                $smarty->assign( "pool", $pool_id );
                $smarty->display('poolapps.tpl');
                exit;

            }
            else {
                fatal_error( "You do not own this pool" );
            }

        break;
        case 'key':
            $pool_id = !isset($_REQUEST['pool' ]) ? '' : (int) $_REQUEST['pool' ];
            if ( ! $pool_id || !is_int( $pool_id ) ) {
                fatal_error( "Invalid pool specified" );
            }
            if( owns_pool( $_SESSION['username'], $pool_id ) ) {
                get_key( $pool_id );
            }
            else {
                fatal_error( "You do not own this pool" );
            }
        break;
        case 'refresh':
            $pool_id = !isset($_REQUEST['pool' ]) ? '' : (int) $_REQUEST['pool' ];
            if ( ! $pool_id || !is_int( $pool_id ) ) {
                fatal_error( "Invalid pool specified" );
            }
            if( owns_pool( $_SESSION['username'], $pool_id ) ) {
                $resp = refresh_pool_applications( $pool_id );
                if( $resp==NULL ) {
                    $d = get_pool_applications( $pool_id );

                    $smarty->assign( "apps", $d );
                    $smarty->assign( "pool", $pool_id );
                    $smarty->display('poolapps.tpl');
                    exit;
                }
                else {
                    # Failed
                    fatal_error( $resp );
                }
            }
            else {
                fatal_error( "You do not own this pool" );
            }

        break;
        case 'delete':
            $pool_id = !isset($_REQUEST['pool' ]) ? '' : (int) $_REQUEST['pool' ];
            if ( ! $pool_id || !is_int( $pool_id ) ) {
                fatal_error( "Invalid pool specified" );
            }
            if( owns_pool( $_SESSION['username'], $pool_id ) ) {
                delete_pool( $_SESSION['username'], $pool_id );
            }
            else {
                fatal_error( "You do not own this pool" );
            }
            display_pools();


        break;
        case 'status':
            $pool_id = !isset($_REQUEST['pool' ]) ? '' : (int) $_REQUEST['pool' ];
            if ( ! $pool_id || !is_int( $pool_id ) ) {
                fatal_error( "Invalid pool specified" );
            }
            if( has_access_to_pool( $pool_id, $_SESSION['uid'] ) ) {
                $arr=pool_status( $pool_id );
                $time = date('l dS \of F Y h:i:s A');
                $smarty->assign( "time", $time );
                $smarty->assign( 'status', $arr );
                $smarty->display( "status.tpl" );
            }
            else {
                fatal_error(  "You do not own this pool" );
            }
        break;

        default:

            display_pools();
        }
    break;


    case 'uploader':
        if( NULL != figshare_has_credentials( $_SESSION['uid'] ) ) {
            $smarty->display( "figshare.tpl" );
        }
        else {
            if( array_key_exists( "upload", $_REQUEST ) )  {
                $project = !isset($_REQUEST['project']) ? '' : (int) $_REQUEST['project' ];
                if ( ! $project || !is_int( $project ) ) {
                    fatal_error( "Invalid project specified" );
                }
                figshare_uploader_upload( $_SESSION['uid'], $project );
            }
            else {
                $b=get_projects( $_SESSION['uid'] );
                while(list(,$v)=each($b['description'])) {
                    $c['description'][] = $v;
                }
                while(list(,$v)=each($b['project_id'])) {
                    $c['project_id'][] = $v;
                }


                    $smarty->assign( "projects", $c['description'] );
                    $smarty->assign( "project_idx", $c['project_id'] );

#               $smarty->assign( 'projects', $b );
                $smarty->display( "uploader.tpl" );
            }
        }
    break;

    case 'profile':
        switch( strtolower( sanify( $_REQUEST['subaction'] ) ) ) {
            case 'update':
                $pub_dspace = array_key_exists( "pub_dspace", $_REQUEST ) ? 1 : 0;
                $pub_chempound = array_key_exists( "pub_chempound", $_REQUEST ) ? 1 : 0 ;
                $pub_figshare = array_key_exists( "pub_figshare", $_REQUEST ) ? 1 : 0;

#print "<P>$pub_dspace";
#print "<P>$pub_chempound";
#print "<P>$pub_figshare";
                save_profile( $_SESSION['uid'], sanify( $_REQUEST['foaf'] ), sanify( $_REQUEST['embargo'] ),  $pub_dspace, $pub_chempound, $pub_figshare );
                display_profile();
            break;
            default:
                display_profile();
            break;
        }
    break;

    case 'editjob':
        $jid = !isset($_REQUEST['jid' ]) ? '' : (int) $_REQUEST['jid' ];
        if ( ! $jid || !is_int( $jid ) ) {
            fatal_error( "Invalid job specified" );
        }

        if ( !check_job_owner( $_SESSION['uid'], $jid ) ) {
            fatal_error( "You do not own this job" );
        }

        $project = !isset($_REQUEST['project' ]) ? '' : (int) $_REQUEST['project' ];
        if ( $project ) {
            if (!is_int( $project ) ) {
                fatal_error( "Invalid project specified" );
            }
            if( !check_project_owner( $_SESSION['uid'], $project ) ) {
                fatal_error( "You do not own this project" );
            }
            set_job_project( $jid, $project );
            $proto = $UP_config['protocol'];
            $host  = $_SERVER['HTTP_HOST'];
            $uri   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');
            $extra = '?action=joblist';
            header("Location: $proto://$host$uri/$extra");
            exit;
        }

        $app_id = get_app_id_for_job( $jid );
        $smarty->assign( "jid", $jid );
        # SJC description
		$smarty->assign( "description", get_job_description( $jid ) );

        $b=get_projects( $_SESSION['uid'] );


        $projectid   = get_project_by_jid( $jid );
        $projectname = get_project_name( $_SESSION['uid'], $projectid );



        $c['description'][] = $projectname;
        $c['project_id' ][] = $projectid;

        while(list(,$v)=each($b['description'])) {
            $c['description'][] = $v;
        }
        while(list(,$v)=each($b['project_id'])) {
            $c['project_id'][] = $v;
        }


        $smarty->assign( "projects", $c['description'] );
        $smarty->assign( "project_idx", $c['project_id'] );

        $smarty->display('editjob.tpl');

    break;

    default:
        if( !isset( $_SESSION['uid'] ) ) {
            $smarty->display('login.tpl');
        }
        else {
            $smarty->assign("motd", get_motd( $_SESSION['username'] ) );
            $smarty->assign( "admin_email", get_admin_email() );
            $smarty->assign( "admin_email_subject", empty($UP_config['admin_email_subject']) ? 'portal email' : $UP_config['admin_email_subject'] );
            $smarty->assign( "admin_name", empty($UP_config['admin_name']) ? 'the admin' : $UP_config['admin_name'] );
            $smarty->display('index.tpl');
        }
}

function display_profile() {
    global $smarty;
    $b = get_profile( $_SESSION['uid'] );


    $smarty->assign( "profile", $b );
    $smarty->display( "profile.tpl" );
}

function display_pools() {
    global $smarty;
            $b = get_pools( $_SESSION['username'] );
            $my_pool_b = false;
            $other_pool_b = false;
            if ( !empty( $b ) ) {
                $smarty->assign( "pools", $b );

                foreach( $b as $a ) {
                    if($a['mine']==true) {
                        $my_pool_b = true;
                    }
                    else {
                        $other_pool_b = true;
                    }
                }
            }
            $smarty->assign( "my_pools", $my_pool_b );
            $smarty->assign( "other_pools", $other_pool_b );

            $smarty->display( "poollist.tpl" );
}

function fatal_error( $err ) {
    global $smarty;
                $smarty->assign( 'error', $err );
                $smarty->display('error.tpl');
                exit;
}
?>
