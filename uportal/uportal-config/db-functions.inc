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
        print "Error: " . $e->getMessage() . "\n";
        die();
    }
    return $db;
}

global $UP_config;
$db = db_connect($UP_config['db_user'], $UP_config['db_password'], $UP_config['db_host'], $UP_config['db_port'], $UP_config['db_dbname']);

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
        print "Error: " . $e->getMessage() . "\n";
        die();
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
        print "Error: " . $e->getMessage() . "\n";
        die();
    }
    return NULL;
}

?>
