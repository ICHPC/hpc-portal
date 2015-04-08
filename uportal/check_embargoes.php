<?
set_include_path(get_include_path() . PATH_SEPARATOR . '../uportal/uportal-config');
require_once 'uportal-functions2.inc';

global $smarty;
$smarty = new Smarty;
$smarty->caching = false;
$smarty->cache_lifetime = 0;
$smarty->compile_check = true;
$smarty->debugging = false;
$smarty->force_compile = true;
$smarty->compile_dir = $UP_options['smarty_compile_dir'];
$smarty->assign( 'url_base', $UP_options['protocol'] .'://' . $UP_options['url_base'] );
$usrs = get_admin_users();
$env_from = !empty($UP_options['email_env_from']) ?
                $UP_options['email_env_from'] : '';
foreach ($usrs as $u) {

    if( $u['blocked'] ) continue;
    $res = get_profile( $u['user_id'] );
    if( !empty( $res['email'] ) &&
#                    new_get_job_list( $uid,          $order, $limit,
         $job_list = new_get_job_list( $u['user_id'], 6,      5,
#           $orderdir=0, $project_id=-1, $offset=0, $filter="",
            1,           -1,             0,         "",
#           $status=0, $published=0, $submittime=0, $embargoed=0,
            3,         0,            0,             3,
#           $embargo_mail_sent=0, $countonly=0 ) {
            1,                   0 ) ) {
#        print ("EMAIL FOR " . $u['uname'] . " to " . $res['email']) . "\n";
        $smarty->assign( "job_list", $job_list );
        $output = $smarty->fetch( 'email.tpl' );
#        print "$output\n";

        $admin_email = get_admin_email();
        $headers = array();
        $headers[] = 'From: ' . $admin_email;
        $headers[] = 'To: ' . $res['email'];
        $headers[] = 'Content-Type: text/html; charset=UTF-8';
        mail( $res['email'], 'Embargoed jobs', $output,
            implode( "\r\n", $headers ), $env_from );
        foreach ($job_list as $job) {
            set_embargo_mail_sent( $job['jid'] );
        }
    }
}

?>
