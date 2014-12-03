#!/usr/bin/php
<?
#
# Changes a uportal user's admin status.
#
# Usage: makeadmin.php [-u] username
#

require_once "../uportal/uportal-config/configuration.inc";
require_once "../uportal/uportal-config/db-functions.inc";

$remove = false;
if( count($argv) < 2 or count($argv) > 3 ) { usage();}
if( $argv[1] == '-u' ) {
    $remove = true;
    array_shift( $argv );
}

if( count($argv) != 2 ) { usage();}
$user = strtolower($argv[1]);
print ($remove ? "Unsetting" : "Setting") . " " . $user . "...";

# CBA faffing with PHP -> SQL booleans
if( $remove ) {
    $query = "UPDATE users SET is_admin=false WHERE username = ?";
} else {
    $query = "UPDATE users SET is_admin=true,is_blocked=false WHERE username = ?";
}
$arr = db_query( $query, array($user) );

if ($arr != 0) {
    print " success.\n";
} else {
    print " failed!\n";
}

function usage() {
    print "Usage: makeadmin.php [-u] username\n";
    print "Sets (-u unsets) username as admin.\n";
    print "Will unblock user if setting as admin\n";
    die;
}
?>
