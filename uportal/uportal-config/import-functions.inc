<?

require 'configuration2.inc';
require_once 'db-functions.inc';

function get_next_seq_id() {
    #$query="SELECT nextval('model_seq_id')";
    $query="SELECT nextval('model_seq_id')";
    $arr = db_query( $query, array() );

    return $arr['nextval'];
}

?>

