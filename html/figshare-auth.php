<?php


include_once '../uportal/oauth/OAuthStore.php';
include_once '../uportal/oauth/OAuthRequester.php';




require '../uportal/smarty/libs/Smarty.class.php';
require '../uportal/uportal-config/uportal-functions2.inc';
require '../uportal/uportal-config/dspace-functions.inc';
require '../uportal/uportal-config/figshareconfig.inc';
session_start();

 if( array_key_exists( 'uid', $_SESSION ) && isset( $_SESSION['uid'] ) ) {
	$uid = $_SESSION['uid'];
	testapiAction( make_figshare_nonce($uid) );
 }
 else {
   header( 'Location: https://scanweb.cc.imperial.ac.uk/uportal2' );
 }




  function testapiAction( $nonce )
  {
       global $dbconn;
	global $figshare_key, $figshare_secret;

 
	$key = $figshare_key;
	$secret = $figshare_secret;

//    $key = 'ShakEsqAFUJVDo4T1bKHTg'; // this is your consumer key
//    $secret = 'O93VChA6Z4sdgvg0L1MKIw'; // this is your secret
 
    $host = 'http://api.figshare.com/v1/pbl';
 
    $request_token = $host .'/oauth/request_token';
    $authorize_url = $host .'/oauth/authorize';
    $access_token = $host .'/oauth/access_token';

 
 
    //CREATE an OAUTH SESSION
    $options = array
    (
      'consumer_key' => $key,
      'consumer_secret' => $secret,
      'request_token_uri' => $request_token,
      'authorize_uri' => $authorize_url,
      'access_token_uri' => $access_token,
    );
 
    OAuthStore::instance("Session", $options);
 
    try
    {
      if (empty($_GET["oauth_token"]))
      { 
        $getAuthTokenParams = array(
#'xoauth_displayname' => 'Imperial College High Performance Computing Service Portal', 
             'oauth_callback' => 'https://scanweb.cc.imperial.ac.uk/uportal2/figshare-auth.php?key='.$nonce
	); 
 
        $tokenResultParams = OAuthRequester::requestRequestToken($key, 0, $getAuthTokenParams);
        header("Location: " . $authorize_url . "?oauth_token=" . $tokenResultParams['token']);
      }
      else 
      {
 
        $oauthToken = $_GET["oauth_token"];
//        $oldnonce = $_GET["key"];
 
        $tokenResultParams = $_GET;
 
        try {
            $lalala = OAuthRequester::requestAccessToken($key, $oauthToken, 0, 'POST', $_GET);
        }
        catch (OAuthException2 $e)
        {
            echo "Errors occured x " . $e;
            return;
        }
//        $oauthTokenSecret = $_GET["oauth_token_secret"];

 
//function set_figshare_key_secret( $nonce, $key, $secret ) {

	set_figshare_key_secret( $nonce, $lalala['oauth_token'], $lalala['oauth_token_secret'] );
   	header( 'Location: https://scanweb.cc.imperial.ac.uk/uportal2/?action=profile' );
 
#	printf("<P>NONCE: $nonce" );
#	printf("<P>TOKEN: $oauthToken<p>SECRET: $oauthTokenSecret<P>");
#	print_r($_GET);
      }
    }
    catch(OAuthException2 $e) {
        echo "OAuthException:  " . $e->getMessage();
    }
 
    exit;
  }

?>
