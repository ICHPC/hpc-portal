<?
require_once 'configuration.inc';
require_once 'auth-functions.inc';
require_once 'db-functions.inc';


function authenticate($user, $password)
# Go through all methods in $UP_options['auth_methods']
# attempting to authenticate with each one.
#
# If $UP_options['auth_type'] is strict then all methods must succeed,
# if it's permissive then any must succeed.
#
# The user's fullname is taken from the first successful method.
# A method cannot return an empty name as this is detected as an auth failure.
#
# Note that $user is not escaped in any way from the user's input.
#
# Returns the fullname if authentication was successful, NULL otherwise.
{
    global $UP_options;
    $methods = array_map('trim',explode(',',$UP_options['auth_methods']));
    switch (trim($UP_options['auth_type']))
    {
        case 'permissive':
            $permissive = true;
            $strict = false;
        break;

        case 'strict':
            $permissive = false;
            $strict = true;
        break;

        #SJC FIXME
        default:
        die ("oops\n");
        break;
    }

    $authed = false;
    $denied = false;
    $gecos = NULL;
    foreach ($methods as $m)
    {
        $a = false;
        switch ($m)
        {
            case 'file':
            $gc = file_authenticate($user, $password);
            break;

            case 'ldap':
            $gc = ldap_authenticate($user, $password);
            break;

            #SJC FIXME
            default:
            die ("oops\n");
            break;
        }
        if (! empty($gc))
        {
            if (empty($gecos)) $gecos = $gc;
            $a = true;
        }
        $authed = $a || $authed;
        $denied = (!$a) || $denied;
    }
    if ( ($strict && (! $denied)) || ($permissive && $authed)) return $gecos;
    return NULL;
}

function get_job_project( $jobid ) {
    $query="SELECT project_id FROM job_list WHERE job_id = ?";
    $arr= db_query($query, array($jobid));
    return $arr['project_id'];
}


function get_job_owner( $jobid ) {
    $query="SELECT user_id FROM job_list WHERE job_id = ?";
    $arr= db_query($query, array($jobid));
    return $arr['user_id'];
}

function get_job_project_name( $jobid ) {
    $uid = get_job_owner( $jobid );
    $pid = get_job_project( $jobid );
    return get_project_name( $uid, $pid );

}

function get_next_job_id() {
    $query="SELECT nextval('job_seQ_id')";
    $arr= db_query($query, array());
    return $arr['nextval'];

}

function new_get_job_list( $uid, $order, $limit, $orderdir=0, $project_id=-1, $offset=0, $filter="",
                            $status=0, $published=0, $submittime=0, $embargoed=0,
                            $embargo_mail_sent=0, $countonly=0 ) {
    $values = array($uid);

    $statidx = array ('I' => 'Pending', 'R' => 'Running', 'C' => 'Finished', 'X' => 'Deleted', 'U'=>'Pending', 'H' => 'Paused', '?' => 'Not Queued', '!' => 'Failed', ' ' => 'Unknown' );
    $statq = array( 'I', 'R', 'C', 'O' );

    $orderby = array ( 'job_id' , 'a.app_id' , 'submit_time', 'wall_time',  'status', 'is_published', 'embargo' );

    if ( $countonly == 0 ) {
        $query = "SELECT a.*, b.app_id, b.app_name, a.embargo_date-current_date AS embargo FROM job_list a, app_list b WHERE user_id = ? AND deleted=false AND a.app_id = b.app_id";
    } else {
        $query = "SELECT COUNT(*) FROM job_list a, app_list b WHERE user_id = ? AND deleted=false AND a.app_id = b.app_id";
    }

    if ( $submittime > 0 ) {
    # Ah, sorry MySQL.  Blame lack of standards.
        $query .= " AND (CURRENT_TIMESTAMP -";
        switch ( $submittime ) {
            case 1:
                $query .= " INTERVAL '1 HOUR'";
                break;
            case 2:
                $query .= " INTERVAL '1 DAY'";
                break;
            case 3:
                $query .= " INTERVAL '1 WEEK'";
                break;
            default:
                $query .= " INTERVAL '1 MONTH'";
                break;
        }
        $query .= ") <= submit_time";
    }

    if ( $published == 1 ) $query .= " AND is_published = true";
    if ( $published == 2 ) $query .= " AND is_published = false";

    if ( $status > 0 ) {
        if ( $status < 4 ) {
            $query .= " AND status = ?";
            $values[] = $statq[$status-1];
        } else {
            $query .= " AND NOT (status = 'I' OR status = 'R' OR status = 'C')";
        }
    }

    if ( $project_id > -1 ) {
        $query .= " AND project_id = ?";
        $values[] = $project_id;
    }

    # SJC eh?
    $filter = str_replace( "*", "%", $filter );
    $filter = str_replace( "?", "%", $filter );
    if( $filter != "" ) {
        #$query .= " AND description ILIKE '%" . $filter ."%'";
        $query .= " AND description ILIKE ?";
        $values[] = "%$filter%";
    }

    if( $embargoed > 1 ) {
        $query .= " AND embargo_status != 0";
    } else if( $embargoed == 1 ) {
        $query .= " AND embargo_status = 0";
    }
    if( $embargoed == 3 ) {
        $query .= " AND  a.embargo_date-current_date <= 0";
    }
    if( $embargo_mail_sent ) {
        $query .= " AND a.embargo_mail_sent = false";
    }

    if ( $countonly == 0 ) {
        $query .= " ORDER BY "; # can't use prepared statements here
        if( isset($order) && $order >=0 && $order < count($orderby) && $order != 6 )
        {
            #$orderbystr = $orderby[$order];
            $query .= $orderby[$order];
        } else if( $order == 6 ) {
            $query .= " embargo_status DESC, " . $orderby[$order];
        } else {
            $query .= $orderby[0];
        }
        if( $orderdir==0 )
        {
            $query .=' ASC';
            $otherdir=1;
        } else {
            $query .=' DESC';
            $otherdir=0;
        }
    }

    if ( $limit > 0 ) {
        $query .= " LIMIT ?";
        $values[] = $limit;
    }
    $query .= " OFFSET ?";
    $values[] = $offset;


#echo "<!-- $query -->";
#echo $query;
#print_r($values);
#print_r($_SESSION);

    $list = array();
    if ( $countonly == 0 ) {
        $allarr  = db_query_all( $query, $values );
    } else {
        $arr  = db_query( $query, $values );
        return $arr['count'];
    }

#echo "";
#print_r($allarr);

    if( !empty($allarr) ) {

        foreach ($allarr as $arr) {

            $job_array=array();

            $is_complete=0;

                    # Get job status, updating from server if necessary
            $jid =  $arr[ 'job_id' ];

            $job_array['jid'] =$jid ;

            $stat = get_job_status( $arr['staging_dir'], $jid );




            $job_array[ 'app_name' ]= $arr['app_name'] ;
            $job_array[ 'description'] = $arr['description'] ;


            $time_submit  = $arr['submit_time'];

            # Remove the decimal at the end of the time string

            $time_submit = preg_replace( "/\..*$/", "", $time_submit );

            $job_array['submit_time'] = $time_submit ;


            $wall_time  = $arr['wall_time'];
            if( $wall_time == -1 ) {
                $wt = "Not reported";
            }
            else if( $wall_time <1 ) {
                $wt = "--";
            }
            else {
                $hr = (int)($wall_time/3600);
                $min = (int) (($wall_time - ($hr)*3600 )/60);
                $sec = $wall_time - $hr*3600 - $min*60;
               $wt = $hr .":".$min.":".$sec;
            }

            $job_array['wall_time'] = $wt;

            $job_array['status'] = $statidx[$stat[0]] ;


            $job_array['input_files'] = list_input_files( $arr['app_id'] ) ;

            $input_name    = array();
            $input_selected= array();
            $input_value   = array();

            $idx=0;
            foreach(  $job_array['input_files'] as $K => $T) {
                $input_name[] = $T;
                $input_value[] = $K;
                $idx++;
                if( $idx==0) { $input_selected[]=1; } else {$input_selected[]=0; }
            }
            $job_array[ 'input_name'] = $input_name;
            $job_array[ 'input_selected'] = $input_selected;
            $job_array[ 'input_values'] = $input_value;

            if( $stat[0] == 'C' ) {
                $job_array[ 'output_files']= list_output_files( $arr['app_id'] , false ) ;
                $is_complete=1;
            }
            else if ($stat[0] == 'R' ) {
                $job_array[ 'output_files'] = list_output_files( $arr['app_id'] , true )  ;
            }
            else {
                $job_array[ 'output_files' ] = array() ;
            }

            $idx=0;

            $output_name=array();
            $output_value=array();
            $output_selected=array();
            foreach(  $job_array['output_files'] as $K => $T) {
                $output_name[] = $T;
                $output_value[] = $K;
                $idx++;
                if( $idx==0) { $output_selected[]=1; } else {$output_selected[]=0; }
            }
            $job_array[ 'output_name'] = $output_name;
            $job_array[ 'output_selected'] = $output_selected;
            $job_array[ 'output_values'] = $output_value;




            # FIXME
            if( $is_complete==1 ) {
#printf("<!-- %d %d %d %d -->\n", $job_id, ($arr['app_id'] ==0) ,  (is_publishable(  $arr['app_id'], $jid )) ,  is_published(  $jid ) );
                if( ($arr['app_id'] ==0) ||  (is_publishable(  $arr['app_id'], $jid )) ) {
                    if( is_published(  $jid ) ) {
                        $job_array['publish'] = 'view';
                        if(( $arr['handle'] != "" )) {
                            $job_array['handle']  = $arr[ 'handle' ];
                        }
                        $job_array['chempound']  = $arr[ 'chempound_url' ];
                        $job_array['figshare']  = $arr[ 'figshare_handle' ];
                        if( ! strstr( $arr['figshare_handle'], "figshare" ) ) {
                            $job_array['figshare_draft']  = "1";
                        }
                    }
                    else {
                        $job_array['publish'] = 'publish';
                    }
                }
                else {
    printf("<!-- here -->\n");
                    $job_array['publish'] = 'na';
                }
            }
            else {
                $job_array['publish'] = '';
            }
    #           echo "</TD>";

    #           echo "<TD>";
    #           echo '<A HREF="deleteJob.php?jid='.$arr['job_id'].'">Delete</a>';
    #           echo "</TD>";

    #       echo "</TR>\n";

            $job_array['embargo'] = $arr['embargo'];
            $job_array['embargo_status'] = $arr['embargo_status'];
            $job_array['embargo_date'] = $arr['embargo_date'];

            $list[] = $job_array;

        }

        return $list;
    }
    else {
        return null;
    }




}

function is_required_input_file( $app_id , $idx ) {
    $query="SELECT required FROM app_input WHERE app_id= ? AND index = ?";

    $arr  = db_query( $query, array($app_id, $idx) );

    return ($arr['required']);

}

function is_admin( $user ) {

    $query = "SELECT is_admin FROM users WHERE username = ?";

    $arr  = db_query( $query, array($user) );

    return ($arr['is_admin']);

}

function is_blocked( $user ) {
    $query = "SELECT is_blocked FROM users WHERE username = ?";
    $arr = db_query( $query, array($user) );
    return ($arr['is_blocked']);
}

function check_user_registration(  $user ) {

    if( ! get_uid(  $user ) )  {
        add_user( $user );
        return 1;
    }
    return 0;
}

function add_user( $dn ) {
    # SJC transaction?
    $query = "INSERT INTO users ( username ) VALUES ( ? )";
    # SJC check duplicate username?
//echo $query;
    $result = db_query( $query, array($dn) );

    $query = "SELECT user_id FROM users WHERE username = ?";
    $arr = db_query( $query, array($dn) );
    $user_id = $arr['user_id'];

    $query ="SELECT * from pools WHERE public = true";
    $arr = db_query( $query, array() );

    while( $arr!=NULL ) {
        #$q2 = "INSERT INTO acl ( user_id, pool_id ) VALUES ( ".$userid.", ". $arr['pool_id'] .")";
        $q2 = "INSERT INTO acl ( user_id, pool_id ) VALUES ( ?, ? )";
        $result = db_query( $q2, array($user_id, $arr['pool_id']) );
    }

    add_profile(  $user_id );
    add_project(  $user_id, 'default' );
}

function owns_job( $uid, $jid ) {
    #$query = "SELECT * FROM job_list WHERE userid = ".$uid." AND job_id = ".$jid;
    $query = "SELECT * FROM job_list WHERE user_id = ? AND job_id = ?";
    $arr = db_query( $query, array($uid, $jid) );
    if( $arr != NULL )
    {
        return true;
    } else {
        return false;
    }
}


function owns_project( $uid , $project_id ) {
    #$query = "SELECT * from projects where userid = ". $uid. " AND project_id = " .$project_id ;
    $query = "SELECT * FROM projects WHERE user_id = ? AND project_id = ?";

    $arr = db_query( $query, array($uid, $project_id) );

    return ( $arr != NULL );
}

function delete_project( $uid , $project_id ) {
    #$query = "DELETE FROM projects WHERE userid = ". $uid. " AND project_id = " .$project_id ;
    $query = "DELETE FROM projects WHERE user_id = ? AND project_id = ?";
    $arr = db_query( $query, array($uid, $project_id) );
}

function add_project( $uid , $description ) {
    #$query = "INSERT INTO projects ( userid, description ) VALUES ( ". $uid . " , '" . $description . "') ";
    $query = "INSERT INTO projects ( user_id, description ) VALUES ( ?, ? )";
    $arr = db_query( $query, array($uid, $description) );
}

function get_project_by_jid( $jid ) {
    $query = "SELECT project_id FROM job_list WHERE job_id = ?";

    $arr = db_query( $query, array($jid) );
    if($arr!=NULL ) {
        return $arr['project_id'];
    }
    else {
        return NULL;
    }

}

function get_project_name( $uid, $project_id ) {
    $query = "SELECT * FROM projects WHERE user_id = ? AND project_id = ?";

    $arr = db_query( $query, array($uid, $project_id) );

    if($arr!=NULL ) {
        return $arr['description'];
    }
    else {
        return NULL;
    }
}

function get_projects( $uid ) {
    $query = "SELECT * FROM projects WHERE user_id = ? ORDER BY description ASC";
    $allarr = db_query_all( $query, array($uid) );

    $alist = array();
    $vlist = array();
    if (! empty($allarr) ) {
        foreach ($allarr as $arr) {
            $alist[] =  $arr['description'];
            $vlist[] = $arr['project_id'];
        }
    }
    if( !empty ( $alist ) ) {
        return  array( 'description' => $alist, 'project_id' => $vlist );
    } else {
        return array();
    }
}


function app_list(   $pool_id ) {
    $query="SELECT * FROM app_list WHERE pool_id = ? AND current=true";

    $allarr = db_query_all( $query, array($pool_id) );

    $alist = array();
    $vlist = array();
    if (!empty($allarr)) {
        foreach ($allarr as $arr) {
            $alist[] =  $arr['app_name'];
            $vlist[] = $arr['app_id'];
        }
    }

    if( !empty ( $alist ) ) {
        return array( 'description' => $alist , 'app_id' => $vlist );
    } else {
        return array();
    }
}

function get_foaf (  $uid ) {
    $query = 'SELECT foafurl FROM profile WHERE user_id = ?';
    $arr = db_query( $query, array($uid) );

    return $arr[ 'foafurl' ];
}

function get_orcid (  $uid ) {
    $query = 'SELECT orcid FROM profile WHERE user_id = ?';
    $arr = db_query( $query, array($uid) );

    return $arr[ 'orcid' ];
}



function get_uid (  $uname ) {
    $query = 'SELECT user_id FROM users WHERE username = ?';
    $arr  = db_query( $query, array($uname) );

    return $arr[ 'user_id' ];
}

function list_input_files( $app_type ) {
    $query = 'SELECT * FROM app_input WHERE app_id = ? ORDER BY index ASC';
    $allarr = db_query_all( $query, array($app_type) );

    $file_list = array();
    if (!empty($allarr)) {
        foreach ($allarr as $arr) {
            $file_list[] =   $arr['description'];
        }
    }

    return $file_list;
}

function list_output_files( $app_type , $is_running) {
    if( $is_running ) {

        #$result = pg_query( $dbconn, 'SELECT * FROM app_output WHERE app_id = ' .$app_type . ' AND available_at_runtime=true ORDER BY index ASC' );
        $query = 'SELECT * FROM app_output WHERE app_id = ? AND available_at_runtime=true ORDER BY index ASC';
    }
    else {
        #$result = pg_query( $dbconn, 'SELECT * FROM app_output WHERE app_id = ' .$app_type . ' ORDER BY index ASC' );
        $query = 'SELECT * FROM app_output WHERE app_id = ? ORDER BY index ASC';
    }

    $allarr = db_query_all( $query, array($app_type) );

    $file_list = array();
    $count=0;
    if (! empty($allarr)) {
        foreach ($allarr as $arr) {
            $file_list[$count] = $arr['description'];
            $count++;
        }
    }

    if( $count != 0 ) {
        $file_list[-1] = 'Tarball';

    }


    return $file_list;
}

function sanify(  $a ) {

    $search = array (   '@[\r\n]+@',                // Strip out newlines
                        '@&(nbsp|#160);@i',
                    );
    $replace = array (  '',
                        ' ',
                     );
    if (!isset($a)) { $a = ''; } # saner than null or warnings
    $text = preg_replace($search, $replace, $a);
    $text =  htmlentities( $text, ENT_QUOTES );
    $text = trim($text);
    return $text;
}

function make_key_name_from_pool( $pool_id ) {
    global $UP_options;

    $query = "SELECT * from pools WHERE pool_id = ?";

    $arr = db_query( $query, array($pool_id) );

    return  ( $UP_options['ssh_keydir']. '/'. $arr['pool_id'].'@'. $arr['remote_user'] . '@' . $arr['remote_host'] . '.id_rsa' );


}


function make_key_name_from_app( $app_id ) {
    global $UP_options;

    # SJC make a JOIN ?
    $query   = "SELECT pool_id FROM app_list WHERE app_id = ?";
    $arr     = db_query( $query, array($app_id) );
    $pool_id = $arr['pool_id'];

    return make_key_name_from_pool( $pool_id );
}

function make_staging_directory( $app_id, $user_id, $job_id  ){
    global $UP_options;

    $UP_make_staging_directory = $UP_options['make_staging_directory'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );

    $key = make_key_name_from_app ( $app_id );

    $cmdarray = array(  "$app_path$UP_make_staging_directory",
                        $job_id,
                        $_SESSION['gecos'],
                    );

    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );
    $output = array();

print "<!-- $cmd -->";

    exec  ($cmd, $output );

print "<!--";
print_r( $output );
print "-->";

#echo $output[0];

    if( isset ($output[0]) ) {
        return $output[0];
    }
    else {
        echo "Unable to make staging directory";
        print "cmd: $cmd\n";
        die();
    }
}


function get_job_status( $staging_dir, $job_id ) {

    $query  = "SELECT status, substatus, app_id FROM job_list WHERE job_id = ?";
    $arr    = db_query( $query, array($job_id) );
    $app_id = $arr[ 'app_id' ];

#echo "STATUS:: [".$arr['status']."]";

    if( $arr['status'] == 'C' ) {
        $ret = array ( 0=> $arr['status'], 1=> $arr['substatus'] );
        return $ret;
    }
    else {
        // fetch updated state
    global $UP_options;
    $UP_job_status          = $UP_options['job_status'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );

    $key = make_key_name_from_app ( $app_id );

    $cmdarray = array(  "$app_path$UP_job_status",
                        $staging_dir,
                    );

    $output = array ();

    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );
#print $cmd."</BR>";
    exec( $cmd, $output );

#print "TEST\n";
#print $cmd."\n";
#print "<!--";
#print_r ($output);
#print "-->\n";

    if( empty($output) ) {
        $ret = array ( 0=> ' ', 1=> ' ' ); # unknown
        return $ret;
    }

    $status = substr( sanify( $output[0] ), 0, 2 );
    $substatus = substr( sanify( $output[1] ), 0 , 2);

    #$query = "UPDATE job_list SET status= '".$status."' , substatus='". $substatus ."' WHERE job_id = " . $job_id;
    $query = "UPDATE job_list SET status= ?, substatus=? WHERE job_id = ?";

#echo $query."</BR>";

    # SJC check result?
    $result = db_query( $query, array($status, $substatus, $job_id) );

    $wt=-1;
    foreach( $output as $t ) {
        if ( strstr($t, "Walltime" ) ) {
            $ar = explode( ":", $t );
            if( $ar[1]>0 ) {
                $wt=$ar[1];
            }
        }
    }

    #$query = "UPDATE job_list SET wall_time = $wt WHERE job_id = ". $job_id;
    $query = "UPDATE job_list SET wall_time = ? WHERE job_id = ?";
    $result =  db_query( $query, array($wt, $job_id) );

    return $output;
    }

}

function copy_metadata_to_staging_directory( $app_id, $staging_dir, $filename, $contents ) {
    global $UP_options;

    $UP_make_staging_directory          = $UP_options['make_staging_directory'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );

    $key = make_key_name_from_app ( $app_id );

    $tempfile = tempnam( "/tmp" , "uportal" );
    $handle = fopen( $tempfile, "w" );
    fwrite( $handle, $contents );

    $cmdarray = array(  $tempfile,
                        "$app_user@$app_host:$staging_dir/$filename",
                    );

    $output = array ();

    fclose( $handle );

    $cmd = make_scp_cmd( $key, $cmdarray );
    exec( $cmd, $output );


    unlink( $tempfile );
}

function copy_file_to_staging_directory( $app_id, $staging_dir, $file, $input_index , $user_id ) {
    global $UP_options;

    $UP_make_staging_directory          = $UP_options['make_staging_directory'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );

    $key = make_key_name_from_app ( $app_id );

    $cmdarray = array(  $file,
                        "$app_user@$app_host:$staging_dir/input-$input_index",
                    );

    $output = array ();

#echo "</br>";
#echo $cmd;
#echo "</br>";

    $cmd = make_scp_cmd( $key, $cmdarray );
    exec( $cmd, $output );
#print_r($output );


}

function get_app_script( $app_id ) {
    $query = 'SELECT app_script FROM app_list WHERE app_id = ?';
    $arr = db_query( $query, array($app_id) );
    if( $arr != NULL ) {
        return $arr['app_script'];
    }
    else {
        return NULL;
    }
}


function get_app_user( $app_id ) {

    # SJC Make a JOIN?
    #$query   = "SELECT pool_id FROM app_list WHERE app_id = ". $app_id;
    $query   = "SELECT pool_id FROM app_list WHERE app_id = ?";
    $arr     = db_query( $query, array($app_id) );
    $pool_id = $arr['pool_id'];

    #$query   = 'SELECT remote_host, remote_user FROM pools WHERE pool_id = ' . $pool_id;
    $query   = 'SELECT remote_host, remote_user FROM pools WHERE pool_id = ?';
    $arr     = db_query( $query, array($pool_id) );
    if( $arr != NULL ) {
        return $arr['remote_user'];
    }
    else {
        return NULL;
    }
}

function get_app_path( $app_id ) {

    # SJC Make a JOIN?
    $query   = "SELECT pool_id FROM app_list WHERE app_id = ?";
    $arr     = db_query( $query, array($app_id) );
    $pool_id = $arr['pool_id'];

    $query = 'SELECT script_path FROM pools WHERE pool_id = ?';
    $arr   = db_query( $query, array($pool_id) );
    if( $arr != NULL ) {
        return $arr['script_path'].'/';
    }
    else {
        return NULL;
    }

}

function get_app_host( $app_id ) {

    # SJC Make a JOIN?
    $query   = "SELECT pool_id FROM app_list WHERE app_id = ?";
    $arr     = db_query( $query, array($app_id) );
    $pool_id = $arr['pool_id'];

    $query = 'SELECT remote_host, remote_user FROM pools WHERE pool_id = ?';
    $arr   = db_query( $query, array($pool_id) );
    if( $arr != NULL ) {
        return $arr['remote_host'];
    }
    else {
        return NULL;
    }
}


function get_app_dspace_collection( $app_id ) {

    $query = "SELECT dspace_collection FROM app_list WHERE app_id  = ?";
    $arr   = db_query( $query, array($app_id) );
    if( isset( $arr['dspace_collection'] )) {
        return $arr['dspace_collection'];
    }
    else {
        return NULL;
    }
}

function get_app_name( $app_id ) {

    $query = "SELECT app_name FROM app_list WHERE app_id  = ?";
    $arr   = db_query( $query, array($app_id) );
    if( isset( $arr['app_name'] )) {
        return $arr['app_name'];
    }
    else {
        return NULL;
    }
}

function get_app_id_for_job( $job_id ) {

    #$query = "SELECT app_id FROM job_list WHERE job_id  = '". $job_id . "'";
    $query = "SELECT app_id FROM job_list WHERE job_id  = ?";
    $arr = db_query( $query, array($job_id) );
    if( isset( $arr['app_id'] )) {
        return $arr['app_id'];
    }
    else {
        return NULL;
    }
}


function get_app_id( $app_name ) {

    #$query = "SELECT app_id FROM app_list WHERE app_name  = '". $app_name . "'";
    $query = "SELECT app_id FROM app_list WHERE app_name = ?";
    $arr = db_query( $query, array($app_name) );
    if( isset( $arr['app_id'] )) {
        return $arr['app_id'];
    }
    else {
        return NULL;
    }
}

function validate_app_type( $uid, $app_id ) {
    #$query = 'SELECT * FROM app_list a, acl b WHERE  a.app_id = '. $app_id.' AND (( a.app_id = b.app_id AND  userid = '.$uid.' ))';
    $query = 'SELECT * FROM app_list a, acl b WHERE  a.app_id = ? AND (( a.app_id = b.app_id AND  user_id = ? ))';

    $arr = db_query( $query, array($app_id, $uid) );
    if( isset( $arr['app_id'] )) {
        return true;
    }
    else {
        return false;
    }
}

function get_app_memory_requirement( $app_id ) {
    $query = "SELECT memory from app_list where app_id = ?";
    $arr = db_query( $query, array($app_id) );
    return $arr['memory'];
}

function get_app_cpu_requirement( $app_id ) {
    $query = "SELECT ncpus from app_list where app_id = ?";
    $arr = db_query( $query, array($app_id) );
    return $arr['ncpus'];
}

function get_app_walltime_requirement( $app_id ) {
    $query = "SELECT wall_time  from app_list where app_id = ?";
    $arr = db_query( $query, array($app_id) );
    return $arr['wall_time'];
}

function get_app_input_file_count( $app_id ) {
    $query = "SELECT count(index) from app_input where app_id = ?";
    $arr = db_query( $query, array($app_id) );
    return $arr['count'];
}

function get_app_output_file_count( $app_id ) {
    $query = "SELECT count(index) from app_output where app_id = ?";
    $arr = db_query( $query, array($app_id) );
    return $arr['count'];
}

function app_input_file_description( $app_id ) {
    $query = "SELECT * FROM app_input WHERE app_id = ? ORDER BY index ASC ";
    $allarr = db_query_all( $query, array($app_id) );

    $d=array();
    if (!empty($allarr)) {
        foreach ($allarr as $arr) {
            $d[] =  array( 'description' => $arr['description'], 'index' => $arr['index'],
                        'required' => $arr['required'] );
        }
    }

    return $d;
}

function register_job_details( $app_id, $user_id, $description, $staging_dir, $project_id, $jid ) {
#   $query = "SELECT nextval('job_seq_id' )";
#   $result = pg_query( $dbconn, $query );
#   $arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

#   $jid = $arr['nextval'];

    # If no projects defined, this will be -1. Reset to null to prevent foreign key violation

    if( $project_id < 0 ) {
        $project_id= 'null';
    }


    $query = "INSERT INTO job_list ( job_id, app_id, user_id,  description, staging_dir, is_published, project_id ) VALUES ( ?, ?, ?, ?, ?, ?, ? )";

    $result = db_query( $query, array($jid, $app_id, $user_id, $description, $staging_dir,
                                        0, $project_id) );

    return $jid;
}

function set_job_project( $jid, $project_id ) {
    $query = "UPDATE job_list set project_id = ? WHERE job_id = ?";

    $arr = db_query( $query, array($project_id, $jid) );
}

function check_project_owner( $uid, $project_id ) {

    $query = "SELECT count(*) FROM projects WHERE project_id = ? AND user_id = ?";
    $arr = db_query( $query, array($project_id, $uid) );

    if( $arr['count'] > 0 ) {
        return true;
    }
    else {
        return false;
    }
}


function check_job_owner( $uid, $jid ) {
    $query ="SELECT count(*) FROM job_list WHERE job_id = ? AND user_id = ?";
    $arr = db_query( $query, array($jid, $uid) );


    if( $arr['count'] > 0 ) {
        return true;
    }
    else {
        return false;
    }
}

function return_job_output_file( $jid, $idx ) {
    global $UP_options;

    $query ="SELECT * FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );

    $staging_dir = $arr['staging_dir' ];
    $app_id = $arr['app_id'];

    $query = "SELECT * FROM app_output WHERE app_id = ? AND index = ?";
    $arr = db_query( $query, array($app_id, $idx) );

    if( !isset( $arr['app_id'] ) ) {
        echo "No such output file";
        # SJC Be more graceful
        die();
    }

    $UP_remote_cat            = $UP_options['remote_cat'];
    $UP_filetest               = $UP_options['filetest'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );

    $key = make_key_name_from_app ( $app_id );

        $cmdarray = array(  "$app_path$UP_filetest",
                            "$staging_dir/output-$idx",
                        );

        $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );
        exec( $cmd, $output );
        if( $output[0] == "0" ) {
            # return a 403
            # SJC Hmm ?
            header($_SERVER["SERVER_PROTOCOL"]." 404 Not Found");
            return;
        }

    $cmdarray = array(  $UP_remote_cat,
                        "$staging_dir/output-$idx",
                    );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );


    $filename = $arr['filename'];

    # Perform a simple substitution of occurances of '{ID}' with the job id
    $filename = str_replace( "{ID}", $jid, $filename );

    header('Content-Type: "'.$arr['mimetype'].'"');
    header('Content-disposition: attachment; filename="'.$filename.'"' );
    header('Content-Transfer-Encoding: binary');
    header('Pragma: public');
    header('Expires: 0');
    header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
    passthru( $cmd );

    exit;
}


function remove_job_file( $jid, $file, $staging_file ) {
    global $UP_options;

    $query ="SELECT staging_dir, app_id FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );

    $staging_dir = $arr['staging_dir' ];
    $app_id = $arr['app_id'];

    $UP_remote_rm    = $UP_options['remote_rm'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );


    $key = make_key_name_from_app ( $app_id );
    //copy the remote file $file to the staging_file

    $cmdarray = array(  $UP_remote_rm,
                        "$staging_dir/$file",
                    );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );
    echo "<!-- $cmd -->\n\n";

    exec( $cmd, $output );

//  print $cmd;
//  print ("<BR><BR>");
//  print_r( $output );


}


function copy_job_file( $jid, $file, $staging_file, $user_id ) {
    global $UP_options;

    $query ="SELECT staging_dir, app_id FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );

    $staging_dir = $arr['staging_dir' ];
    $app_id = $arr['app_id'];

    $UP_remote_cat          = $UP_options['remote_cat'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );


    $key = make_key_name_from_app ( $app_id );
    //copy the remote file $file to the staging_file

    #$cmd = $UP_scp . ' '. $UP_ssh_key . ' '. $key . ' '. $app_user.'@'.$app_host;
    #$cmd = $cmd . ':"' . $staging_dir.'/'.$file. '" "'.$staging_file.'"';
    $cmdarray = array(  "$app_user@$app_host:$staging_dir/$file",
                        $staging_file,
                    );
    $cmd = make_scp_cmd( $key, $cmdarray );

    exec( $cmd, $output );

//  print $cmd;
//  print ("<BR><BR>");
//  print_r( $output );


}

function return_job_input_file(  $jid, $idx ) {
    global $UP_options;

    $query ="SELECT staging_dir, app_id FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );

    $staging_dir = $arr['staging_dir' ];
    $app_id = $arr['app_id'];

    $query = "SELECT * FROM input_file WHERE job_id = ? AND index = ?";
    $arr = db_query( $query, array($jid, $idx) );

    if( !isset( $arr['job_id']) ) {
        return 0;
    }

    $UP_remote_cat        = $UP_options['remote_cat'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );

    $key = make_key_name_from_app ( $app_id );
    // call the tar script

    $cmdarray = array(  $UP_remote_cat,
                        "$staging_dir/input-$idx",
                    );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );

    header('Content-Type: "'.$arr['mimetype'].'"');
    header('Content-disposition: attachment; filename="'.$arr['filename'].'"' );
    header('Pragma: public');
    header('Expires: 0');
    header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
    passthru( $cmd );


    exit;
}

function return_job_tar_file(  $jid , $user_id ) {
    global $UP_options;

    $query ="SELECT staging_dir, app_id FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );

    $staging_dir = $arr['staging_dir' ];

    $app_id = $arr['app_id'];

    $UP_make_input_tar        = $UP_options['make_input_tar'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );

    $key = make_key_name_from_app ( $app_id );

    // call the tar script

    $cmdarray = array(  "$app_path$UP_make_input_tar",
                        $staging_dir,
                    );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );

    header("Content-Type: application/x-gzip");
    header("Content-disposition: attachment; filename=\"output_".$jid.".tar.gz\"");

    header('Pragma: public');
    header('Expires: 0');
    header("Cache-Control: must-revalidate, post-check=0, pre-check=0");

    passthru( $cmd );

    exit;
}

function record_input_type( $job_id, $name, $mime_type, $index ) {

    $query ="INSERT INTO input_file ( job_id, filename, mimetype, index ) VALUES ( ?, ?, ?, ? )";
    #echo "</br>";
    //echo $query;
    $result = db_query( $query, array($job_id, $name, $mime_type, $index) );
}

function execute_job( $job_id, $app_id, $app, $staging_dir ) {
    global $UP_options;


    $input_file_count=1;
    $output_file_count=4;
    $memory_requirement=1000;

    $UP_run_job    = $UP_options['run_job'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );

    $key = make_key_name_from_app ( $app_id );

    $app_script = get_app_script( $app_id );

    $input_file_count  = get_app_input_file_count ( $app_id );
    $output_file_count = get_app_output_file_count( $app_id );
    $memory_requirement = get_app_memory_requirement( $app_id );
    $cpu_requirement    = get_app_cpu_requirement( $app_id );
    $walltime           = get_app_walltime_requirement( $app_id );

    $cmdarray = array(  "$app_path$UP_run_job",
                        $staging_dir,
                        $input_file_count,
                        $output_file_count,
                        $app_script,
                        $memory_requirement,
                        $cpu_requirement,
                        $walltime,
                    );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );
#echo $cmd;
print "<!-- $cmd -->";

    exec  ($cmd, $output );

print "<!--";
print_r( $output );
print "-->";

}


function delete_job( $job_id ) {
    global $UP_options;

    #$query = "SELECT count(*) FROM job_list WHERE job_id = ".$job_id ." AND deleted = false";
    $query = "SELECT count(*) FROM job_list WHERE job_id = ? AND deleted = false";
    $arr = db_query( $query, array($job_id) );

    if( $arr['count']==0 ) {
        return 0;
    }

    #$query = "UPDATE job_list SET deleted= true WHERE job_id = " .$job_id . " AND deleted=false";
    $query = "UPDATE job_list SET deleted= true WHERE job_id = ? AND deleted=false";
    $arr = db_query( $query, array($job_id) );

    $query = "SELECT * FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($job_id) );
    $app_id = $arr['app_id'];
    $staging_dir = $arr['staging_dir'];



#   $batch_id = $arr['batch_id'];
#   $app_id = $arr[ 'app_id' ];
#   $staging_dir = $arr[ 'staging_dir' ];
#
#   if(!isset($batch_id)) {
#       echo "Cannot find remote job id";
#       $batch_id = -1;
#       die_now();
#   }

    $UP_delete_job  = $UP_options['delete_job'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );


    $key = make_key_name_from_app ( $app_id );

    $cmdarray = array(  "$app_path$UP_delete_job",
                        $staging_dir,
                    );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );
//echo $cmd;
    exec  ($cmd, $output );


    return 1;
}

function is_published( $jid ) {

    $query  = "SELECT is_published FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );

    return ($arr['is_published']);
}

function output_file_index_of_mimetype( $appid, $mime ) {

    $query="SELECT index FROM app_output WHERE mimetype = ? AND app_id = ?";
    $arr = db_query( $query, array($mime, $appid) );
    if( $arr == NULL ) {
        return -1;
    }
    else {
        return $arr['index'];
    }
}

function has_output_file_of_mimetype(  $appid, $mime ) {

    $query="SELECT count(*) FROM app_output WHERE mimetype = ? AND app_id = ?";
    $arr = db_query( $query, array($mime, $appid) );
    $cnt = $arr['count'];

    if( $cnt == 0 ) {
        return false;
    }
    else {
        return true;
    }
}

function is_publishable( $appid, $jid ) {

    # Check to see whether this app type has an output file of type 'application/x-mets-archive'
    $result = ( has_output_file_of_mimetype(  $appid, 'application/x-mets-archive' ) );
    return $result;
}

function pool_motd( $pool_id) {
    global $UP_options;

    $query   = "SELECT * from pools WHERE pool_id = ?";
    $arr = db_query( $query, array($pool_id) );

    # call app_config on the remote host

    $key = make_key_name_from_pool( $pool_id );

    $UP_pool_status = "motd";

    $app_user =  $arr['remote_user'];
    $app_host =  $arr['remote_host'];
    $app_path =  $arr['script_path'] ."/";

    $cmdarray = array(  "$app_path$UP_pool_status",
                    );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );

    $output = array();

#   print "<p>$cmd";
    exec  ($cmd, $output );
    return implode( "</br>",$output);

}
function pool_status( $pool_id) {
    global $UP_options;

    $query   = "SELECT * from pools WHERE pool_id = ?";
    $arr = db_query( $query, array($pool_id) );

    # call app_config on the remote host

    $key = make_key_name_from_pool( $pool_id );

    $UP_pool_status = $UP_options['pool_status'];

    $app_user =  $arr['remote_user'];
    $app_host =  $arr['remote_host'];
    $app_path =  $arr['script_path'] ."/";

    $cmdarray = array(  "$app_path$UP_pool_status",
                    );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );

    $output = array();

    exec  ($cmd, $output );

    # $output should now contain an XML doc describing thte applications
    # print_r( $output );

    $xml = implode( "\n", $output );



    $apps = pool_status_xml_parse( $xml );

    if( is_array( $apps ) ) {
        return $apps;

    }
    else {
        return NULL;
    }

}

function app_in_pool( $pool_id, $app_id ) {

    #$query = "SELECT * FROM app_list WHERE  pool_id = " . $pool_id . "AND app_id = ". $app_id;
    $query = "SELECT * FROM app_list WHERE  pool_id = ? AND app_id = ?";

    $arr = db_query( $query, array($pool_id, $app_id) );

    return ( $arr!=NULL );

}

function has_access_to_pool( $pool_id, $user_id ) {

    #$query = "SELECT * FROM acl WHERE  user_id = " . $user_id . "AND pool_id = ". $pool_id;
    #$query = "SELECT * FROM acl WHERE  user_id = ? AND pool_id = ? AND deleted = false";
    $query = "SELECT * from acl a, pools b WHERE a.user_id = ? AND a.pool_id = ? AND (a.pool_id = b.pool_id) AND b.deleted=false";
    $arr = db_query( $query, array($user_id, $pool_id) );

    return ( $arr!=NULL );

}

function get_available_pools( $user_id ) {

    $query = "SELECT * FROM acl a JOIN pools b ON a.pool_id = b.pool_id WHERE  a.user_id = ? AND deleted = false";
    $allarr = db_query_all( $query, array($user_id) );

    $idx  = array();
    $desc = array();

    if (! empty($allarr)) {
        foreach ($allarr as $arr) {
            $idx[] = $arr['pool_id'];
            $desc[]= $arr['description'];
        }
    }

    return array( 'index' => $idx , 'description' => $desc );
}

function get_pools( $user ) {
    $user_id = get_uid( $user );

    $query = "SELECT * FROM pools WHERE ( pool_id in ( SELECT pool_id from acl WHERE user_id = ? ) OR public=true ) AND deleted = false";
    $allarr = db_query_all( $query, array($user_id) );

    $plist = array();
#print $query;
    if (!empty($allarr)) {
        foreach ($allarr as $arr) {
            $mine= false;
            $foo = array(
                'user' => $arr['remote_user'],
                'host'=>$arr['remote_host'],
                'description' => $arr['description'],
                'path' => $arr['script_path'],
                'pool_id' => $arr['pool_id'],
                'mine' => $mine,
            );
            if( $arr['owner'] == $user_id ) { $foo['mine']=true; }
            $plist[] = $foo;
        }
    }

    return $plist;
}

function get_pool( $pool_id ) {

    $query = "SELECT * FROM pools WHERE pool_id = ?";
    $arr = db_query( $query, array($pool_id) );

    $foo = array();
#print $query;
    if (!empty($arr)) {
        $foo = array(
            'user' => $arr['remote_user'],
            'host'=>$arr['remote_host'],
            'description' => $arr['description'],
            'path' => $arr['script_path'],
            'pool_id' => $arr['pool_id'],
            'mine' => true,
            'public' => $arr['public'],
        );
    }
    return $foo;
}

function set_pool_public( $pool_id, $public ) {
    if( $public ) {
        $query = "UPDATE pools SET public=true WHERE pool_id = ?";
    } else {
        $query = "UPDATE pools SET public=false WHERE pool_id = ?";
    }
    $arr = db_query( $query, array($pool_id) );
}

function delete_pool( $user, $pool_id ) {
    $user_id = get_uid( $user );

    $query = "UPDATE pools SET deleted=true WHERE pool_id = ? AND owner = ?";
    $result = db_query( $query, array($pool_id, $user_id) );

}

function owns_pool( $username, $pool_id ) {

    $user_id = get_uid( $username );

    #$query = "SELECT owner FROM pools WHERE pool_id = " . $pool_id . " AND owner = " . $user_id;
    $query = "SELECT owner FROM pools WHERE pool_id = ? AND owner = ? AND deleted = false";

    $arr = db_query( $query, array($pool_id, $user_id) );

    if( $arr!=NULL ) {
        return true;
    }
    else {
        return false;
    }

}

function get_key( $pool_id ) {
    $key = make_key_name_from_pool( $pool_id );

    header('Content-Type: "application/x-public-key"');
    header('Content-disposition: attachment; filename="portalkey.rsa.pubkey"' );
    header('Pragma: public');
    header('Expires: 0');
    header("Cache-Control: must-revalidate, post-check=0, pre-check=0");

    $arr = file_get_contents( $key . ".pub" );
    print $arr;
    exit;
}

function add_pool( $user, $remote_user, $remote_host, $script_path, $description ) {
    global $UP_options;

    # first get pool id
    $query = "SELECT nextval('pool_id_seq')";
    $arr = db_query( $query, array() );

    $pool_id = $arr['nextval'];

    $user_id = get_uid( $user );

    #Special untainting

    $remote_host = preg_replace( "/[^\w\.]/", "", $remote_host );
    $remote_user = preg_replace( "/[^\w\.]/", "", $remote_user );
    $script_path = preg_replace( "/[^\w\.\/\-\_]/", "", $script_path );


    #$query = "INSERT INTO pools ( pool_id, owner, remote_user, remote_host, script_path, description ) VALUES ( " . $pool_id. ", ". $user_id . ",'" . $remote_user ."','".$remote_host ."','" . $script_path ."', '". $description ."')";
    $query = "INSERT INTO pools ( pool_id, owner, remote_user, remote_host, script_path, description ) VALUES (?, ?, ?, ?, ?, ? )";

    $result = db_query( $query, array($pool_id, $user_id, $remote_user, $remote_host, $script_path,
                                    $description) );

    # Add to ACL
    #$query  = "INSERT INTO acl (pool_id, user_id ) VALUES ( ". $pool_id . "," . $user_id .")";
    $query  = "INSERT INTO acl (pool_id, user_id ) VALUES ( ?, ? )";
    $arr = db_query( $query, array($pool_id, $user_id) );

    $key = make_key_name_from_pool( $pool_id );

    # Generate a new ssh key

#    $sshkeygen = $UP_options['ssh-keygen'];
#    $cmd = $sshkeygen . " -t rsa -f " . $key;
    $cmdarray = array(    $UP_options['ssh-keygen'],
                        '-t', 'rsa',
                        '-f', $key,
                    );
    $cmd = escapearguments( $cmdarray );
    $arr = exec( $cmd );

    return ( $pool_id );
}

function get_pool_applications( $pool_id ) {

    #$query = "SELECT * from app_list WHERE current= true and pool_id = " . $pool_id;
    $query = "SELECT * FROM app_list WHERE current = true AND pool_id = ?";
    $allarr = db_query_all( $query, array($pool_id) );

    $list = array();

    if (!empty($allarr)) {
        foreach ($allarr as $arr) {
            $app_id = $arr['app_id'];

            # Input
            #$query2   = "SELECT * FROM app_input WHERE app_id = ". $app_id . " ORDER BY index ASC";
            $query2   = "SELECT * FROM app_input WHERE app_id = ? ORDER BY index ASC";
            $allarr2  = db_query_all( $query2, array($app_id) );

            $inp=array();
            if (! empty($allarr2)) {
                foreach ($allarr2 as $arr2) {
                    $inp[] = array( 'description' => $arr2['description'], 'mimetype' => $arr2['mimetype'] );
                }
            }


            #Output
            #$query2   = "SELECT * FROM app_output WHERE app_id = ". $app_id . " ORDER BY index ASC";
            $query2   = "SELECT * FROM app_output WHERE app_id = ? ORDER BY index ASC";
            $allarr2  = db_query_all( $query2, array($app_id) );

            $out=array();
            if (!empty($allarr2)) {
                foreach ($allarr2 as $arr2) {
                    $out[] = array( 'description' => $arr2['description'],
                            'mimetype' => $arr2['mimetype'], 'filename' => $arr2['filename'] );
                }
            }

            $a = array ( 'ncpus'=>$arr['ncpus'], 'memory'=>$arr['memory'],
                    'name' => $arr['app_name'], 'script' => $arr['app_script'],
                    'input' => $inp, 'output' => $out ,
                    'walltime' => $arr['wall_time'] );

            $list[] = $a;
        }
    }

    return $list;
}

function refresh_pool_applications( $pool_id ) {
    global $UP_options;

    #$query   = "SELECT * FROM pools WHERE pool_id = " . $pool_id;
    $query   = "SELECT * FROM pools WHERE pool_id = ?";
    $arr = db_query( $query, array($pool_id) );

    # call app_config on the remote host

    $key = make_key_name_from_pool( $pool_id );

    $UP_app_config = $UP_options['app_config'];

    $app_user =  $arr['remote_user'];
    $app_host =  $arr['remote_host'];
    $app_path =  $arr['script_path'] ."/";

    $cmdarray = array(  "$app_path$UP_app_config",
                    );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );

    $output = array();

print "<!--";
print $cmd;
print "-->";

    exec  ($cmd, $output );

print "<!--";
print_r( $output );
print "-->";
    # $output should now contain an XML doc describing thte applications
    $xml = implode( "\n", $output );



    $apps = pool_xml_parse( $xml );

    if( ! is_array( $apps ) ) {
        return $apps;

    }
    else {
        pool_table_update( $apps, $pool_id ) ;
    }
}

function pool_table_update( $apps, $pool_id ) {

    #$query = "UPDATE app_list SET current = false WHERE pool_id = ". $pool_id;
    $query = "UPDATE app_list SET current = false WHERE pool_id = ?";
    $result = db_query( $query, array($pool_id) );


    foreach( $apps as $app ) {

        $q1 = "SELECT nextval('app_seq_id')";
        $arr = db_query( $q1, array() );
        $app_id = $arr['nextval'];


        #$query = "INSERT INTO app_list (app_id, app_name, app_script, pool_id, ncpus, memory, wall_time, current ) VALUES ( ". $app_id .", '" . $app['description'] . "' , '" . $app['script']. "' , " . $pool_id . ", " . $app['ncpus']. ", " . $app['memory'] .  ", " . $app['walltime'].", true );";
        $query = "INSERT INTO app_list (app_id, app_name, app_script, pool_id, ncpus, memory, wall_time, current ) VALUES ( ?, ?, ?, ?, ?, ?, ?, ? )";

        $result = db_query( $query, array($app_id, $app['description'],
                                    $app['script'], $pool_id, $app['ncpus'],
                                    $app['memory'], $app['walltime'], true) );

        $inputs = $app['inputs'];

        foreach( $inputs as $input ) {
            #$query = "INSERT INTO app_input ( app_id, index, description, mimetype, required ) VALUES ( " . $app_id .", ". $input['index'] . ", '" . $input['description'] ."', '". $input['mimetype']."', ". $input['required'] .")";
            $query = "INSERT INTO app_input ( app_id, index, description, mimetype, required ) VALUES ( ?, ?, ?, ?, ? )";

            $result = db_query( $query, array($app_id, $input['index'],
                                        $input['description'], $input['mimetype'],
                                        $input['required']) );

        }

        $outputs = $app['outputs'];

        foreach( $outputs as $output ) {
            #$query = "INSERT INTO app_output ( app_id, index, description, mimetype, filename ) VALUES ( " . $app_id .", ". $output['index'] . ", '" . $output['description'] ."', '". $output['mimetype']."', '" . $output['filename']."')";
            $query = "INSERT INTO app_output ( app_id, index, description, mimetype, filename ) VALUES ( ?, ?, ?, ?, ? )";
            $result = db_query( $query, array($app_id, $output['index'],
                                        $output['description'], $output['mimetype'],
                                        $output['filename']) );
        }
    }

}

function pool_status_xml_parse( $arr ) {

    if( empty($arr) ) return 'No XML passed in';

    $dom = new DomDocument();
    if( !$dom->loadXML( $arr ) ) {
        return 'No XML found';
    }

    $apps = $dom->getElementsByTagName('host');

    $error = NULL;

    $statusx=array();

    foreach( $apps as $app ) {
        $hostname = sanify( $app->getAttribute('hostname')  );
        $memory   = (int) $app->getAttribute('memory');
        $ncpus    = (int) $app->getAttribute('ncpus');
        $status   = sanify( $app->getAttribute('status')  );

        $statusx[] = array ( 'hostname' => $hostname, 'memory' => $memory, 'ncpus' => $ncpus, 'status' => $status );
    }

    return $statusx;
}

function pool_xml_parse( $arr ) {

    if( empty($arr) ) return 'No XML passed in';

    $dom = new DomDocument();
    if( !$dom->loadXML( $arr ) ) {
        return 'No XML found';
    }

    $applist= array();

    $apps = $dom->getElementsByTagName('application');

    $error = NULL;

    foreach( $apps as $app ) {

        $script = sanify( $app->getAttribute('script')  );
        $app_description = sanify( $app->getAttribute('description')  );
        $ncpus= (int) $app->getAttribute('ncpus');
        $memory=(int) $app->getAttribute('memory');
        $walltime =(int) $app->getAttribute('walltime');



        if( empty( $script   )  ) { $error = "[$app_description]: No script"; }
        if( empty( $app_description   )  ) { $error = "[$app_description]: No description"; }
        if( empty( $ncpus  ) || $ncpus<1  ) { $error = "[$app_description]: No NCPUS";  }
        if( empty( $memory ) || $memory<0 ) { $error = "[$app_description]: No memory"; }
        if( empty( $walltime ) || $walltime<0 ) { $error = "[$app_description]: No walltime"; }



        $inp = array();

        $inputs = $app->getElementsByTagName( 'input' );

        foreach( $inputs as $input ) {
            $mimetype   = sanify( $input->getAttribute( 'mimetype' ) );
            $description= sanify( $input->getAttribute( 'description' ) );
            $index      = (int) $input->getAttribute( 'index' );
            $required   = sanify( $input->getAttribute( 'required' ) );

            if( empty ( $required ) ) { $required = "yes"; }

            if( $required=="yes" ) { $required='true'; }
            else { $required='false'; }

            if( empty ( $mimetype    ) ) { $error = "No mimetype (input)"   ; }
            if( empty ( $description ) ) { $error = "No description (input)"; }
            if( $index!=0 && !is_numeric( $index ) ) { $error = "No index (input) $description"; }

            $inp[] = array ( 'mimetype'=>$mimetype, 'description'=>$description, 'index'=>$index, 'required'=>$required );
        }

        $oup = array();

        $outputs = $app->getElementsByTagName( 'output' );

        foreach( $outputs as $output ) {
            $mimetype    = sanify( $output->getAttribute( 'mimetype' ) );
            $description = sanify( $output->getAttribute( 'description' ) );
            $index       = (int) $output->getAttribute( 'index' );
            $filename    = sanify( $output->getAttribute( 'filename' ) );

            if( empty ( $mimetype    ) ) { $error = "No mimetype (output)"   ; }
            if( empty ( $description ) ) { $error = "No description (output)"; }
            if( empty ( $filename    ) ) { $error = "No filename (output)"; }
            if( $index!=0 && !is_numeric( $index ) ) { $error = "No index (output) "; }

            $oup[] = array ( 'mimetype'=>$mimetype, 'description'=>$description, 'index'=>$index, 'filename'=>$filename );


        }

        $app = array( 'description'=>$app_description, 'script'=>$script, 'ncpus'=>$ncpus, 'memory'=>$memory, 'inputs'=>$inp, 'outputs'=>$oup, 'walltime'=>$walltime );

        $applist[] = $app;


        if( !empty($error) ) { $error = "Application $name : $error"; return $error; }
    }

    return $applist;
}

function get_script_for_app_id( $app_id ) {
    $query = "SELECT app_script FROM app_list WHERE app_id = ?";

    $arr = db_query( $query, array($app_id) );

    return $arr['app_script'];

}

function manage_job_invocation( $uid, $app_id, $description, $project_id ) {
    global $_FILES;



   #// have the input files been specified? //
   #if( isset ( $_FILES['input-0'] )  ) {


            $app = get_script_for_app_id( $app_id );
   //    echo "we'll make this now!</br>";
            $job_id = get_next_job_id();


         // copy the input files over to the staging directory
         $i=0;

         $input_file_count = get_app_input_file_count( $app_id );


            // check to see whether all mandatory input files have been specified
            for( $i=0; $i< $input_file_count; $i++ ) {
                if( is_required_input_file( $app_id, $i ) ) {
                    if( !strcmp( $_FILES['input-'.$i]['tmp_name'], '' ) ) {
                        print_r($_FILES);
                        return "Required input file not specified: " . $_FILES['input-'.$i]['name'];
                    }
                    if ($_FILES['input-'.$i]['error']!=0 ) {
                        return "Required input file failed: " .  $_FILES['input-'.$i]['error']!=0;
                    }
                }
            }

         $staging_dir = make_staging_directory( $app_id, $uid, $job_id );

            copy_metadata_to_staging_directory( $app_id, $staging_dir, "project",  get_project_name( $uid, $project_id ) );
            copy_metadata_to_staging_directory( $app_id, $staging_dir, "description", $description );

            $foaf = get_foaf( $uid );
            if( strlen(trim($foaf)) > 0 ) {
                copy_metadata_to_staging_directory( $app_id, $staging_dir, "foaf", $foaf );
            }
            $orcid = get_orcid( $uid );
            if( strlen(trim($orcid)) > 0 ) {
                copy_metadata_to_staging_directory( $app_id, $staging_dir, "orcid", $orcid );
            }


         #for( $i=0; $i < $input_file_count; $i++ ) {
#
         #   if( !strcmp( $_FILES['input-'.$i]['tmp_name'], '' ) || $_FILES['input-'.$i]['error']!=0 ) {
         #      echo "Input file " . ($i+1) . " Not specified";
         #      die_now();
         #   }
         #}

         register_job_details( $app_id, $uid, $description, $staging_dir, $project_id, $job_id );

         for( $i=0; $i < $input_file_count; $i++ ) {
                if( isset ($_FILES['input-'.$i ]) ) {
                $tmp_file  =  $_FILES['input-' . $i ][ 'tmp_name' ];
             $mime_type =  $_FILES['input-'.$i ][ 'type' ];
              $name      =  $_FILES['input-'.$i ][ 'name' ];
               $error     = $_FILES['input-'.$i ][ 'error' ];
                copy_file_to_staging_directory( $app_id, $staging_dir, $tmp_file, $i, $uid );
                record_input_type( $job_id, $name, $mime_type, $i );
                }
         }

         execute_job( $job_id, $app_id, $app, $staging_dir );

            return "";

   #}
   #else {
    #   return 0;
   #}

}

function get_pool_acl( $pool_id ) {

    $query = "SELECT user_id, username FROM users ORDER BY username ASC";
    $allarrusers = db_query_all( $query, array() );

    $query = "SELECT user_id FROM acl WHERE pool_id = ?";
    $allarracl = db_query_all( $query, array($pool_id) );
    $acls = array();
    if (! empty($allarracl)) {
        foreach ($allarracl as $acl) {
            $acls[$acl['user_id']] = 1;
        }
    }

    $res = array();
    if (! empty($allarrusers)) {
        foreach ($allarrusers as $arr) {
            $is_mem = false;
            if ( array_key_exists( $arr['user_id'], $acls ) ) { $is_mem=true; }
            $res[] = array( 'uname' => $arr['username'] , 'user_id' => $arr['user_id'], 'member' => $is_mem );

        }
    }
    return $res;
}

function set_pool_acl( $acl_user_ids, $pool_id, $user_id ) {
    $query = "DELETE FROM acl WHERE pool_id = ? AND user_id != ?";
    $arr = db_query( $query, array($pool_id, $user_id) );

    # If this gets slow then make a transaction method
    $query = "INSERT INTO acl (user_id, pool_id) VALUES (?, ?)";
    foreach( $acl_user_ids as $id ) {
        if( user_id_exists($id) && $user_id != $id ) {
            $arr = db_query( $query, array($id, $pool_id) );
        }
    }
}

function get_admin_users() {
    $query = "SELECT user_id, username, is_admin, is_blocked FROM users ORDER BY username ASC";
    $allarrusers = db_query_all( $query, array() );

    $res = array();
    if (! empty($allarrusers)) {
        foreach ($allarrusers as $arr) {
            $res[] = array( 'uname' => $arr['username'] , 'user_id' => $arr['user_id'],
                'admin' => $arr['is_admin'], 'blocked' => $arr['is_blocked'] );

        }
    }
    return $res;
}

function set_admin_blk_users( $adm_user_ids, $blk_user_ids, $user_id ) {
    # Can't de-admin yourself
    $query = "UPDATE users SET is_admin=false,is_blocked=false WHERE user_id != ?";
    $arr = db_query( $query, array($user_id) );

    # If this gets slow make a transaction
    $query = "UPDATE users SET is_admin=true WHERE user_id = ?";
    foreach( $adm_user_ids as $adm ) {
        $arr = db_query( $query, array($adm) );
    }
    # Can't block yourself
    $query = "UPDATE users SET is_blocked=true WHERE user_id = ? AND user_id != ?";
    foreach( $blk_user_ids as $blk ) {
        $arr = db_query( $query, array($blk, $user_id) );
    }
}

function check_figshare( $str ) {
    $matches = null;
    preg_match( "/[^a-zA-Z1234567890_-]/", $str, $matches, PREG_OFFSET_CAPTURE );
    if ( count($matches) === 0 ) { return $str; }
    else { return ""; }
}

function save_profile( $uid, $foafurl, $embargo, $pub_dspace, $pub_chempound,
                        $pub_figshare, $email) {

//  $figsharekey = check_figshare( $figsharekey );
//  $figsharesecret = check_figshare( $figsharesecret );
    $embargo = (int) $embargo ;
    if( $embargo < 0 )  {$embargo=0;}
    if( $embargo > 999 ) { $embargo=999; }

    # Not trying to find valid email here.  Just want something non-local.
    # Should match xxx@xxx.xxx[.xxx] strings.
    $email = trim($email);
    if( !preg_match( "/^.+[^\\\]@.+\..+$/", $email ) ) {
        $email = '';
    }
        

    $query = "UPDATE profile SET foafurl=?, embargo=?, pub_dspace=?, pub_chempound=?, pub_figshare=?,"
            . "email_address = ? WHERE user_id=?";
    $result = db_query( $query, array( $foafurl, $embargo, $pub_dspace,
                                $pub_chempound, $pub_figshare, $email, $uid) );

}

function add_profile( $uid ) {
    $query = "INSERT INTO profile (user_id, pub_dspace ) VALUES ( ?, true )";

    $result = db_query( $query, array($uid) );


}

function get_profile( $uid ) {


    $query = "SELECT * FROM profile WHERE user_id = ?";

    $arr = db_query( $query, array($uid) );

    $res = array();
    $res['name'] =  !empty($_SESSION['gecos']) ? $_SESSION['gecos'] : 'unknown';
    $res['foaf'] =  $arr['foafurl'];
    $res['orcid'] =  $arr["orcid"];
    $res['embargo'] =  $arr['embargo'];
//  $res['figsharekey'] =  $arr['figsharekey'];
    $res['figsharesecretstate'] =  !empty($arr['figsharesecret']) ? 'present' : 'absent';
    $res['pub_dspace']    = $arr['pub_dspace']    == 1 ? "checked" : "";
    $res['pub_chempound'] = $arr['pub_chempound'] == 1 ? "checked" : "";
    $res['pub_figshare']  = $arr['pub_figshare']  == 1 ? "checked" : "";
    $res['email'] = $arr['email_address'];
    return $res;
}


function preview_file( $jid, $idx, $user_id ) {
    /* first, we get the mime type of the file */
    $app_id = get_app_id_for_job($jid);
    $query = "SELECT mimetype FROM app_output WHERE app_id = ? AND index = ?";

    $arr = db_query( $query, array($app_id, $idx) );

    echo $mimetype;

    /* get the preview template associated with this mime type */
    $query = "SELECT template,linebreak FROM preview_templates WHERE mimetype = ?";
    $arr = db_query( $query, array($mimetype) );

    $template = $arr['template'];
    $linebreak = $arr['linebreak' ];

    if ($template==NULL ) {
        $template="default";
        $linebreak='<BR>';
    }

    /* next, we transfer the contents of the file */

    $output = return_job_output_file_str( $app_id, $jid, $idx );

#echo "$template";
#echo "<!--";
#echo implode( $linebreak,$output);
#echo "-->";

#   return array( $template, implode("<br>", $output) );
    return array( $template, implode( $linebreak, $output) );
}

function return_job_output_file_str( $app_id, $jid, $idx , $user_id ) {
    global $UP_options;

    $query ="SELECT * FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );


    $staging_dir = $arr['staging_dir' ];

    $UP_remote_cat            = $UP_options['remote_cat'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );

    $key = make_key_name_from_app ( $app_id );

    $cmdarray = array(  $UP_remote_cat,
                        "$staging_dir/output-$idx",
                    );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );

    $foo= array();
    exec( $cmd,  $foo );
    return $foo;
}

function get_motd( $user ) {
#   print_r( get_pools( $user ));
    $motd="";
    $pools = get_pools( $user );
    $i=0;
    foreach( $pools as $p ) {
#       print("<p> $i");
        $motd = $motd . pool_motd( $p['pool_id'] );
        $i++;
    }
    return $motd;

}

function get_job_description( $jid ) {
    $query="SELECT description FROM job_list WHERE job_id = ?";
    $arr= db_query($query, array($jid));
    return $arr['description'];
}

function set_job_description( $jid, $desc ) {
    $query="UPDATE job_list SET description = ? WHERE job_id = ?";
    $arr= db_query($query, array($desc, $jid));
}

function get_admin_email() {
    global $UP_options;

    return (empty($UP_options['admin_email']) ? 'admin' : $UP_options['admin_email']);
}

function is_project_empty( $project_id ) {
    $query="SELECT count(job_id) FROM job_list WHERE project_id = ? AND deleted=false";
    $arr = db_query($query, array($project_id));
    $cnt = $arr['count'];
    return $cnt == 0;
}

function tempdir() {
    global $UP_options;
    $dir = $UP_options['tempdir'] ? $UP_options['tempdir'] : '/tmp/';
    if (substr($dir, -1) != '/') $dir .= '/';

    $prefix = 'uportal';
    do
    {
      $path = $dir.$prefix.mt_rand(0, 9999999);
    } while (!mkdir($path, 0777));

    return $path;
}

function escapearguments( $args ) {
    $str = '';
    foreach( $args as $a ) {
        $str .= ' ' . escapeshellarg( $a );
    }
    return ltrim($str);
}

function make_ssh_cmd( $sshkey, $app_user, $app_host, $cmdarray ) {
    global $UP_options;
    $UP_ssh     = $UP_options['ssh'];
    $UP_ssh_flags = $UP_options['ssh_flags'];
    $UP_ssh_key = $UP_options['ssh_key_flag'];
    $ssharray = array(  $UP_ssh,
                        $UP_ssh_key,
                        $sshkey,
                        "$app_user@$app_host",
                    );
    foreach ( explode(',', $UP_ssh_flags) as $f ) {
        $ssharray[] = trim($f);
    }
    $cmd  = escapearguments( $ssharray);
    if( ! $cmdarray ) { die( 'Invalid command for ssh' ); };
    # Double escaped because it's running on the remote host
    $cmd .= ' ' . escapeshellarg( escapearguments($cmdarray) );
    return $cmd;
}

function make_scp_cmd( $sshkey, $cmdarray ) {
    global $UP_options;
    $UP_scp     = $UP_options['scp'];
    $UP_ssh_flags = $UP_options['ssh_flags'];
    $UP_ssh_key = $UP_options['ssh_key_flag'];
    $scparray = array(  $UP_scp,
                        $UP_ssh_key,
                        $sshkey,
                    );
    foreach ( explode(',', $UP_ssh_flags) as $f ) {
        $scparray[] = trim($f);
    }
    $cmd = escapearguments( array_merge($scparray, $cmdarray) );
    return $cmd;
}

function user_id_exists( $user_id ) {
    $query = "SELECT count(user_id) FROM users WHERE user_id = ?";
    $arr = db_query( $query, array($user_id) );
    return ($arr['count'] == 1);
}

function get_default_embargo( $user_id ) {
    $query = "SELECT embargo FROM profile WHERE user_id = ?";
    $arr = db_query( $query, array($user_id) );
    return $arr['embargo'];
}

function embargo_job( $jid, $embargo_days ) {
    $query = "UPDATE job_list SET embargo_date = current_date + CAST( ? AS INTEGER), embargo_status = 1, embargo_mail_sent = false WHERE job_id = ?";
    $arr = db_query( $query, array($embargo_days, $jid) );
}

function cancel_embargo( $jid ) {
    $query = "UPDATE job_list SET embargo_date = '0000-01-01', embargo_status = 0 WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );
}

function get_embargo_days( $jid ) {
    $query = 'SELECT embargo_date-current_date AS embargo, embargo_status FROM job_list WHERE deleted=false AND job_id = ?';
    $arr = db_query( $query, array($jid) );
    if( $arr['embargo_status'] == 0 ) {
        return NULL;
    } else {
        return $arr['embargo'];
    }
}

function set_embargo_mail_sent( $jid ) {
    $query = 'UPDATE job_list SET embargo_mail_sent = true WHERE job_id = ?';
    $arr = db_query( $query, array($jid) );
}


?>
