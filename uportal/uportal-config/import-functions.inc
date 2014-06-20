<?

require 'configuration2.inc';

$dbconn = pg_connect($UP_config['conn_string']);


function get_next_seq_id() {
  global $dbconn;
   $query="SELECT nextval('model_seq_id')";
   $result  = pg_query( $dbconn, $query );

   $arr = pg_fetch_array( $result, NULL, PGSQL_ASSOC );
   return $arr['nextval'];


}

?>

