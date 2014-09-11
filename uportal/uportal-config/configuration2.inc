<?

### PHP options

###############################################################################
### User configurable options follow
###############################################################################

###############################################################################
### Errors.  Note that if scripts fail to parse no errors
###   will show unless appropriate options are set in the system-wide
###   php.ini file.

###############################################################################
### Error reporting.  Uncomment one of these
## Show no errors.  No information leaks to outside world if an error occurs.
# ini_set('error_reporting', 0);
## Show all errors.  Use for debugging only.
ini_set('error_reporting', E_ALL);

###############################################################################
### Where errors go.  Uncomment suitably.
## Display all errors in browsers
ini_set('display_errors', 1);
## Send errors to web error log
ini_set('log_errors', 1);
## Send errors to a file
ini_set('error_log', '/opt/portal-devel/error.log');

###############################################################################
### Local admin options
$UP_config['admin_email'] = 'adminname@example.com';
$UP_config['admin_name'] = 'Admin Name';
$UP_config['admin_subject'] = 'Portal Mail';

###############################################################################
### Authentication of users.
## LDAP host.  If using LDAP, where requests are sent, and port number.
$UP_config['ldap_host'] = 'unixldap.cc.ic.ac.uk';
$UP_config['ldap_port'] = NULL;
## LDAP basedns.  As many DNs as you wish to check for the user in.
$UP_config['ldap_basedn'] = array('ou=chemistry,dc=ic,dc=ac,dc=uk', 'ou=hpc,dc=ic,dc=ac,dc=uk' );

## File authentication.  Location of file.
$UP_config['file_location'] = '/opt/portal-devel/deploy/uportal/uportal-config/password.txt';

## Which authentication methods to use.  Can use: ldap and/or file.
$UP_config['auth_methods'] = 'file';
## Auth type: strict or permissive
#  strict means all methods in 'auth_methods' must succeed.
#  permissive means at least one method must succeed.
$UP_config['auth_type'] = 'strict';

###############################################################################
### Database
## Connection parameters for the Postgresql database
#  From the webserver you should be able to do:
#  psql -h db_host -p db_port -U db_user -W db_dbname
#  entering db_password when requested.
$UP_config['db_host'] =  '127.0.0.1';
$UP_config['db_password'] = 'devel';
$UP_config['db_dbname'] =  'devel';
$UP_config['db_port'] =  '5432';
$UP_config['db_user'] = 'devel';

###############################################################################
### SSH commands
##  For the webserver to connect to the run hosts.
##  These are the paths to the commands on the webserver
$UP_config['ssh']        =  "/usr/bin/ssh -oConnectTimeout=10 ";
$UP_config['scp']        = "/usr/bin/scp";
$UP_config['ssh-keygen'] = "/usr/bin/ssh-keygen";
## The flag to give to ssh / scp to pass it a private key
$UP_config['ssh_key']    = "-i";

###############################################################################
### Other local system commands
$UP_config['remote_cat'] = '/bin/cat';
$UP_config['remote_rm'] = '/bin/rm';

###############################################################################
### Local UPortal commands
$UP_config['dspace_importer']= "/var/www/uportal/uportal-config/bin/lniimport";
$UP_config['chempound_importer']= "/var/www/uportal/uportal-config/bin/qc";

###############################################################################
##  Remote UPortal commands
$UP_config['publish-figshare' ] = 'publish_figshare';
$UP_config['make_staging_directory'] = 'make_staging_directory';
$UP_config['make_input_tar'] = 'make_tar';
$UP_config['delete_job']    =  'delete_job';
$UP_config['run_job']       =  'run_app';
$UP_config['job_status']    =  'job_state';
$UP_config['filetest']      =  'filetest';
$UP_config['app_config']    =  'app_configuration';
$UP_config['pool_status']   =  'pool_status';

###############################################################################
### Publishing
##  DSpace
# e.g. $UP_config['dspace_url']     = "http://scanweb.cc.ic.ac.uk:8080/dspace/handle/";
$UP_config['dspace_url']     = "http://scanweb.cc.ic.ac.uk:8080/dspace/handle/";

## ORCID.  Your institution's ORCID credentials.
$UP_config['orcid_client_id']     = 'XXXX-XXXX-XXXX-XXXX';
$UP_config['orcid_client_secret'] = 'XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX';

## Figshare.  Your institution's Figshare credentials.
$UP_config['figshare_key'] = 'XXXXXXXXXXXXXXXXXXXXXX'; // this is your consumer key
$UP_config['figshare_secret'] = 'XXXXXXXXXXXXXXXXXXXXXX'; // this is your secret

###############################################################################
### Directories
## ssh_keydir.  The webserver must be able to read and write from this directory.
#  UPortal will permanently store the users' ssh keys here.
#  Non-webserver users should NOT be able to write files here.
#  The best solution is to set the directory owned by the webserver and have
#   the permissions 700
$UP_config['ssh_keydir'] = '/opt/portal-dev/deploy/keys';
$UP_config['smarty_compile_dir'] = '/opt/portal-devel/deploy/html/templates_c';

## Temporary directory location on webserver
$UP_config['tempdir'] = '/tmp/';

###############################################################################
### HTTP protocol to use
$UP_config['protocol'] = 'https';

###############################################################################
###############################################################################
# Options that you shouldn't have to worry about follow.
# Set variables that go into $_REQUEST
ini_set('variables_order', 'EGPS'); # exclude ccokies

?>
