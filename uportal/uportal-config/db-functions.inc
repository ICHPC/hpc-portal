<?
function db_connect($dbuser, $dbpassword, $dbhost, $dbport, $dbname)
{
    try
    {
        $dsn = "pgsql:host=$dbhost;port=$dbport;dbname=$dbname";
        #$db = new PDO('pgsql:host=localhost;dbname=uportal;port=5432', $dbuser, $dbpassword);
        $db = new PDO($dsn, $dbuser, $dbpassword);
        $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    } catch (PDOException $e)
    {
        $err = "Database connection error: " . $e->getMessage() .
            ", at " . $e->getLine() . " of " . $e->getFile() . "\n";
        die("$err\n");
    }
    return $db;
}

global $UP_options;
$db = db_connect($UP_options['db_user'], $UP_options['db_password'], $UP_options['db_host'], $UP_options['db_port'], $UP_options['db_dbname']);

function db_query($query, array $values)
{
    global $db;
    try
    {
        $row = NULL;
        $st = $db->prepare($query);
        if ($st->execute($values)) $row = $st->fetch(PDO::FETCH_ASSOC);
        return $row;
    } catch (PDOException $e)
    {
        $err = "Database error: " . $e->getMessage() .
            ", at " . $e->getLine() . " of " . $e->getFile() . "\n" .
            "doing $query\nwith values:";
        $err .= print_r($values, true);
        die("$err\n");
    }
    return NULL;
}

function db_query_all($query, array $values)
{
    global $db;
    try
    {
        $row = NULL;
        $st = $db->prepare($query);
        if ($st->execute($values)) $row = $st->fetchAll(PDO::FETCH_ASSOC);
        return $row;
    } catch (PDOException $e)
    {
        print "Database error: " . $e->getMessage() .
            ", at " . $e->getLine() . " of " . $e->getFile() . "\n" .
            "doing $query\nwith values:";
        print_r($values);
        print "\n";
        die();
    }
    return NULL;
}

?>
