<?php 

require '../uportal/uportal-config/figshareconfig.inc';

function figshare_credentials( $user_id ) {
	global $dbconn;
	global $figshare_key, $figshare_secret;
	pg_prepare( $dbconn, "mfn2", "SELECT figsharekey, figsharesecret FROM profile WHERE userid = $1" );
	$results = pg_execute( $dbconn, "mfn2", array( $nonce ) );
        $arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	$res = array();

	$res[ 'consumer_key' ] = $figshare_key;
	$res[ 'consumer_secret' ] = $figshare_secret;
	$res[ 'token_key' ] = $arr['figsharekey' ];
	$res[ 'token_secret' ] = $arr['figsharesecret' ];
		
	if( empty( $res[ 'token_key' ] ) || empty( $res[ 'token_secret' ] ) ) {
		return NULL;
	}

	return $res;

	
}


function set_figshare_key_secret( $nonce, $key, $secret ) {
	
       global $dbconn;


	pg_prepare( $dbconn, "mfn0", "UPDATE profile SET figsharekey = $1, figsharesecret = $2 where figsharekey = $3" );
	$results = pg_execute( $dbconn, "mfn0", array( $key, $secret, $nonce ) );
}

function make_figshare_nonce( $user_id ) {
       global $dbconn;

	$nonce = md5( $user_id . " dfgjsgg " . time() );

	pg_prepare( $dbconn, "mfn1", "UPDATE profile SET figsharekey = $2, figsharesecret = NULL where userid = $1" );
	$results = pg_execute( $dbconn, "mfn1", array( $user_id, $nonce) );

	return $nonce;
	
}

function figshare_publish( $app_id, $jid, $user_id ) {
        # This is quick and dirty. Always assume job type is 'gaussian'
	
	# 1. Get figshare credentials
       global $dbconn;
	global $figshare_secret, $figshare_key;
        global $UP_config;
	pg_prepare( $dbconn, "fpq0", "SELECT figshare_is_published FROM job_list WHERE job_id = $1" );
	$results = pg_execute( $dbconn, "fpq0", array( $jid ) );
	if ( $results['figshare_is_published'] === true || $results['figshare_is_published'] === 't' ) { return; }

	pg_prepare( $dbconn, "fpq1", "SELECT figsharekey,figsharesecret FROM profile WHERE userid = $1" );
	$results = pg_execute( $dbconn, "fpq1", array( $user_id) );
   	$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	$tokkey     = $arr[ 'figsharekey' ];	
	$toksecret  = $arr[ 'figsharesecret' ];	

	if( $tokkey === "" || $toksecret === ""  || $tokkey === null || $toksecret === null ) {
print "<!--No FIGSHARE credentials [$tokkey] [$toksecret]-->\n";
		return;
	}
	
	#2. Get job directory
	pg_prepare( $dbconn, "fpq2", "SELECT staging_dir FROM job_list WHERE job_id = $1" );
	$results = pg_execute( $dbconn, "fpq2", array( $jid ) );
        $arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

        $staging_dir = $arr['staging_dir' ];

	#3. run teh publish-figshare script

        $UP_ssh                = $UP_config['ssh'];
        $UP_ssh_key                 = $UP_config['ssh_key'];
        $UP_ssh_keydir           = $UP_config['ssh_keydir'];
        $UP_publish              = $UP_config['publish-figshare'];

        $app_user = get_app_user( $dbconn, $app_id );
        $app_host = get_app_host( $dbconn, $app_id );
        $app_path = get_app_path( $dbconn, $app_id );

	$project_id = get_project_by_jid( $jid );
	$project  = get_project_name( $user_id, $project_id );
	$project = str_replace( " ", "_" , $project );
	$project = str_replace( "'", "" , $project );
	$project = str_replace( '"', "" , $project );
	$project = str_replace( '\\', "" , $project );

        $key = make_key_name( $app_id );

        $cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key . ' '. $app_user.'@'.$app_host;
        $cmd = $cmd . ' ' . $app_path. '/' . $UP_publish . ' '. $staging_dir . ' gaussian ' . $figshare_key. ' '. $figshare_secret. ' '. $tokkey. ' ' . $toksecret . ' '. $project;

        $foo= array();
        exec( $cmd,  $foo );
	echo "<!-- $cmd -->";
	print_r( $foo );

	if( strstr( $foo[0], "DOI:" ) != FALSE ) {
		$url = str_replace( "DOI: ", "", $foo[0] );

		pg_prepare( $dbconn, "fpq3", "UPDATE job_list SET figshare_handle=$1, figshare_is_published=true, is_published=true WHERE job_id = $2" ); 
	 	$results = pg_execute( $dbconn, "fpq3", array( $url, $jid ) );
		return $url;

	}
	else {
		return null;
	}
	

}

function figshare_make_public( $user_id, $jid ) {
        global $dbconn;
	global $figshare_secret, $figshare_key;
        global $UP_config;
	pg_prepare( $dbconn, "fpq1", "SELECT figsharekey,figsharesecret FROM profile WHERE userid = $1" );
	$results = pg_execute( $dbconn, "fpq1", array( $user_id) );
   	$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	$tokkey     = $arr[ 'figsharekey' ];	
	$toksecret  = $arr[ 'figsharesecret' ];	

	if( $tokkey === "" || $toksecret === ""  || $tokkey === null || $toksecret === null ) {
print "No FIGSHARE credentials\n"; # [$fskey] [$fssecret]";
		return;
	}

	# Get the Figshare ID
	$query = "SELECT figshare_handle FROM job_list WHERE job_id = $jid";
	$res = pg_fetch_row(  pg_query( $query ) );
	$fid = $res[0];
#print_r ($res);
#print "<br> $fid";
	if( !strstr( $fid, "figshare" ) ) {
		$cmd = "/var/www/uportal/bin/make_public.py -k ". $figshare_key . " ";
		$cmd = $cmd . " -s " . $figshare_secret;	
		$cmd = $cmd . " -K " . $tokkey;	
		$cmd = $cmd . " -S " . $toksecret;	
		$cmd = $cmd . " -a ". $fid;
		exec( $cmd, $output );
#		print $cmd;
#		print "<br>";
#		print_r( $output );
		foreach( $output as $temp ) {
			if( strstr( $temp, "DOI:" ) != FALSE ) {
				$url = str_replace( "DOI: ", "", $temp );
#				printf("<a href='http://dx.doi.org/$url'>$url</a>");
#				Add to the job list
				pg_prepare( $dbconn, "fmp1", "UPDATE job_list SET figshare_handle = $1 WHERE job_id = $2"); 
				$results = pg_execute( $dbconn, "fmp1", array( $url, $jid) );
#echo $query;
			}
		}
#exit;	
header('Location: /?action=joblist');

	}
	
}

function chempound_publish( $app_id, $jid, $user_id ) {
	# This function must:
	# retrieve output files with mime-type application/x-mets-archive
	# Pass them on to the DSpace LNI interface
	# Retrieve the pseudo-handle returned bu the importer
	# update the db


	global $dbconn;
	global $UP_config;

	$res=false;

	# Get index of mets archive

	$index = output_file_index_of_mimetype(  $app_id, 'chemical/x-gaussian-log' );

	if( $index< 0 ) { echo "No publishable content"; return; }
	
   # Now we make the temporary dspace staging directory

   exec( "mktemp -d", $result );

   $tmpdir = $result[0];
	$tmpfile= $tmpdir."/output.log";


	copy_job_file( $dbconn,  $jid, "output-".$index, $tmpfile, $user_id );
	
	$cmd = $UP_config['chempound_importer'] . ' '. $tmpfile;

	exec( $cmd, $output );


	$handle = $output[0];
print_r( $output );

	if( $handle != "" ) {
		$query="UPDATE job_list SET chempound_is_published=true, chempound_url='" . sanify( $handle ) . "' WHERE job_id = ". sanify($jid );
		pg_query( $query );
		$res = true;
	}

	exec( "rm -rf $tmpdir" );
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


	global $dbconn;
	global $UP_config;

	$res=false;

	# Get index of mets archive

	$index = output_file_index_of_mimetype(  $app_id, 'application/x-mets-archive' );

	if( $index< 0 ) { return; }
	
   # Now we make the temporary dspace staging directory

   exec( "mktemp -d", $result );

   $tmpdir = $result[0];
	$tmpfile= $tmpdir."/mets.zip";


	copy_job_file( $dbconn,  $jid, "output-".$index, $tmpfile, $user_id );
	
	$cmd = $UP_config['dspace_importer'] . ' '. $tmpfile;

	exec( $cmd, $output );


	$handle = $output[0];


	if( $handle != "" ) {
		$query="UPDATE job_list SET is_published=true, handle='" . sanify( $handle ) . "' WHERE job_id = ". sanify($jid );
		pg_query( $query );
		$res = true;
	}
	else {
		$query="UPDATE job_list SET tried_to_publish=true WHERE job_id =" .sanify($jid);
		pg_query( $query );
		$res = false;
	}

	exec( "rm -rf $tmpdir" );
	if( $res == true ) {
		remove_job_file( $dbconn, $jid, "output-".$index, $tmpfile, $user_id );
		return $handle;
	}
	else {
		return null;
	}
}

function figshare_has_credentials( $uid ) {
	if( figshare_credentials( $uid) == NULL ) { return NULL; }
	return true;
}

function figshare_uploader_upload( $user_id ) {
	# 1. Get figshare credentials
       global $dbconn;
	global $figshare_secret, $figshare_key;
        global $UP_config;
	pg_prepare( $dbconn, "fpq1", "SELECT figsharekey,figsharesecret FROM profile WHERE userid = $1" );
	$results = pg_execute( $dbconn, "fpq1", array( $user_id) );
   	$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	$tokkey     = $arr[ 'figsharekey' ];	
	$toksecret  = $arr[ 'figsharesecret' ];	

	if( $tokkey === "" || $toksecret === ""  || $tokkey === null || $toksecret === null ) {
print "No FIGSHARE credentials [$fskey] [$fssecret]";
		return;
	}

	$projectid = sanify($_REQUEST['project']);
	$porjectid= (int)($projectid);
	$project = get_project_name( $user_id, $projectid );

        $UP_publish              = $UP_config['publish-figshare-local'];

        $cmd =  $UP_publish . ' ' . $figshare_key. ' '. $figshare_secret. ' '. $tokkey. ' ' . $toksecret ;

	$cmd = "/var/www/uportal/bin/create.py -k ". $figshare_key . " ";
	$cmd = $cmd . " -g ImperialCollege ";
	$cmd = $cmd . " -g " . escapeshellarg($project);
	$cmd = $cmd . " -s " . $figshare_secret;	
	$cmd = $cmd . " -K " . $tokkey;	
	$cmd = $cmd . " -S " . $toksecret;	
	$cmd = $cmd . " -t " . escapeshellarg($_REQUEST['title']);
	$cmd = $cmd . " -d " . escapeshellarg($_REQUEST['description']);
	if( $project ) {
		$cmd = $cmd . " -p " . escapeshellarg($project) ;
	}



	if( ($_REQUEST['title'] === "" ) ) {
		printf("No title");
		exit(0);
	}
	if( ($_REQUEST['description'] === "") ) {
		printf("No description");
		exit(0);
	}

	$i=0;
	$files_for_processing = "/var/www/uportal/bin/get_inchi ";
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
			$cmd = $cmd . " -f " . $tmpfile . ":" .  $file;
			$i++;
		}
		$files_for_processing = $files_for_processing . " ". escapeshellarg( $tmpfile );
	}

        exec( $files_for_processing,  $foo );

	foreach( $foo as $temp ) {
		$cmd = $cmd . " -g " . escapeshellarg( $temp ) ;
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
	$cmd = $cmd . " -c $cat";

#	echo "<p>";
#	passthru( "$cmd 2>&1" );
        exec( $cmd,  $bar );
#	print_r( $foo );
#	echo "<p>";
#	echo "$cmd ";
#	echo "\n\n<p>\n\n";
#	echo "<p>";
#	print_r( $bar );
#	echo "-->";

	foreach( $bar as $temp ) {
		if( strstr( $temp, "DOI:" ) != FALSE ) {
		$url = str_replace( "DOI: ", "", $temp );

		printf("<a href='http://dx.doi.org/$url'>$url</a>");
#		Add to the job list
		$jobid=get_next_job_id();
		$title = $_REQUEST['title'];
		$query="INSERT INTO job_list( job_id, app_id, userid, description, staging_dir, is_published, project_id, status, figshare_is_published, figshare_handle ) VALUES ( ". $jobid .", 0 , ". $user_id . ", '" . sanify($title) . "', '0' , true, ". sanify($projectid) . ", 'C', true, '" . sanify($url) . "' )";
#echo $query;
		pg_query( $query );
		}
	}
	
	header('Location: /?action=joblist&orderby=0&orderdir=0&byproject='. $projectid);


}
function figshare_uploader_show() {
}



?>
