<?

function ldap_authenticate( $user, $password ) {

# SJC: SSL?

		  global $UP_config;
			for( $i =0; $i< count(  $UP_config['ldap_basedn'] ); $i++ ) {
        $ds=@ldap_connect($UP_config['ldap_host'],$UP_config['ldap_port']);
        # SJC check for failures
        $r = @ldap_search( $ds, $UP_config['ldap_basedn'][$i], 'uid=' . $user );
#print "<P>Trying ".$UP_config['ldap_basedn'][$i];
        if ($r) {
            $result = @ldap_get_entries( $ds, $r);

            if ($result[0]) {
                if (@ldap_bind( $ds, $result[0]['dn'], $password) ) {
                        # SJC: unbind
						return  ($result[0]['gecos'][0] );
                }
            }
        }
			}
    
    return NULL;
}

# Authenticate by looking in UP_config('file_location')
# Lines in the file have the format:
# user:Full Name:$salt$passwordhashinsomestandardformat
#
# Returns: Full Name if $user is in file and pwdhash matches hashed $password

function file_authenticate($user, $password)
{
    global $UP_config;
    $pwdhandle = fopen($UP_config['file_location'], "r");
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
