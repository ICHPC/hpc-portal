<?
# Parse the configuration file.  Also set any
# PHP options we need.

# Set variables that go into $_REQUEST
ini_set('variables_order', 'EGPS'); // exclude cookies

# Read config file.
$UP_options = parse_ini_file( '../config/configuration.ini' );

if( !$UP_options ) {
    print "No config file\n";
    die;
}

###############################################################################
### Error reporting.
if( isset($UP_options['error_reporting']))
    ini_set('error_reporting', $UP_options['error_reporting']);
if( isset($UP_options['display_errors']))
    ini_set('display_errors', $UP_options['display_errors']);
if( isset($UP_options['log_errors']))
    ini_set('log_errors', $UP_options['log_errors']);
if( isset($UP_options['error_log']))
    ini_set('error_log', $UP_options['error_log']);

###############################################################################
### Smarty template engine.  Either version 2 or 3 should work.
if( empty($UP_options['smarty_location']) ) {
    require_once('Smarty/Smarty.class.php');
} else {
    require_once($UP_options['smarty_location']);
}

?>
