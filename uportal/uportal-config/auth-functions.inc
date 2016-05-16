<?

function ldap_authenticate( $user, $password ) {

# SJC: SSL?

		  global $UP_options;
			for( $i =0; $i< count(  $UP_options['ldap_basedn'] ); $i++ ) {
        $ds=@ldap_connect($UP_options['ldap_host'],$UP_options['ldap_port']);
        # SJC check for failures
        $r = @ldap_search( $ds, $UP_options['ldap_basedn'][$i], 'uid=' . $user );
#print "<P>Trying ".$UP_options['ldap_basedn'][$i];
        if ($r) {
            $result = @ldap_get_entries( $ds, $r);

            if (array_key_exists(0,$result) && $result[0]) {
                if (@ldap_bind( $ds, $result[0]['dn'], $password) ) {
                        # SJC: unbind
									error_log( "ldap_authenticate [".$user."] - SUCCESS\n", 3, "/var/tmp/portal-login.log" );
									return  ($result[0]['gecos'][0] );
                }
								else {
									error_log( "ldap_authenticate [".$user."] - FAILED bind (bad password)\n", 3, "/var/tmp/portal-login.log" );
								}
            }
						else {
							error_log( "ldap_authenticate [".$user."] - FAILED search for fn [ " . $UP_options['ldap_basedn'][$i] ."]\n", 3, "/var/tmp/portal-login.log" );
						}
        }
				else {
							error_log( "ldap_authenticate [".$user."] - FAILED unexpected error from ldap_search()\n", 3, "/var/tmp/portal-login.log" );
				}
			}

		error_log( "ldap_authenticate [".$user."] - FAILED \n", 3, "/var/tmp/portal-login.log" );
    return NULL;
}

# Authenticate by looking in UP_options('file_location')
# Lines in the file have the format:
# user:Full Name:$salt$passwordhashinsomestandardformat
#
# Returns: Full Name if $user is in file and pwdhash matches hashed $password

function file_authenticate($user, $password)
{
    global $UP_options;
    $pwdhandle = fopen($UP_options['file_location'], "r");
    $pattern = "/^$user:([^:]+):([^:]+)/";
    $found = 0;

    if ($pwdhandle)
    {
        $matches = array();
        while(!feof($pwdhandle))
        {
            $line = trim(fgets($pwdhandle));
            if ($found = preg_match($pattern, $line, $matches) == 1) break;
        }

        if ($found)
        {
            # SJC Need PHP 5.5 for password_verify so use own rolled equiv
            $pwdhash = $matches[2];
            if (SC_password_verify($password, $pwdhash))
            {
                $gecos = trim($matches[1]);
                # If authenticated we must return something non null.
                if (empty($gecos)) $gecos = '(no name)';
                return $gecos;
            }
        }
    }
    return NULL;
}

function SC_password_verify($password, $hash)
{
    if (crypt($password, $hash) == $hash) return true;
    return false;
}
?>
