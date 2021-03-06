<?php

require_once 'db-functions.inc';
require_once 'configuration2.inc';
require_once 'uportal-functions2.inc';

function figshare_credentials( $user_id ) {
    global $UP_config;

    #pg_prepare( $dbconn, "mfn2", "SELECT figsharekey, figsharesecret FROM profile WHERE userid = $1" );
    $query = "SELECT figsharekey, figsharesecret FROM profile WHERE user_id = ?";
    #SJC nonce is bug here, presumably?
    #$results = pg_execute( $dbconn, "mfn2", array( $nonce ) );
    $arr = db_query( $query, array($user_id) );

    $res = array();
    $res[ 'consumer_key' ] = $UP_config['figshare_key'];
    $res[ 'consumer_secret' ] = $UP_config['figshare_secret'];
    $res[ 'token_key' ] = $arr['figsharekey' ];
    $res[ 'token_secret' ] = $arr['figsharesecret' ];

    if( empty( $res[ 'token_key' ] ) || empty( $res[ 'token_secret' ] ) ) {
        return NULL;
    }

    return $res;


}


function set_figshare_key_secret( $nonce, $key, $secret ) {

    #pg_prepare( $dbconn, "mfn0", "UPDATE profile SET figsharekey = $1, figsharesecret = $2 where figsharekey = $3" );
    $query = "UPDATE profile SET figsharekey = ?, figsharesecret = ? where figsharekey = ?";
    #$results = pg_execute( $dbconn, "mfn0", array( $key, $secret, $nonce ) );
    $arr = db_query( $query, array($key, $secret, $nonce) );
}

function make_figshare_nonce( $user_id ) {

    $nonce = md5( $user_id . " dfgjsgg " . time() );

    #pg_prepare( $dbconn, "mfn1", "UPDATE profile SET figsharekey = $2, figsharesecret = NULL where userid = $1" );
    $query = "UPDATE profile SET figsharekey = ?, figsharesecret = NULL where user_id = ?";
    #$results = pg_execute( $dbconn, "mfn1", array( $user_id, $nonce) );
    $result = db_query( $query, array($nonce, $user_id) );

    return $nonce;

}

function figshare_publish( $app_id, $jid, $user_id ) {
    # This is quick and dirty. Always assume job type is 'gaussian'

    # 1. Get figshare credentials
    global $UP_config;

    #pg_prepare( $dbconn, "fpq0", "SELECT figshare_is_published FROM job_list WHERE job_id = $1" );
    #$results = pg_execute( $dbconn, "fpq0", array( $jid ) );
    $query = "SELECT figshare_is_published FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );

    if ( $arr['figshare_is_published'] === true || $arr['figshare_is_published'] === 't' ) { return; }

    #pg_prepare( $dbconn, "fpq1", "SELECT figsharekey,figsharesecret FROM profile WHERE userid = $1" );
    #$results = pg_execute( $dbconn, "fpq1", array( $user_id) );
    $query = "SELECT figsharekey,figsharesecret FROM profile WHERE user_id = ?";
    $arr = db_query( $query, array($user_id) );
    $tokkey     = $arr[ 'figsharekey' ];
    $toksecret  = $arr[ 'figsharesecret' ];

    if( $tokkey === "" || $toksecret === ""  || $tokkey === null || $toksecret === null ) {
        return;
    }

    #2. Get job directory
    #pg_prepare( $dbconn, "fpq2", "SELECT staging_dir FROM job_list WHERE job_id = $1" );
    #$results = pg_execute( $dbconn, "fpq2", array( $jid ) );
    $query = "SELECT staging_dir FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );

    $staging_dir = $arr['staging_dir' ];

    #3. run teh publish-figshare script

    $UP_publish              = $UP_config['publish-figshare'];

    $app_user = get_app_user( $app_id );
    $app_host = get_app_host( $app_id );
    $app_path = get_app_path( $app_id );

    $project_id = get_project_by_jid( $jid );
    $project  = get_project_name( $user_id, $project_id );
    $project = str_replace( " ", "_" , $project );
    $project = str_replace( "'", "" , $project );
    $project = str_replace( '"', "" , $project );
    $project = str_replace( '\\', "" , $project );

    $key = make_key_name_from_app( $app_id );

    $cmdarray = array(
        "$app_path/$UP_publish",
        $staging_dir,
        'gaussian',
        $UP_config['figshare_key'],
        $UP_config['figshare_secret'],
        $tokkey,
        $toksecret,
        $project,
        );
    $cmd = make_ssh_cmd( $key, $app_user, $app_host, $cmdarray );
    $foo= array();
    exec( $cmd,  $foo );
    echo "<!-- $cmd -->";
    print_r( $foo );

    if( strstr( $foo[0], "DOI:" ) != FALSE ) {
        $url = str_replace( "DOI: ", "", $foo[0] );

        #pg_prepare( $dbconn, "fpq3", "UPDATE job_list SET figshare_handle=$1, figshare_is_published=true, is_published=true WHERE job_id = $2" );
        #$results = pg_execute( $dbconn, "fpq3", array( $url, $jid ) );
        $query = "UPDATE job_list SET figshare_handle=?, figshare_is_published=true, is_published=true WHERE job_id = ?";
        $arr = db_query( $query, array($url, $jid) );
        return $url;

    }
    else {
        return null;
    }


}

function figshare_make_public( $user_id, $jid ) {
    global $UP_config;

    #pg_prepare( $dbconn, "fpq1", "SELECT figsharekey,figsharesecret FROM profile WHERE userid = $1" );
    #$results = pg_execute( $dbconn, "fpq1", array( $user_id) );
    $query = "SELECT figsharekey,figsharesecret FROM profile WHERE user_id = ?";
    $arr = db_query( $query, array($user_id) );
    $tokkey     = $arr[ 'figsharekey' ];
    $toksecret  = $arr[ 'figsharesecret' ];

    if( $tokkey === "" || $toksecret === ""  || $tokkey === null || $toksecret === null ) {
        print "figshare_make_public: No FIGSHARE credentials\n"; # [$fskey] [$fssecret]";
        return;
    }

    # Get the Figshare ID
    #$query = "SELECT figshare_handle FROM job_list WHERE job_id = $jid";
    #$res = pg_fetch_row(  pg_query( $query ) );
    $query = "SELECT figshare_handle FROM job_list WHERE job_id = ?";
    $arr = db_query( $query, array($jid) );
    $fid = $arr['figshare_handle'];
#print_r ($res);
#print "<br> $fid";
    if( !strstr( $fid, "figshare" ) ) {
        $cmdarray = array(
            '/var/www/uportal/bin/make_public.py',
            '-k', $UP_config['figshare_key'],
            '-s', $UP_config['figshare_secret'],
            '-K', $tokkey,
            '-S', $toksecret,
            '-a', $fid,
            );
        $cmd = escapearguments( $cmdarray );

        exec( $cmd, $output );
#       print $cmd;
#       print "<br>";
#       print_r( $output );
        foreach( $output as $temp ) {
            if( strstr( $temp, "DOI:" ) != FALSE ) {
                $url = str_replace( "DOI: ", "", $temp );
#               printf("<a href='http://dx.doi.org/$url'>$url</a>");
#               Add to the job list
                #pg_prepare( $dbconn, "fmp1", "UPDATE job_list SET figshare_handle = $1 WHERE job_id = $2");
                #$results = pg_execute( $dbconn, "fmp1", array( $url, $jid) );
                $query = "UPDATE job_list SET figshare_handle = ? WHERE job_id = ?";
                $arr = db_query( $query, array($url, $jid) );
#echo $query;
            }
        }
#exit;  
    $proto = $UP_config['protocol'];
    $host  = $_SERVER['HTTP_HOST'];
    $uri   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');
    $extra = '?action=joblist';
    header("Location: $proto://$host$uri/$extra");

    }

}

function chempound_publish( $app_id, $jid, $user_id ) {
    # This function must:
    # retrieve output files with mime-type application/x-mets-archive
    # Pass them on to the DSpace LNI interface
    # Retrieve the pseudo-handle returned bu the importer
    # update the db


    global $UP_config;

    $res=false;

    # Get index of mets archive

    $index = output_file_index_of_mimetype(  $app_id, 'chemical/x-gaussian-log' );

    if( $index< 0 ) { echo "No publishable content"; return; }

   # Now we make the temporary dspace staging directory

    $tmpdir = tempdir();
    $tmpfile= $tmpdir."/output.log";


    copy_job_file( $jid, "output-".$index, $tmpfile, $user_id );

    $cmdarray = array( $UP_config['chempound_importer'], $tmpfile );
    $cmd = escapearguments( $cmdarray );

    exec( $cmd, $output );


    $handle = $output[0];
    print_r( $output );

    if( $handle != "" ) {
        $query="UPDATE job_list SET chempound_is_published=true, chempound_url = ? WHERE job_id = ?";
        $arr = db_query( $query, array($handle, $jid) );
        $res = true;
    }

    unlink( $tmpfile );
    rmdir( $tmpdir );

    if( $res == true ) {
        return $handle;
    }
    else {
        return null;
    }
}

function dspace_publish(  $app_id, $jid, $user_id ) {

    # This function must:
    # retrieve output files with mime-type application/x-mets-archive
    # Pass them on to the DSpace LNI interface
    # Retrieve the pseudo-handle returned bu the importer
    # update the db


    global $UP_config;

    $res=false;

    # Get index of mets archive

    $index = output_file_index_of_mimetype(  $app_id, 'application/x-mets-archive' );

    if( $index< 0 ) { return; }

    # Now we make the temporary dspace staging directory

    $tmpdir = tempdir();
    $tmpfile= $tmpdir."/mets.zip";


    copy_job_file( $jid, "output-".$index, $tmpfile, $user_id );

    $cmdarray = array( $UP_config['dspace_importer'], $tmpfile);
    $cmd = escapearguments( $cmdarray );

    exec( $cmd, $output );


    $handle = $output[0];


    if( $handle != "" ) {
        $query="UPDATE job_list SET is_published=true, handle=? WHERE job_id = ?";
        $result = db_query( $query, array($handle, $jid) );
        $res = true;
    }
    else {
        $query="UPDATE job_list SET tried_to_publish=true WHERE job_id = ?";
        $result = db_query( $query, array($jid) );
        $res = false;
    }

    if( $res == true ) {
        remove_job_file( $jid, "output-".$index, $tmpfile );
        return $handle;
    }
    else {
        return null;
    }

    unlink( $tmpfile );
    rmdir( $tmpdir );
}

function figshare_has_credentials( $uid ) {
    if( figshare_credentials( $uid) == NULL ) { return NULL; }
    return true;
}

function figshare_uploader_upload( $user_id, $projectid ) {
    # 1. Get figshare credentials
    global $UP_config;

    $query = "SELECT figsharekey,figsharesecret FROM profile WHERE user_id = ?";
    $arr = db_query( $query, array($user_id) );

    $tokkey     = $arr[ 'figsharekey' ];
    $toksecret  = $arr[ 'figsharesecret' ];

    if( $tokkey === "" || $toksecret === ""  || $tokkey === null || $toksecret === null ) {
print "figshare_uploader_upload: No FIGSHARE credentials [$fskey] [$fssecret]";
        return;
    }

    if( empty($_REQUEST['title']) ) {
        printf("No title");
        exit(0);
    }
    if( empty($_REQUEST['description']) ) {
        printf("No description");
        exit(0);
    }

    $cmdarray = array(  '/var/www/uportal/bin/create.py',
                        '-k', $UP_config['figshare_key'],
                        '-s', $UP_config['figshare_secret'],
                        '-K', $tokkey,
                        '-S', $toksecret,
                        '-t', $_REQUEST['title'],
                        '-d', $_REQUEST['description'],
                    );

    if ( !empty($UP_config['figshare_tags']) ) {
        foreach (explode( ',', $UP_config['figshare_tags']) as $a) {
            $cmdarray[] = '-g';
            $cmdarray[] = trim ($a);
        }
    }

    $project = get_project_name( $user_id, $projectid );

    if( $project ) {
        $cmdarray[] = '-p';
        $cmdarray[] = $project;
        $cmdarray[] = '-g';
        $cmdarray[] = $project;
    }



    $i=0;
    $processing_cmd_array = array( '/var/www/uportal/bin/get_inchi' );
    print_r( $_FILES );
    foreach( $_FILES as $temp ) {

        $tmpfile = $temp[ 'tmp_name' ];
        if( file_exists( $tmpfile ) ) {
             $file    = $temp[ 'name' ];
            $file = preg_replace( "/\//", "", $file );
            $file = preg_replace( "/\s/", "", $file );
            $file = preg_replace( "/\.\./", "", $file );
            $file = preg_replace( "/\n/", "", $file );
            $file = preg_replace( "/\r/", "", $file );
#            $cmd = $cmd . " -f " . $tmpfile . ":" .  $file;
            $cmdarray[] = '-f';
            $cmdarray[] = "$tmpfile:$file";
            $i++;
        }
#        $files_for_processing = $files_for_processing . " ". escapeshellarg( $tmpfile );
        $processing_cmd_array[] = $tmpfile;
    }

    $files_for_processing = escapearguments( $processing_cmd_array );
    exec( $files_for_processing,  $foo );

    foreach( $foo as $temp ) {
        $cmdarray[] = '-g';
        $cmdarray[] = $temp;
    }


    if( $i===0 ) {
        printf("No files\n");
        exit(0);
    }

    $cat=1;
    if( array_key_exists( 'category', $_REQUEST  ) ) {
        $cat = $_REQUEST['category'];
        $cat = intval($cat);
        if( $cat <= 0 ) {
            $cat=1;
        }
    }
    #$cmd = $cmd . " -c $cat";
    $cmdarray[] = '-c';
    $cmdarray[] = $cat;

#   echo "<p>";
#   passthru( "$cmd 2>&1" );

    $cmd = escapearguments( $cmdarray );
    exec( $cmd,  $bar );

#   print_r( $foo );
#   echo "<p>";
#   echo "$cmd ";
#   echo "\n\n<p>\n\n";
#   echo "<p>";
#   print_r( $bar );
#   echo "-->";

    foreach( $bar as $temp ) {
        if( strstr( $temp, "DOI:" ) != FALSE ) {
            $url = str_replace( "DOI: ", "", $temp );

            printf("<a href='http://dx.doi.org/$url'>$url</a>");
            # Add to the job list
            $jobid=get_next_job_id();
            $title = $_REQUEST['title'];
            $query="INSERT INTO job_list( job_id, app_id, user_id, description, staging_dir, is_published, project_id, status, figshare_is_published, figshare_handle ) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )";
    #echo $query;
            $result = db_query( $query, array($jobid, 0, $user_id, $title, 0 , true, $projectid, 'C', true, $url) );
        }
    }
    
    $proto = $UP_config['protocol'];
    $host  = $_SERVER['HTTP_HOST'];
    $uri   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');
    $extra = '/?action=joblist&orderby=0&orderdir=0&byproject='. $projectid;
    header("Location: $proto://$host$uri/$extra");


}
function figshare_uploader_show() {
}



?>
