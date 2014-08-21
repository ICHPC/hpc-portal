<?

require_once('domxml-php4-to-php5.php');

require 'configuration2.inc';
require 'auth-functions.inc';

$dbconn = pg_connect($UP_config['conn_string']);

# Go through all methods in $UP_config['auth_methods']
# attempting to authenticate with each one.
#
# If $UP_config['auth_type'] is strict then all methods must succeed,
# if it's permissive then any must succeed.
#
# The user's fullname is taken from the first successful method.
# A method cannot return an empty name as this is detected as an auth failure.
#
# Returns the fullname if authentication was successful, NULL otherwise.
function authenticate($user, $password)
{
    global $UP_config;
    $methods = array_map('trim',explode(',',$UP_config['auth_methods']));
    switch (trim($UP_config['auth_type']))
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
            print "doing ldap\n";
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
	global $dbconn;
	$query="SELECT project_id FROM job_list WHERE job_id = ". sanify( $jobid );
	$result= pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	return $arr['project_id'];
}


function get_job_owner( $jobid ) {
	global $dbconn;
	$query="SELECT userid FROM job_list WHERE job_id = ". sanify( $jobid );
	$result= pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	return $arr['userid'];
}

function get_job_project_name( $jobid ) {
	global $dbconn;
	$uid = get_job_owner( $jobid );
	$pid = get_job_project( $jobid );
	return get_project_name( $uid, $pid );

}

function get_next_job_id() {
	global $dbconn;
	$query="SELECT nextval('job_seQ_id')";
	$result  = pg_query( $dbconn, $query );

	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	return $arr['nextval'];

}

function new_get_job_list( $user, $order, $orderdir=0, $project_id=-1, $limit=10, $offset=0, $filter="" ) {
	global $dbconn;
	$user =sanify( $user );
	$uid = get_uid( $user );

	$limit = (int) sanify($limit);
	$offset= (int) sanify($offset);
	$filter = sanify($filter);

	$statidx = array ('I' => 'Pending', 'R' => 'Running', 'C' => 'Finished', 'X' => 'Deleted', 'U'=>'Pending', 'H' => 'Paused', '?' => 'Not Queued', '!' => 'Failed' );

	$orderby = array ( 'job_id' , 'a.app_id' , 'submit_time', 'wall_time',  'status', 'is_published' );
	

	if( $orderdir==0 ) { $orderdirstr='ASC';$otherdir=1; }
	else { $orderdirstr='DESC';  $otherdir=0;  };
	
	if( !empty($order)  && $order >=0 && $order < count($orderby ) ) { $orderbystr = $orderby[$order]; }
	else { $orderbystr = $orderby[0]; }

	$uid = sanify ( $uid );

	if ( $project_id > -1 ) { 
		$projectclause = "AND project_id = ". (int) sanify($project_id);
	}
	else {		
		$projectclause="";
	}

	$filter = str_replace( "*", "%", $filter );
	$filter = str_replace( "?", "%", $filter );
	if( $filter != "" ) {
		$filterclause = " AND description ILIKE '%" . $filter ."%'";
	}
	else {
		$filterclause = "";
	}

	$query = "SELECT a.*, b.app_id, b.app_name FROM job_list a, app_list b WHERE userid = " . $uid . " AND deleted=false and a.app_id = b.app_id " . $projectclause . " " . $filterclause ." ORDER BY ".$orderbystr. " ".  $orderdirstr . " LIMIT $limit OFFSET $offset";

#echo "<!-- $query -->";

	$result  = pg_query( $dbconn, $query );

	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	$list = array();

	if( $arr != NULL ) {


//echo $query;

	while( $arr != NULL ) {

		 $job_array=array();

		$is_complete=0;

                # Get job status, updating from server if necessary
		$jid =  $arr[ 'job_id' ];

		$job_array['jid'] =$jid ;

		$stat = get_job_status( $dbconn, $arr['staging_dir'], $jid , $user );

	


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


			$job_array['input_files'] = list_input_files( $dbconn, $jid, $arr['app_id'] ) ;

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
				$job_array[ 'output_files']= list_output_files( $dbconn, $jid, $arr['app_id'] , false ) ;
				$is_complete=1;
			}
			else if ($stat[0] == 'R' ) {
				$job_array[ 'output_files'] = list_output_files( $dbconn, $jid, $arr['app_id'] , true )  ;
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
#			echo "</TD>";

#			echo "<TD>";
#			echo '<A HREF="deleteJob.php?jid='.$arr['job_id'].'">Delete</a>';
#			echo "</TD>";

#		echo "</TR>\n";

		$list[] = $job_array;
		
		$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	}

	return $list;
	}
	else {
		return null;
	}

	


}

function is_required_input_file( $app_id , $idx ) {
	global $dbconn;
	$query="SELECT required from app_input where app_id= ". sanify($app_id) . " and index= ". sanify($idx );

	$result  = pg_query( $dbconn, $query );

	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	if ($arr['required'] == 'f' ) { return false; }
	else { return true; }

}

function is_admin( $user ) {
	global $dbconn;

	$user = sanify( $user );
	$query = "SELECT is_admin FROM users WHERE username = '" . $user ."'";

	$result  = pg_query( $dbconn, $query );

	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	if ($arr['is_admin'] == 'f' ) { return false; }
	else { return true; }
	
}


function check_user_registration(  $user ) {
	global $dbconn;
	
	$user = sanify( $user );
	
	if( ! get_uid(  $user ) )  {
		add_user( $dbconn, $user );
		return 1;
	}	
	add_profile( $user );
	return 0;
}

function add_user($dbconn,  $dn ) {
	$query = "INSERT INTO users ( username ) values ( '" . sanify($dn) . "')";
//echo $query;
	$result = pg_query( $dbconn, $query );


	$query = "SELECT userid FROM users where username = '".sanify($dn)."'";
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	$userid = $arr['userid'];
	
	$query ="SELECT * from pools WHERE public = true";
	$result = pg_query( $dbconn, $query );

	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	while( $arr!=NULL ) {
		$q2 = "INSERT INTO acl ( user_id, pool_id ) VALUES ( ".$userid.", ". $arr['pool_id'] .")";
		pg_query( $dbconn,  $q2 );
		$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	}

	add_profile(  $userid );
}

function owns_job( $dbconn, $uid, $jid ) {
	$query = "SELECT * FROM job_list WHERE userid = ".$uid." AND job_id = ".$jid;
	$result = pg_query( $dconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( $arr != NULL ) { return true; }
	else  { return false; }
}


function owns_project( $uid , $project_id ) {
	global $dbconn;
	$uid = (int) sanify( $uid );
	$project_id = (int) sanify( $project_id );

	$query = "SELECT * from projects where userid = ". $uid. " AND project_id = " .$project_id ;

	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	
	return ( $arr != NULL );
}

function delete_project( $uid , $project_id ) {
	global $dbconn;
	$uid = (int) sanify( $uid );
	$project_id = (int) sanify( $project_id );

	$query = "delete from projects where userid = ". $uid. " AND project_id = " .$project_id ;
	pg_query( $dbconn, $query );
}
	
function add_project( $uid , $description ) {
	global $dbconn;
	$uid = sanify( $uid );
	$description = sanify( $description );

	$query = "insert into projects ( userid, description ) values ( ". $uid . " , '" . $description . "') ";
	pg_query( $dbconn, $query );
}

function get_project_by_jid( $jid ) {
	global $dbconn;
	$query = "SELECT project_id from job_list where job_id = ". (int)  sanify( $jid ) ;

	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
   if($arr!=NULL ) {
		return $arr['project_id'];
   }
	else { 
		return NULL;
	}

}

function get_project_name( $uid, $project_id ) {
	global $dbconn;

	$query = "SELECT * from projects where userid = ". (int)  sanify( $uid ) . " AND project_id = " . (int) sanify( $project_id );

	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

   if($arr!=NULL ) {
		return $arr['description'];
   }
	else { 
		return NULL;
	}
}
	
function get_projects( $uid ) {
	global $dbconn;

	$query = "SELECT * from projects where userid = ". sanify( $uid ) . "ORDER BY description ASC";
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

   if($arr!=NULL ) {
      while( $arr!=NULL ) {
         $alist[] =  $arr['description'];
         $vlist[] = $arr['project_id'];
   
         $arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
      } 
   }

	if( !empty ( $alist ) ) {
		return  array( 'description' => $alist, 'project_id' => $vlist );
	} else {
		return array();
	}
}


function app_list(   $pool_id ) {
	global $dbconn;
	$pool_id = sanify( $pool_id );

	$query="select * from app_list WHERE pool_id = ". $pool_id . "AND CURRENT=true";

	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	
	$alist = array();
	$vlist = array();
	if($arr!=NULL ) {
		while( $arr!=NULL ) {
			$alist[] =  $arr['app_name'];
			$vlist[] = $arr['app_id'];

			$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
		}
	}
	return array( 'description' => $alist , 'app_id' => $vlist );
}

function get_foaf (  $uid ) {
	global $dbconn;
	$query = 'SELECT foafurl FROM profile WHERE userid = '. sanify($uid);
	$result  = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	return $arr[ 'foafurl' ];
}

function get_orcid (  $uid ) {
	global $dbconn;
	$query = 'SELECT orcid FROM users WHERE userid = '. sanify($uid);
	$result  = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	return $arr[ 'orcid' ];
}



function get_uid (  $uname ) {
	global $dbconn;
	$query = 'SELECT userid FROM users WHERE username ~* \'' . sanify($uname) .'\'' ;
	$result  = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	return $arr[ 'userid' ];
}

function get_jobs( $dbconn, $uid ) {
	$query = 'SELECT job_list FROM users WHERE userid = ' . sanify($uid). ' ORDER BY job_id DESC' ;
	$result  = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	return $arr;

}
function list_input_files( $dbconn, $jid, $app_type ) {
	$result = pg_query( $dbconn, 'SELECT * FROM app_input WHERE app_id = ' .$app_type . ' ORDER BY index ASC' );



	$count=0;


	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	$file_list = array();

	if( $arr!=NULL ) {	
	while( $arr != NULL ) {
		$file_list[] =   $arr['description'];
		$count++;
		$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	}
	}

	return $file_list;
}

function list_output_files( $dbconn, $jid , $app_type , $is_running) {
	if( $is_running ) {

		$result = pg_query( $dbconn, 'SELECT * FROM app_output WHERE app_id = ' .$app_type . ' AND available_at_runtime=true ORDER BY index ASC' );
	}
	else {
		$result = pg_query( $dbconn, 'SELECT * FROM app_output WHERE app_id = ' .$app_type . ' ORDER BY index ASC' );
	}

	$file_list = array();

	$count=0;


	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	if( $arr!=NULL ) {	
	while( $arr != NULL ) {
		$file_list[$count] = $arr['description'];			

		$count++;
		$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	}
	
	if( $count!=0 ) {
		$file_list[-1] = 'Tarball';

	}

	}

	return $file_list;
}

function sanify(  $a ) {

$search = array ('@<script[^>]*?>.*?</script>@si', // Strip out javascript
                 '@<[\/\!]*?[^<>]*?>@si',          // Strip out HTML tags
                 '@([\r\n])[\s]+@',                // Strip out white space
                 '@&(quot|#34);@i',                // Replace HTML entities
                 '@&(amp|#38);@i',
                 '@&(lt|#60);@i',
                 '@&(gt|#62);@i',
                 '@&(nbsp|#160);@i',
                 '@&(iexcl|#161);@i',
                 '@&(cent|#162);@i',
                 '@&(pound|#163);@i',
                 '@&(copy|#169);@i',
                 '@&#(\d+);@e');                    // evaluate as php

$replace = array ('',
                 '',
                 '\1',
                 '"',
                 '&',
                 '<',
                 '>',
                 ' ',
                 chr(161),
                 chr(162),
                 chr(163),
                 chr(169),
                 'chr(\1)');

	$text = preg_replace($search, $replace, $a);
	$text =  htmlentities( $text, ENT_QUOTES );
return $text;
}

function make_key_name2( $pool_id ) {
	global $dbconn;
	global $UP_config;

	$query = "SELECT * from pools WHERE pool_id = " .$pool_id ;

	$result = pg_query( $dbconn, $query );
	$arr    = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	return  ( $UP_config['ssh_keydir']. '/'. $arr['pool_id'].'@'. $arr['remote_user'] . '@' . $arr['remote_host'] . '.id_rsa' );


}


function make_key_name( $app_id ) {
	global $dbconn;
	global $UP_config;

	$query   = "SELECT pool_id FROM app_list WHERE app_id = " . $app_id;
	$result  = pg_query( $dbconn, $query );
	$arr     = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	$pool_id = $arr['pool_id'];

	$query = "SELECT * from pools WHERE pool_id = " .$pool_id ;

	$result = pg_query( $dbconn, $query );
	$arr    = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	return  ( $UP_config['ssh_keydir']. '/'. $arr['pool_id'].'@'. $arr['remote_user'] . '@' . $arr['remote_host'] . '.id_rsa' );


}

function make_staging_directory( $dbconn, $app_id, $user_id, $job_id  ){ 
	global $UP_config;
	global $_SESSION;

	$UP_ssh		= $UP_config['ssh'];
	$UP_ssh_key = $UP_config['ssh_key'];
	$UP_make_staging_directory = $UP_config['make_staging_directory'];


	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );

	$key = make_key_name( $app_id );

	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key; #$app_user . '@'.$app_host. '.id_rsa';
	$cmd = $cmd . ' '. $app_user .'@' . $app_host;
	$cmd = $cmd . ' '. $app_path.$UP_make_staging_directory . " ". $job_id . " '".   $_SESSION['gecos']. "'";

#echo $cmd;

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
		echo "Unable to start job";
		die_now();
	}
}


function get_job_status( $dbconn, $staging_dir, $job_id, $user_id ) {
	$query  = " SELECT status, substatus, app_id FROM job_list WHERE job_id = " . sanify( $job_id );
	$result = pg_query( $dbconn, $query );
	$arr    = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	$app_id = $arr[ 'app_id' ];

#echo "STATUS:: [".$arr['status']."]";

	if( $arr['status'] == 'C' ) {
		$ret = array ( 0=> $arr['status'], 1=> $arr['substatus'] );
		return $ret;
	}
	else {
		// fetch updated state
		global $UP_config;
		$UP_ssh			= $UP_config['ssh'];
		$UP_scp			= $UP_config['scp'];
		$UP_ssh_key			= $UP_config['ssh_key'];
		$UP_ssh_keydir			= $UP_config['ssh_keydir'];
		$UP_job_status			= $UP_config['job_status'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );

	$key = make_key_name( $app_id );

#	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $UP_ssh_keydir.'/'. $app_user . '@'. $app_host. '.id_rsa' . ' '. $app_user.'@'.$app_host;
	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key . ' '. $app_user.'@'.$app_host;
	$cmd = $cmd . ' ' . $app_path.$UP_job_status . ' '. $staging_dir;

	$output = array ();
	
#print $cmd."</BR>";	
	exec( $cmd, $output );

#print "TEST\n";
#print $cmd."\n";
#print "<!--";
#print_r ($output);
#print "-->\n";

	$status = substr( sanify( $output[0] ), 0, 2 );
	$substatus = substr( sanify( $output[1] ), 0 , 2);

	$query = "UPDATE job_list SET status= '".$status."' , substatus='". $substatus ."' WHERE job_id = " . $job_id;

#echo $query."</BR>";

	pg_query( $dbconn, $query );

	$wt=-1;
	foreach( $output as $t ) {
		if ( strstr($t, "Walltime" ) ) {
			$ar = explode( ":", $t );
			if( $ar[1]>0 ) {
				$wt=$ar[1];
			}		
		}
	}

				$query = "UPDATE job_list SET wall_time = $wt WHERE job_id = ". $job_id;
		    $result =  pg_query( $dbconn, $query );

	return $output;
	}
	
}

function copy_metadata_to_staging_directory( $dbconn, $app_id, $staging_dir, $filename, $contents ) {
	global $UP_config;

	$UP_ssh			= $UP_config['ssh'];
	$UP_scp			= $UP_config['scp'];
	$UP_ssh_key			= $UP_config['ssh_key'];
	$UP_ssh_keydir			= $UP_config['ssh_keydir'];
	$UP_make_staging_directory			= $UP_config['make_staging_directory'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );

	$key = make_key_name( $app_id );

#	$cmd = $UP_scp . ' -v  '. $UP_ssh_key . ' '. $UP_ssh_keydir.'/'. $app_user . '@'.$app_host.'.id_rsa';

	$tempfile = tempnam( "/tmp" , "uportal" );
	$handle = fopen( $tempfile, "w" );
	fwrite( $handle, $contents );

	$cmd = $UP_scp . ' -v  '. $UP_ssh_key . ' '. $key;
	$cmd = $cmd . ' ' . $tempfile . ' '. $app_user .'@' . $app_host . ':'.$staging_dir. '/' . $filename;

	$output = array ();

	fclose( $handle );

	exec( $cmd, $output );
	
	
	unlink( $tempfile );
}
  
function copy_file_to_staging_directory( $dbconn, $app_id, $staging_dir, $file, $input_index , $user_id ) {
	global $UP_config;

	$UP_ssh			= $UP_config['ssh'];
	$UP_scp			= $UP_config['scp'];
	$UP_ssh_key			= $UP_config['ssh_key'];
	$UP_ssh_keydir			= $UP_config['ssh_keydir'];
	$UP_make_staging_directory			= $UP_config['make_staging_directory'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );

	$key = make_key_name( $app_id );

#	$cmd = $UP_scp . ' -v  '. $UP_ssh_key . ' '. $UP_ssh_keydir.'/'. $app_user . '@'.$app_host.'.id_rsa';
	$cmd = $UP_scp . ' -v  '. $UP_ssh_key . ' '. $key;
	$cmd = $cmd . ' ' . $file . ' '. $app_user .'@' . $app_host . ':'.$staging_dir. '/input-' . $input_index;

	$output = array ();

#echo "</br>";
#echo $cmd;
#echo "</br>";


	exec( $cmd, $output );
#print_r($output );
	
	
}

function get_app_script( $dbconn, $app_id ) {
	$query = 'SELECT app_script FROM app_list WHERE app_id = ' . $app_id;
	$result =  pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( $arr != NULL ) {
		return $arr['app_script'];
	}
	else { return NULL; }
}


function get_app_user( $dbconn, $app_id ) {
	$query   = "SELECT pool_id FROM app_list WHERE app_id = ". $app_id;
	$result  =  pg_query( $dbconn, $query );
	$arr     = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	$pool_id = $arr['pool_id'];

	$query   = 'SELECT remote_host, remote_user FROM pools WHERE pool_id = ' . $pool_id;
	$result  =  pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( $arr != NULL ) {
		return $arr['remote_user'];
	}
	else { return NULL; }
}

function get_app_path( $dbconn, $app_id ) {

	$query   = "SELECT pool_id FROM app_list WHERE app_id = ". $app_id;
	$result  =  pg_query( $dbconn, $query );
	$arr     = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	$pool_id = $arr['pool_id'];

	$query = 'SELECT script_path FROM pools WHERE pool_id = ' . $pool_id;
	$result =  pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( $arr != NULL ) {
		return $arr['script_path'].'/';
	}
	else { return NULL; }

}

function get_app_host( $dbconn, $app_id ) {

	$query   = "SELECT pool_id FROM app_list WHERE app_id = ". $app_id;
	$result  =  pg_query( $dbconn, $query );
	$arr     = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	$pool_id = $arr['pool_id'];

	$query = 'SELECT remote_host, remote_user FROM pools WHERE pool_id = ' . $pool_id;
	$result =  pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( $arr != NULL ) {
		return $arr['remote_host'];
	}
	else { return NULL; }
}


function get_app_dspace_collection( $app_id ) {
	$app_id = sanify( $app_id );

	$query = "SELECT dspace_collection FROM app_list WHERE app_id  = '". $app_id . "'";
	$result = pg_query( $query );
	$arr =pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( isset( $arr['dspace_collection'] )) { return $arr['dspace_collection']; } else { return NULL; }
}

function get_app_name( $app_id ) {
	$app_id = sanify( $app_id );

	$query = "SELECT app_name FROM app_list WHERE app_id  = '". $app_id . "'";
	$result = pg_query( $query );
	$arr =pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( isset( $arr['app_name'] )) { return $arr['app_name']; } else { return NULL; }
}

function get_app_id_for_job( $job_id ) {
	
	$job_id = sanify($job_id);

	$query = "SELECT app_id FROM job_list WHERE job_id  = '". $job_id . "'";
	$result = pg_query( $query );
	$arr =pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( isset( $arr['app_id'] )) { return $arr['app_id']; } else { return NULL; }
}


function get_app_id( $app_name ) {
	
	$app_name = sanify($app_name);

	$query = "SELECT app_id FROM app_list WHERE app_name  = '". $app_name . "'";
	$result = pg_query( $query );
	$arr =pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( isset( $arr['app_id'] )) { return $arr['app_id']; } else { return NULL; }
}

function validate_app_type( $uid, $app_id ) {
	$uid = sanify( $uid );
	$app_id = sanify( $app_id );
	$query = 'SELECT * FROM app_list a, acl b WHERE  a.app_id = '. $app_id.' AND (( a.app_id = b.app_id AND  userid = '.$uid.' ))';

	$result = pg_query( $query );
	$arr =pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( isset( $arr['app_id'] )) { return true; } else { return false;  }
}

function get_app_memory_requirement( $dbconn, $app_id ) {
	$query = "SELECT memory from app_list where app_id=".$app_id;
	$results = pg_query( $query );
	$arr = pg_fetch_array( $results,  NULL, PGSQL_ASSOC );
	return $arr['memory'];
}
function get_app_cpu_requirement( $dbconn, $app_id ) {
	$query = "SELECT ncpus from app_list where app_id=".$app_id;
	$results = pg_query( $query );
	$arr = pg_fetch_array( $results,  NULL, PGSQL_ASSOC );
	return $arr['ncpus'];
}

function get_app_walltime_requirement( $dbconn, $app_id ) {
	$query = "SELECT wall_time  from app_list where app_id=".$app_id;
	$results = pg_query( $query );
	$arr = pg_fetch_array( $results,  NULL, PGSQL_ASSOC );
	return $arr['wall_time'];
}

function get_app_input_file_count( $dbconn, $app_id ) {
	$query = "SELECT count(index) from app_input where app_id=".$app_id;
	$results = pg_query( $query );
	$arr = pg_fetch_array( $results,  NULL, PGSQL_ASSOC );
	return $arr['count'];
}

function get_app_output_file_count( $dbconn, $app_id ) {
	$query = "SELECT count(index) from app_output where app_id=".$app_id;
	$results = pg_query( $query );
	$arr = pg_fetch_array( $results,  NULL, PGSQL_ASSOC );
	return $arr['count'];
}

function app_input_file_description( $app_id ) {
	$query = "SELECT * FROM app_input WHERE app_id = ".sanify($app_id). " ORDER BY index ASC ";
	$results = pg_query( $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	$d=array();
	while( isset ($arr['index'] ) ) {
		$d[] =  array( 'description' => $arr['description'], 'index' => $arr['index'], 'required' => $arr['required'] );
		$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	}

	return $d;
}

function register_job_details( $dbconn, $app_id, $userid, $description, $staging_dir, $project_id, $jid ) {
#	$query = "SELECT nextval('job_seq_id' )";
#	$result = pg_query(	$dbconn, $query );
#	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

#	$jid = $arr['nextval'];

	# If no projects defined, this will be -1. Reset to null to prevent foreign key violation

	$project_id = (int) sanify( $project_id );
	if( $project_id	< 0 ) {
		$project_id= 'null';
	}


	$query = "INSERT INTO job_list ( job_id, app_id, userid,  description, staging_dir, is_published, project_id ) VALUES (" . $jid  . ", " . $app_id . ", " . $userid . ",  '" . sanify($description) . "' , '" . sanify( $staging_dir) . "', false, ". $project_id . "  )";


	pg_query( $query );

	return $jid;
}

function set_job_project( $jid, $projectid ) {
	global $dbconn;
	$query = "UPDATE job_list set project_id = " . sanify($projectid) . " WHERE job_id = " . sanify($jid);

	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
}

function check_project_owner( $uid, $jid ) {
	global $dbconn;

	$query =" SELECT count(*) FROM projects WHERE project_id=".sanify( $jid ). " AND userid = " . sanify($uid) ;
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );


	if( $arr['count'] > 0 ) { return true; } else { return false; }
}


function check_job_owner( $uid, $jid ) {
	global $dbconn;

	$query =" SELECT count(*) FROM job_list WHERE job_id=".sanify( $jid ). " AND userid = " . sanify($uid) ;
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );


	if( $arr['count'] > 0 ) { return true; } else { return false; }
}

function return_job_output_file( $jid, $idx , $user_id ) {

	global $dbconn;
	global $UP_config;
	$query ="SELECT * FROM job_list WHERE job_id = " . sanify( $jid );
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	$staging_dir = $arr['staging_dir' ];

	$app_id = $arr['app_id'];

	$query = "SELECT * FROM app_output WHERE app_id = ".$app_id." AND index = " . sanify( $idx);
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	if( !isset( $arr['app_id'] ) ) {
		echo "No such output file";
		die_now();
	}

	$UP_ssh		       = $UP_config['ssh'];
	$UP_ssh_key		    = $UP_config['ssh_key'];
	$UP_ssh_keydir		 = $UP_config['ssh_keydir'];
	$UP_make_staging_directory		= $UP_config['make_staging_directory'];
	$UP_make_input_tar = $UP_config['make_input_tar'];
	$UP_cat		       = $UP_config['cat'];
	$UP_filetest		       = $UP_config['filetest'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );

	$key = make_key_name( $app_id );

		$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key . ' '. $app_user.'@'.$app_host;
		$cmd = $cmd . ' ' . $app_path.$UP_filetest . ' '. $staging_dir.'/output-'. $idx;


		exec( $cmd, $output );
		if( $output[0] == "0" ) {
			# return a 403
			header("HTTP/1.0 404 Not Found");
			return;
		}

	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key . ' '. $app_user.'@'.$app_host;
	$cmd = $cmd . ' ' . $UP_cat . ' '. $staging_dir.'/output-'. $idx;



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


function remove_job_file( $dbconn, $jid, $file, $staging_file, $user_id ) {
	global $UP_config;

	$query ="SELECT staging_dir, app_id FROM job_list WHERE job_id = " . sanify( $jid );
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	$staging_dir = $arr['staging_dir' ];

	$app_id = $arr['app_id'];

	$UP_ssh			 = $UP_config['ssh'];
	$UP_ssh_key		 = $UP_config['ssh_key'];
	$UP_ssh_keydir	 = $UP_config['ssh_keydir'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );


	$key = make_key_name( $app_id );
	//copy the remote file $file to the staging_file

	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key . ' '. $app_user.'@'.$app_host;
	$cmd = $cmd . ' rm ' . $staging_dir.'/'.$file ;

	echo "<!-- $cmd -->\n\n";

	exec( $cmd, $output );

//	print $cmd;
//	print ("<BR><BR>");
//	print_r( $output );


}


function copy_job_file( $dbconn, $jid, $file, $staging_file, $user_id ) {
	global $UP_config;

	$query ="SELECT staging_dir, app_id FROM job_list WHERE job_id = " . sanify( $jid );
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	$staging_dir = $arr['staging_dir' ];

	$app_id = $arr['app_id'];

	$UP_scp			 = $UP_config['scp'];
	$UP_ssh_key		 = $UP_config['ssh_key'];
	$UP_ssh_keydir	 = $UP_config['ssh_keydir'];
	$UP_make_staging_directory	 = $UP_config['make_staging_directory'];
	$UP_make_input_tar = $UP_config['make_input_tar'];
	$UP_cat			 = $UP_config['cat'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );


	$key = make_key_name( $app_id );
	//copy the remote file $file to the staging_file

	$cmd = $UP_scp . ' '. $UP_ssh_key . ' '. $key . ' '. $app_user.'@'.$app_host;
	$cmd = $cmd . ':"' . $staging_dir.'/'.$file. '" "'.$staging_file.'"';


	exec( $cmd, $output );

//	print $cmd;
//	print ("<BR><BR>");
//	print_r( $output );


}

function return_job_input_file(  $jid, $idx , $user_id) {
	global $dbconn;
	global $UP_config;

	$query ="SELECT staging_dir, app_id FROM job_list WHERE job_id = " . sanify( $jid );
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	$staging_dir = $arr['staging_dir' ];

	$app_id = $arr['app_id'];

	$query = "SELECT * FROM input_file WHERE job_id = ".sanify($jid)." AND index = " . sanify( $idx);
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	if( !isset( $arr['job_id']) ) {
		return 0;
	}

	$UP_ssh        = $UP_config['ssh'];
	$UP_ssh_key    = $UP_config['ssh_key'];
	$UP_ssh_keydir = $UP_config['ssh_keydir'];
	$UP_make_staging_directory = $UP_config['make_staging_directory'];
	$UP_make_input_tar = $UP_config['make_input_tar'];
	$UP_cat        = $UP_config['cat'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );


	$key = make_key_name( $app_id );
	// call the tar script

	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key . ' '. $app_user.'@'.$app_host;
	$cmd = $cmd . ' ' . $UP_cat . ' '. $staging_dir.'/input-'. $idx;


	header('Content-Type: "'.$arr['mimetype'].'"');
	header('Content-disposition: attachment; filename="'.$arr['filename'].'"' );
	header('Pragma: public');
	header('Expires: 0');
	header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
	passthru( $cmd );


	exit;
}

function return_job_tar_file(  $jid , $user_id ) {
	global $dbconn;
	global $UP_config;

	$query ="SELECT staging_dir, app_id FROM job_list WHERE job_id = " . sanify( $jid );
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	$staging_dir = $arr['staging_dir' ];

	$app_id = $arr['app_id'];

	$UP_ssh        = $UP_config['ssh'];
	$UP_ssh_key    = $UP_config['ssh_key'];
	$UP_ssh_keydir = $UP_config['ssh_keydir'];
	$UP_make_staging_directory        = $UP_config['make_staging_directory'];
	$UP_make_input_tar        = $UP_config['make_input_tar'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );

	$key = make_key_name( $app_id );

	// call the tar script

	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key . ' '. $app_user.'@'.$app_host;
	$cmd = $cmd . ' ' . $app_path.$UP_make_input_tar . ' '. $staging_dir ;


	header("Content-Type: application/x-gzip");
	header("Content-disposition: attachment; filename=\"output_".$jid.".tar.gz\"");

	header('Pragma: public');
	header('Expires: 0');
	header("Cache-Control: must-revalidate, post-check=0, pre-check=0");




	passthru( $cmd );

	exit;
}

function record_input_type( $dbconn, $job_id, $name, $mime_type, $index ) {
	$query ="INSERT INTO input_file ( job_id, filename, mimetype, index ) VALUES (" . sanify($job_id) . ", '" . sanify($name) ."', '".$mime_type."', " . sanify( $index ) . ")";
	echo "</br>";
	//echo $query;
	$result = pg_query( $dbconn, $query );
}

function execute_job( $dbconn, $job_id, $app_id, $app, $staging_dir ) {
	global $UP_config;

	$job_id = sanify( $job_id );
	$app    = sanify( $app );
	$staging_dir    = sanify( $staging_dir );


	$input_file_count=1;
	$output_file_count=4;
	$memory_requirement=1000;	

	$UP_ssh        = $UP_config['ssh'];
	$UP_ssh_key    = $UP_config['ssh_key'];
	$UP_ssh_keydir = $UP_config['ssh_keydir'];
	$UP_delete_job = $UP_config['delete_job'];
	$UP_run_job    = $UP_config['run_job'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );

	$key = make_key_name( $app_id );

	$app_script = get_app_script( $dbconn, $app_id );

	$input_file_count  = get_app_input_file_count ( $dbconn, $app_id );
	$output_file_count = get_app_output_file_count( $dbconn, $app_id );
	$memory_requirement = get_app_memory_requirement( $dbconn, $app_id );
	$cpu_requirement    = get_app_cpu_requirement( $dbconn, $app_id );
	$walltime           = get_app_walltime_requirement( $dbconn, $app_id );
		
	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key;
	$cmd = $cmd . ' '. $app_user .'@' . $app_host;
	$cmd = $cmd . ' '. $app_path.$UP_run_job . ' '. $staging_dir . ' ' . $input_file_count . ' ' . $output_file_count . ' "' . $app_script .'" '. $memory_requirement . ' ' . $cpu_requirement  . ' ' . $walltime;

#echo $cmd;
print "<!-- $cmd -->";

	exec  ($cmd, $output );

print "<!--";
print_r( $output );
print "-->";

}


function delete_job( $job_id ) {
	global $dbconn;
	global $UP_config;

	$job_id =sanify( $job_id );

	$query = "SELECT count(*) FROM job_list WHERE job_id = ".$job_id ." AND deleted = false";
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	if( $arr['count']==0 ) { 
		return 0;
	}

	$query = "UPDATE job_list SET deleted= true WHERE job_id = " .$job_id . " AND deleted=false";
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	$query = "SELECT * FROM job_list WHERE job_id = " . sanify( $job_id );
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	$app_id = $arr['app_id'];
	$staging_dir = $arr['staging_dir'];



#	$batch_id = $arr['batch_id'];
#	$app_id = $arr[ 'app_id' ];
#	$staging_dir = $arr[ 'staging_dir' ];
#	
#	if(!isset($batch_id)) {
#		echo "Cannot find remote job id";
#		$batch_id = -1;
#		die_now();
#	}

	$UP_ssh			= $UP_config['ssh'];
	$UP_ssh_key		= $UP_config['ssh_key'];
	$UP_ssh_keydir = $UP_config['ssh_keydir'];
	$UP_delete_job	= $UP_config['delete_job'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );


	$key = make_key_name( $app_id );
	
	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key;
	$cmd = $cmd . ' '. $app_user .'@' . $app_host;
	$cmd = $cmd . ' '. $app_path.$UP_delete_job . ' '. $staging_dir;

//echo $cmd;
	exec  ($cmd, $output );


	return 1;
}

function is_published( $jid ) {
		$jid = sanify($jid );
			$query  = "SELECT is_published FROM job_list WHERE job_id = ".sanify($jid);
			$result = pg_query(  $query );
			$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );



			if( $arr['is_published'] == 't' ) { 
				return true; 
			} else { 
				return false; 
			}
}

function output_file_index_of_mimetype( $appid, $mime ) {
	$query="SELECT index FROM app_output WHERE mimetype='" . sanify( $mime ) . "' AND app_id = " . sanify( $appid );
	$result = pg_query(  $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	if( $arr == NULL ) {  return -1; }
	else { return $arr['index']; }	
}

function has_output_file_of_mimetype(  $appid, $mime ) {
	$query="SELECT count(*) FROM app_output WHERE mimetype='" . sanify( $mime ) . "' AND app_id = " . sanify( $appid );
	$result = pg_query(  $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );

	$cnt = $arr['count'];
	

	if( $cnt == 0 ) {  return false; }
	else { return true; }	
}

function is_publishable( $appid, $jid ) {

	# Check to see whether this app type has an output file of type 'application/x-mets-archive'
	$result = ( has_output_file_of_mimetype(  $appid, 'application/x-mets-archive' ) );
	return $result;
}




function	 get_job_description( $dbconn, $jid ) {
	$query = "SELECT description FROM job_list WHERE job_id = " . sanify( $jid);
	$results = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	return $arr['description'];
}


function	 get_job_input_filename( $dbconn, $jid, $index ) {
	$query = "SELECT filename FROM input_file WHERE job_id = " . sanify( $jid) . " AND index = " . sanify($index);
	$results = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	return $arr['filename'];
}


function pool_motd( $pool_id) {
	global $dbconn;
	global $UP_config;

	$pool_id = sanify( $pool_id );

	$query   = "SELECT * from pools WHERE pool_id = " . $pool_id;
	$results = pg_query( $dbconn, $query );
	$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	# call app_config on the remote host

	$key = make_key_name2( $pool_id );	

	$UP_ssh		= $UP_config['ssh'];
	$UP_ssh_key = $UP_config['ssh_key'];
	$UP_pool_status = "motd";

	$app_user =  $arr['remote_user'];
	$app_host =  $arr['remote_host'];
	$app_path =  $arr['script_path'] ."/";

	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key; #$app_user . '@'.$app_host. '.id_rsa';
	$cmd = $cmd . ' '. $app_user .'@' . $app_host;
	$cmd = $cmd . ' '. $app_path.$UP_pool_status;

	$output = array();

#	print "<p>$cmd";
	exec  ($cmd, $output );
	return implode( "</br>",$output);

}
function pool_status( $pool_id) {
	global $dbconn;
	global $UP_config;

	$pool_id = sanify( $pool_id );

	$query   = "SELECT * from pools WHERE pool_id = " . $pool_id;
	$results = pg_query( $dbconn, $query );
	$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	# call app_config on the remote host

	$key = make_key_name2( $pool_id );	

	$UP_ssh		= $UP_config['ssh'];
	$UP_ssh_key = $UP_config['ssh_key'];
	$UP_pool_status = $UP_config['pool_status'];

	$app_user =  $arr['remote_user'];
	$app_host =  $arr['remote_host'];
	$app_path =  $arr['script_path'] ."/";

	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key; #$app_user . '@'.$app_host. '.id_rsa';
	$cmd = $cmd . ' '. $app_user .'@' . $app_host;
	$cmd = $cmd . ' '. $app_path.$UP_pool_status;

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
	global $dbconn;
	$pool_id = sanify( $pool_id );
	$app_id  = sanify( $app_id  );

	$query = "SELECT * FROM app_list WHERE  pool_id = " . $pool_id . "AND app_id = ". $app_id;

	$results = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	return ( $arr!=NULL );
	
}

function has_access_to_pool( $pool_id, $user_id ) {
	global $dbconn;
	$pool_id = sanify( $pool_id );
	$user_id = sanify( $user_id );

	$query = "SELECT * FROM acl WHERE  user_id = " . $user_id . "AND pool_id = ". $pool_id;

	$results = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	return ( $arr!=NULL );
	
}

function get_available_pools( $user_id ) {
	global $dbconn;
	$query = "SELECT * FROM acl a JOIN pools b ON a.pool_id = b.pool_id WHERE  a.user_id = " . $user_id;

	$results = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	$idx  = array();
	$desc = array();

	while( $arr!=NULL ) {
		$idx[] = $arr['pool_id'];
		$desc[]= $arr['description'];
		$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	}
	
	return array( 'index' => $idx , 'description' => $desc );
}

function get_pools( $user ) {
	global $dbconn;
	$user_id = sanify( get_uid( $user ) );
	$query = "SELECT * FROM pools WHERE pool_id in ( SELECT pool_id from acl WHERE user_id = " . $user_id . ")";

	$results = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	
	$plist = array();
#print $query;
	while( $arr!=NULL ) {
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
		$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	}

	return $plist;
}

function delete_pool( $user, $pool_id ) {
	global $dbconn;
	$user_id = get_uid( $user );
	$query = "DELETE FROM pools WHERE pool_id = ". $pool_id ." AND owner = " . $user_id;
	$results = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

}

function owns_pool( $username, $pool_id ) {
	global $dbconn;

	$username = sanify( $username );
	$pool_id  = (int) sanify( $pool_id );

	$user_id = get_uid( $username );

	$query = "SELECT owner FROM pools WHERE pool_id = " . $pool_id . " AND owner = " . $user_id;

	$results = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	if( $arr!=NULL ) { return true; }
	else { return false; }

}

function get_key( $pool_id ) {
	$key = make_key_name2( $pool_id );

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
	global $dbconn;
	global $UP_config;

	$uid = get_uid( $user );
	# first get pool id
	$query = "SELECT nextval('pool_id_seq')";
	$results = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	$pool_id = $arr['nextval'];

	$user_id = get_uid( $user );
	$remote_user =sanify( $remote_user );
	$remote_host =sanify( $remote_host );
	$script_path =sanify( $script_path );
	$description =sanify( $description );

	#Special untainting
	
	$remote_host = preg_replace( "/[^\w\.]/", "", $remote_host );
	$remote_user = preg_replace( "/[^\w\.]/", "", $remote_user );
	$script_path = preg_replace( "/[^\w\.\/\-\_]/", "", $script_path );


	$query = "INSERT INTO pools ( pool_id, owner, remote_user, remote_host, script_path, description ) VALUES ( " . $pool_id. ", ". $user_id . ",'" . $remote_user ."','".$remote_host ."','" . $script_path ."', '". $description ."')";

	$results = pg_query( $dbconn, $query );

	# Add to ACL
	$query  = "INSERT INTO acl (pool_id, user_id ) VALUES ( ". $pool_id . "," . $user_id .")";
	$results = pg_query( $dbconn, $query );

	$key = make_key_name2( $pool_id );

	# Generate a new ssh key

	$sshkeygen = $UP_config['ssh-keygen'];
	$cmd = $sshkeygen . " -t rsa -f " . $key;
	$arr = exec( $cmd );


}

function get_pool_applications( $pool_id ) {
	global $dbconn;

	$query = "SELECT * from app_list WHERE current= true and pool_id = " . $pool_id;
	$results = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	$list = array();

	while ( $arr != NULL ) {
		$app_id = $arr['app_id'];

		# Input 
		$query2   = "SELECT * FROM app_input WHERE app_id = ". $app_id . " ORDER BY index ASC";
		$results2 = pg_query( $dbconn, $query2 );
		$arr2     = pg_fetch_array( $results2, NULL, PGSQL_ASSOC );

		$inp=array();
		while ( $arr2 != NULL ) {
			$inp[] = array( 'description' => $arr2['description'], 'mimetype' => $arr2['mimetype'] );
			$arr2 = pg_fetch_array( $results2, NULL, PGSQL_ASSOC );
		}


		#Output 
		$query2   = "SELECT * FROM app_output WHERE app_id = ". $app_id . " ORDER BY index ASC";
		$results2 = pg_query( $dbconn, $query2 );
		$arr2     = pg_fetch_array( $results2, NULL, PGSQL_ASSOC );


		$out=array();
		while ( $arr2 != NULL ) {
			$out[] = array( 'description' => $arr2['description'], 'mimetype' => $arr2['mimetype'], 'filename' => $arr2['filename'] );

			$arr2 = pg_fetch_array( $results2, NULL, PGSQL_ASSOC );
		}

		$a = array ( 'ncpus'=>$arr['ncpus'], 'memory'=>$arr['memory'], 'name' => $arr['app_name'], 'script' => $arr['app_script'], 'input' => $inp, 'output' => $out , 'walltime' => $arr['wall_time'] );


		$list[] = $a;
		$arr = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	}	

	return $list;
}

function refresh_pool_applications( $pool_id ) {
	global $dbconn;
	global $UP_config;

	$query   = "SELECT * from pools WHERE pool_id = " . $pool_id;
	$results = pg_query( $dbconn, $query );
	$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	# call app_config on the remote host

	$key = make_key_name2( $pool_id );	

	$UP_ssh		= $UP_config['ssh'];
	$UP_ssh_key = $UP_config['ssh_key'];
	$UP_app_config = $UP_config['app_config'];

	$app_user =  $arr['remote_user'];
	$app_host =  $arr['remote_host'];
	$app_path =  $arr['script_path'] ."/";

	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key; #$app_user . '@'.$app_host. '.id_rsa';
	$cmd = $cmd . ' '. $app_user .'@' . $app_host;
	$cmd = $cmd . ' '. $app_path.$UP_app_config;

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
	global $dbconn;

	$query = "UPDATE app_list SET current = false WHERE pool_id = ". $pool_id;
	$results = pg_query( $dbconn, $query );


	foreach( $apps as $app ) {

		$q1 = "SELECT nextval('app_seq_id')";
		$results = pg_query( $dbconn, $q1 );
		$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
		$app_id = $arr['nextval'];

		
		$query = "INSERT INTO app_list (app_id, app_name, app_script, pool_id, ncpus, memory, wall_time, current ) VALUES ( ". $app_id .", '" . $app['description'] . "' , '" . $app['script']. "' , " . $pool_id . ", " . $app['ncpus']. ", " . $app['memory'] .  ", " . $app['walltime'].", true );";

		$results = pg_query( $dbconn, $query );

		$inputs = $app['inputs'];

		foreach( $inputs as $input ) {
			$query = "INSERT INTO app_input ( app_id, index, description, mimetype, required ) VALUES ( " . $app_id .", ". $input['index'] . ", '" . $input['description'] ."', '". $input['mimetype']."', ". $input['required'] .")";

			$results = pg_query( $dbconn, $query );

		}

		$outputs = $app['outputs'];

		foreach( $outputs as $output ) {
			$query = "INSERT INTO app_output ( app_id, index, description, mimetype, filename ) VALUES ( " . $app_id .", ". $output['index'] . ", '" . $output['description'] ."', '". $output['mimetype']."', '" . $output['filename']."')";

			$results = pg_query( $dbconn, $query );
		}
	} 
	
}

function pool_status_xml_parse( $arr ) {
	$dom = domxml_open_mem( $arr );

	if( !$dom ) {
		return 'No XML found';
	}


	$apps = $dom->get_elements_by_tagname('host');

	$error = NULL;
	
	$statusx=array();

	foreach( $apps as $app ) {
		$hostname = sanify( $app->get_attribute('hostname')  );
		$memory   = sanify( $app->get_attribute('memory')  );
		$ncpus    = sanify( $app->get_attribute('ncpus')  );
		$status   = sanify( $app->get_attribute('status')  );

		$statusx[] = array ( 'hostname' => $hostname, 'memory' => $memory, 'ncpus' => $ncpus, 'status' => $status );
	}

	return $statusx;
}

function pool_xml_parse( $arr ) {

	$dom = domxml_open_mem( $arr );

	if( !$dom ) {
		return 'No XML found';
	}

	$applist= array();

	$apps = $dom->get_elements_by_tagname('application');

	$error = NULL;

	foreach( $apps as $app ) {

		$script = sanify( $app->get_attribute('script')  );
		$app_description = sanify( $app->get_attribute('description')  );
		$ncpus= (int) sanify( $app->get_attribute('ncpus') );
		$memory=(int) sanify( $app->get_attribute('memory'));
		$walltime =(int) sanify( $app->get_attribute('walltime'));



		if( empty( $script   )  ) { $error = "[$app_description]: No script"; }
		if( empty( $app_description   )  ) { $error = "[$app_description]: No description"; }
		if( empty( $ncpus  ) || $ncpus<1  ) { $error = "[$app_description]: No NCPUS";  }
		if( empty( $memory ) || $memory<0 ) { $error = "[$app_description]: No memory"; }
		if( empty( $walltime ) || $walltime<0 ) { $error = "[$app_description]: No walltime"; }



		$inp = array();

		$inputs = $app->get_elements_by_tagname( 'input' );

		foreach( $inputs as $input ) {
			$mimetype   = sanify( $input->get_attribute( 'mimetype' ) );
			$description= sanify( $input->get_attribute( 'description' ) );
			$index      = sanify( $input->get_attribute( 'index' ) );
			$required   = sanify( $input->get_attribute( 'required' ) );

			if( empty ( $required ) ) { $required = "yes"; }

			if( $required=="yes" ) { $required='true'; }
			else { $required='false'; }

			if( empty ( $mimetype    ) ) { $error = "No mimetype (input)"   ; }
			if( empty ( $description ) ) { $error = "No description (input)"; }
			if( $index!=0 && !is_numeric( $index ) ) { $error = "No index (input) $description"; }

			$inp[] = array ( 'mimetype'=>$mimetype, 'description'=>$description, 'index'=>$index, 'required'=>$required );
		}

		$oup = array();

		$outputs = $app->get_elements_by_tagname( 'output' );

		foreach( $outputs as $output ) {
			$mimetype    = sanify( $output->get_attribute( 'mimetype' ) );
			$description = sanify( $output->get_attribute( 'description' ) );
			$index       = sanify( $output->get_attribute( 'index' ) );
			$filename    = sanify( $output->get_attribute( 'filename' ) );

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
	global $dbconn;
	$query = "SELECT app_script FROM app_list WHERE app_id = ".sanify($app_id );

	$results = pg_query( $dbconn, $query );
	$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	return $arr['app_script'];
	
}

function manage_job_invocation( $uid, $app_id, $description, $project_id ) {
	global $_FILES;
	global $dbconn;


	
   #// have the input files been specified? //
   #if( isset ( $_FILES['input-0'] )  ) {


			$app = get_script_for_app_id( $app_id );
   //    echo "we'll make this now!</br>";
			$job_id = get_next_job_id();


         // copy the input files over to the staging directory
         $i=0;

         $input_file_count = get_app_input_file_count( $dbconn,  $app_id );


			// check to see whether all mandatory input files have been specified
			for( $i=0; $i< $input_file_count; $i++ ) {
				if( is_required_input_file( $app_id, $i ) ) {
					if( !strcmp( $_FILES['input-'.$i]['tmp_name'], '' ) || $_FILES['input-'.$i]['error']!=0 ) {
						return "Required input file not specified";
					}
				}
			} 			

         $description = $_POST['description'];
			$description = sanify( $description );


         $staging_dir = make_staging_directory( $dbconn, $app_id, $uid, $job_id );

			copy_metadata_to_staging_directory( $dbconn, $app_id, $staging_dir, "project",  get_project_name( $uid, $project_id ) );
			copy_metadata_to_staging_directory( $dbconn, $app_id, $staging_dir, "description", $description );

			$foaf = get_foaf( $uid );
			if( strlen(trim($foaf)) > 0 ) {
				copy_metadata_to_staging_directory( $dbconn, $app_id, $staging_dir, "foaf", $foaf );
			}
			$orcid = get_orcid( $uid );
			if( strlen(trim($orcid)) > 0 ) {
				copy_metadata_to_staging_directory( $dbconn, $app_id, $staging_dir, "orcid", $orcid );
			}


         #for( $i=0; $i < $input_file_count; $i++ ) {
#
         #   if( !strcmp( $_FILES['input-'.$i]['tmp_name'], '' ) || $_FILES['input-'.$i]['error']!=0 ) {
         #      echo "Input file " . ($i+1) . " Not specified";
         #      die_now();
         #   }
         #}

         register_job_details( $dbconn, $app_id, $uid, $description, $staging_dir, $project_id, $job_id );

         for( $i=0; $i < $input_file_count; $i++ ) {
				if( isset ($_FILES['input-'.$i ]) ) {
	            $tmp_file  =  $_FILES['input-' . $i ][ 'tmp_name' ];
   	         $mime_type =  $_FILES['input-'.$i ][ 'type' ];
      	      $name      =  $_FILES['input-'.$i ][ 'name' ];
         	   $error     = $_FILES['input-'.$i ][ 'error' ];
            	copy_file_to_staging_directory( $dbconn, $app_id, $staging_dir, $tmp_file, $i, $uid );
	            record_input_type( $dbconn, $job_id, $name, $mime_type, $i );
				}
         }

         execute_job( $dbconn, $job_id, $app_id, $app, $staging_dir );

			return "";

   #}
   #else {
	#	return 0;
   #}

}

function get_pool_acl( $pool_id ) {
	global $dbconn;
	$pool_id = (int) sanify( $pool_id );

	$query = "SELECT a.pool_id, b.username FROM acl  a full join users b on a.user_id=b.userid where a.pool_id=" . $pool_id . " or a.pool_id is NULL ORDER BY b.username ASC";

	$results = pg_query( $dbconn, $query );
	$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	$res= array();
	while( $arr!=NULL ) {
		$is_mem = false;
		if ( $arr['pool_id'] == $pool_id ) { $is_mem=true; }
		$res[] = array( 'uname' => $arr['username'] , 'member' => $is_mem );

		$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	}
print_r( $res );
	return $res;
}


function check_figshare( $str ) {
	$matches = null;
	$str= sanify( $str );
	preg_match( "/[^a-zA-Z1234567890_-]/", $str, $matches, PREG_OFFSET_CAPTURE );
	if ( count($matches) === 0 ) { return $str; }
	else { return ""; }
}

function save_profile( $uid, $foafurl, $embargo, 
 $pub_dspace, $pub_chempound, $pub_figshare
 ) {
	global $dbconn;

	$foafurl = sanify( $foafurl );
	$embargo = sanify( $embargo );
//	$figsharekey = check_figshare( $figsharekey );
//	$figsharesecret = check_figshare( $figsharesecret );
	if( $embargo < 0 )  {$embargo=0;}
	if( $embargo > 999 ) { $embargo=999; }

	$embargo = (int) $embargo ;


		
//	$query = "DELETE FROM profile WHERE userid = " . $uid ;
//	pg_query( $dbconn, $query );
#	$query = "INSERT INTO profile (userid, foafurl, embargo, figsharekey, figsharesecret ) VALUES ( ".$uid. ", '" . $foafurl . "', ".$embargo. ", '". $figsharekey ."', '".$figsharesecret."' )";
#	pg_query( $dbconn, $query );

//print "<P>$figsharekey";
//print "<P>$figsharesecret";
//      pg_prepare( $dbconn, "updprof", "INSERT INTO profile ( userid, foafurl, embargo,   pub_dspace, pub_chempound, pub_figshare) VALUES ( $1, $2, $3, $4, $5, $6 )" ); 
	$q = "INSERT INTO profile (userid ) VALUES ( ".$uid.")";
	pg_query( $dbconn, $q );
	pg_prepare( $dbconn, "updprof","UPDATE profile SET foafurl=$2, embargo=$3, pub_dspace=$4, pub_chempound=$5, pub_figshare=$6 WHERE userid=$1" );
        $results = pg_execute( $dbconn, "updprof", array( 
		$uid, $foafurl, $embargo,
		$pub_dspace,
		$pub_chempound,
		$pub_figshare
	) );



}

function add_profile( $username ) {
	global $dbconn;
	$query = "INSERT INTO profile (userid, pub_dspace ) VALUES (" . get_uid( $username ) .", true )";

	   $results = pg_query( $dbconn, $query );


}

function get_profile( $username ) {
	global $dbconn;
	$query = "SELECT * FROM users WHERE userid = " . get_uid( $username );
   $results = pg_query( $dbconn, $query );
   $arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	$orcid = $arr["orcid"];	

	$query = "SELECT * FROM profile WHERE userid = " . get_uid( $username );

   $results = pg_query( $dbconn, $query );
   $arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	if(  $arr === NULL ) {
#print "<-- Adding profile for user $username --> ";
		add_profile( $username );
   		$results = pg_query( $dbconn, $query );
		$arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );
	}

	$res = array();
	$res['name'] =  $_SESSION['gecos'];
	$res['foaf'] =  $arr['foafurl'];
	$res['orcid'] =  $orcid;	
	$res['embargo'] =  $arr['embargo'];
//	$res['figsharekey'] =  $arr['figsharekey'];
//	$res['figsharesecret'] =  $arr['figsharesecret'];
	$res['pub_dspace']    = $arr['pub_dspace']    === "t" ? "checked" : "";
	$res['pub_chempound'] = $arr['pub_chempound'] === "t" ? "checked" : "";
	$res['pub_figshare']  = $arr['pub_figshare']  === "t" ? "checked" : "";
	return $res;
}


function preview_file( $jid, $idx, $user_id ) {
	global $dbconn;
	/* first, we get the mime type of the file */
	$app_id = get_app_id_for_job($jid);
	$query = "SELECT mimetype FROM app_output WHERE app_id = " . $app_id . "AND index = ".sanify($idx);

  $results = pg_query( $dbconn, $query );
  $arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

echo sanify($arr['mimetype']);

	/* get the preview template associated with this mime type */
	$query = "SELECT template,linebreak FROM preview_templates WHERE mimetype = '" . sanify($arr['mimetype'])."'";	
  $results = pg_query( $dbconn, $query );
  $arr     = pg_fetch_array( $results, NULL, PGSQL_ASSOC );

	$template = $arr['template'];
	$linebreak = $arr['linebreak' ];

	if ($template==NULL ) { $template="default"; $linebreak='<BR>'; }

	/* next, we transfer the contents of the file */

	$output = return_job_output_file_str( $app_id, $jid, $idx, $user_id );

#echo "$template";
#echo "<!--";
#echo implode( $linebreak,$output);
#echo "-->";

#	return array( $template, implode("<br>", $output) );	
	return array( $template, implode( $linebreak, $output) );	
}

function return_job_output_file_str( $app_id, $jid, $idx , $user_id ) {

	global $dbconn;
	global $UP_config;

	$query ="SELECT * FROM job_list WHERE job_id = " . sanify( $jid );
	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );


	$staging_dir = $arr['staging_dir' ];

	$UP_ssh		       = $UP_config['ssh'];
	$UP_ssh_key		    = $UP_config['ssh_key'];
	$UP_ssh_keydir		 = $UP_config['ssh_keydir'];
	$UP_make_staging_directory		= $UP_config['make_staging_directory'];
	$UP_make_input_tar = $UP_config['make_input_tar'];
	$UP_cat		       = $UP_config['cat'];
	$UP_filetest		       = $UP_config['filetest'];

	$app_user = get_app_user( $dbconn, $app_id );
	$app_host = get_app_host( $dbconn, $app_id );
	$app_path = get_app_path( $dbconn, $app_id );

	$key = make_key_name( $app_id );

	$cmd = $UP_ssh . ' '. $UP_ssh_key . ' '. $key . ' '. $app_user.'@'.$app_host;
	$cmd = $cmd . ' ' . $UP_cat . ' '. $staging_dir.'/output-'. $idx;

	$foo= array();
	exec( $cmd,  $foo );
	return $foo;	
}

function embargo( $username ) {
	global $dbconn;
	global $UP_config;
	$username =sanify( $username );
	$uid =  get_uid( $username );
	
	$query= "SELECT * FROM profile WHERE userid = ".$uid;

	$result = pg_query( $dbconn, $query );
	$arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	
	$period = $arr[ 'embargo' ];
	if( $period <= 0 ) {
		echo "NO EMBARGO PERIOD SET";
		return;
	}

	$query = 
	"select app_id, job_id from job_list where  userid=".$uid." and is_published=false and deleted=false and  tried_to_publish=false and submit_time < (CURRENT_TIMESTAMP- interval '".$period." days')";
	echo "<BR>".$query;

	$result = pg_query( $dbconn, $query );
	$arr     = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
echo "<BR> ".$arr;
	while( $arr!=NULL ) {
	echo "<BR>Would publish app id ".$arr['app_id'] . " ". $arr['job_id']. " " . $uid."\n";
		publish( $arr['app_id'], $arr['job_id'], $uid );

		$arr     = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
	}

	return;
}

function get_motd( $user ) {
#	print_r( get_pools( $user ));
	$motd="";
	$pools = get_pools( $user );
	$i=0;
	foreach( $pools as $p ) { 
#		print("<p> $i");
		$motd = $motd . pool_motd( $p['pool_id'] );
		$i++;
	}
	return $motd;
	
}

?>
