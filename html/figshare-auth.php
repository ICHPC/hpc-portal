<?php


include_once '../uportal/oauth/OAuthStore.php';
include_once '../uportal/oauth/OAuthRequester.php';




require '../uportal/smarty/libs/Smarty.class.php';
require '../uportal/uportal-config/uportal-functions2.inc';
require '../uportal/uportal-config/dspace-functions.inc';
require '../uportal/uportal-config/figshareconfig.inc';
session_start();

global $UP_config;
 if( array_key_exists( 'uid', $_SESSION ) && isset( $_SESSION['uid'] ) ) {
	$uid = $_SESSION['uid'];
	testapiAction( make_figshare_nonce($uid) );
 }
 else {
    $proto = $UP_config['protocol'];
   $host  = $_SERVER['HTTP_HOST'];
   $uri   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');
   header("Location: $proto://$host$uri");
 }




  function testapiAction( $nonce )
  {
       global $dbconn;
	global $figshare_key, $figshare_secret;
    global $UP_config;

 
	$key = $figshare_key;
	$secret = $figshare_secret;

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
            $proto = $UP_config['protocol'];
            $host  = $_SERVER['HTTP_HOST'];
            $uri   = $_SERVER['PHP_SELF'];
            $getAuthTokenParams = array(
                'oauth_callback' => "$proto://$host$uri/figshare-auth.php?key=" . $nonce,
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
    $proto = $UP_config['protocol'];
    $host  = $_SERVER['HTTP_HOST'];
    $uri   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');
    $extra = '?action=profile';
    header("Location: $proto://$host$uri/$extra");
 
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
