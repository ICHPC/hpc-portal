<?

# Set variables that go into _REQUEST
ini_set('variables_order', 'EGPS'); # exclude ccokies
# Show all errors
ini_set('error_reporting', E_ALL);
# Display all errors in browsers
ini_set('display_errors', 1);
# Send errors to web error log
ini_set('log_errors', 1);
# Send errors to a file
ini_set('error_log', '/opt/portal-devel/error.log');

$UP_config['ldap_host'] = 'unixldap.cc.ic.ac.uk';
$UP_config['ldap_port'] = NULL;
$UP_config['ldap_basedn'] = array('ou=chemistry,dc=ic,dc=ac,dc=uk', 'ou=hpc,dc=ic,dc=ac,dc=uk' );

$UP_config['file_location'] = '/opt/portal-devel/deploy/uportal/uportal-config/password.txt';
$UP_config['auth_methods'] = 'file';
$UP_config['auth_type'] = 'strict';

$UP_config['conn_string'] =  "host=127.0.0.1 port=5432 dbname=devel user=devel password=devel";
$UP_config['db_dbname'] =  'devel';
$UP_config['db_port'] =  '5432';
$UP_config['db_host'] =  '127.0.0.1';
$UP_config['db_user'] = 'devel';
$UP_config['db_password'] = 'devel';


$UP_config['ssh']        =  "/usr/bin/ssh -oConnectTimeout=10 ";
$UP_config['ssh_key']    = "-i ";
$UP_config['scp']        = "/usr/bin/scp ";

$UP_config['ssh_keydir'] = '/opt/portal-devel/deploy/keys/';

$UP_config['ssh-keygen'] = "/usr/bin/ssh-keygen";


$UP_config['cat']           = "/bin/cat";

#$UP_config['remote_uportal_dir'] ='bin/';

$UP_config['publish-figshare' ] = 'publish_figshare';
$UP_config['publish-figshare-local' ] = '/opt/portal-devel/deploy/bin/publish_figshare';
$UP_config['make_staging_directory'] = 'make_staging_directory';
$UP_config['make_input_tar'] = 'make_tar';
$UP_config['delete_job']    =  'delete_job';
$UP_config['run_job']       =  'run_app';
$UP_config['job_status']    =  'job_state';
$UP_config['filetest']      =  'filetest';
$UP_config['app_config']    =  'app_configuration';
$UP_config['pool_status']   =  'pool_status';

$UP_config['dspace_url']     = "http://scanweb.cc.ic.ac.uk:8080/dspace/handle/";
$UP_config['dspace_importer']= "/var/www/uportal/uportal-config/bin/lniimport";
$UP_config['chempound_importer']= "/var/www/uportal/uportal-config/bin/qc";

#$UP_config['condor_status'] = "/var/condor/condor_status";

$UP_config['admin_email'] = 'adminname@example.com';
$UP_config['admin_name'] = 'Admin Name';
$UP_config['admin_subject'] = 'Portal Mail';

$UP_CONFIG['protocol'] = 'https';

?>
