<?
require ('config.php');
function gs_query($query) {
	$res = mysqli_query($GLOBALS['confDBLink'],$query);
	if(!$res) { 
		if($GLOBALS['confDebugMode'] == 1) {			
			echo ('mysql query error: '.mysqli_error($GLOBALS['confDBLink'])."<br>");
			echo "Query: ".$query."<br>";
			die();
		}
	}
	return $res;
}
function multi_query_and_free($query) {
	if (mysqli_multi_query($GLOBALS['confDBLink'],$query)) {
    do {
        if ($result = mysqli_store_result($GLOBALS['confDBLink'])) {
            while ($row = mysqli_fetch_row($result)) {
            }
            mysqli_free_result($result);
        }
    } while (mysqli_next_result($GLOBALS['confDBLink']));
}

}
?>