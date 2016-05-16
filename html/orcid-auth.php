<?php


include_once '../uportal/oauth/OAuthStore.php';
include_once '../uportal/oauth/OAuthRequester.php';




require '../uportal/uportal-config/uportal-functions2.inc';
require '../uportal/uportal-config/dspace-functions.inc';

session_name( 'UPORTAL' . $UP_options['version'] );
session_start();

$proto = $UP_options['protocol'];
$host  = $_SERVER['HTTP_HOST'];
$uri   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');
if( !array_key_exists( 'uid', $_SESSION ) ) {
    header("Location: $proto://$host$uri");
    exit(0);
}

print_r( $_GET );
print_r( $_SESSION );

if( !array_key_exists( 'code', $_GET ) ) {
    $_SESSION["oauthnonce"] = md5(uniqid(rand(), true));
    $redirect_uri = "$proto://$host$uri/orcid-auth.php?state=" . $_SESSION["oauthnonce"];
    header( 'Location: https://orcid.org/oauth/authorize?client_id=' . $UP_options['orcid_client_id'] .'&response_type=code&scope=/authenticate&redirect_uri=' . $redirect_uri );
}
else {
//  curl -i -L -H 'Accept: application/json' --data 'client_id=0000-0001-7197-7095&client_secret=2801423d-88b0-4809-b6d8-87eede5ec00c&grant_type=authorization_code&code=MsjXNS' 'https://api.sandbox.orcid.org/oauth/token'
    $req = new HttpRequest( "https://pub.orcid.org/oauth/token", HTTP_METH_POST );
    $req->addHeaders( array("Accept"=> "application/json")  );
    $req->addPostFields ( array(
            "client_id" => $UP_options['orcid_client_id'],
            "client_secret" => $UP_options['orcid_client_secret'],
            "grant_type" => "authorization_code",
            "code" => $_GET["code"]
        )  );
    $req->send();
    $req->getResponseBody() ;
    $req->getResponseCode() ;
    $obj = json_decode( $req->getResponseBody() );
    if( $req->getResponseCode() === 200  || $req->getResponseCode() === 302 ) {
        if( $_GET["state"] === $_SESSION[ "oauthnonce" ] ) {
            var_dump( $obj );
            $token = $obj->{"access_token"};
            $orcid = $obj->{"orcid"};
            echo( "$token :: $orcid" );
            $uid = $_SESSION["uid"];
            $query = 'UPDATE profile SET orcid=? WHERE user_id=?';
            $results = db_query( $query, array($orcid, $uid) );
            $extra = '?action=profile';
            header( "Location: $proto://$host$uri/$extra" );

        }
        else {
            echo ("Nonce mismatch");
        }


    }
    else {
        echo( "Failed: ". $req->getResponseBody()  );
        echo( "Code: ".  $req->getResponseCode() );
    }
}
