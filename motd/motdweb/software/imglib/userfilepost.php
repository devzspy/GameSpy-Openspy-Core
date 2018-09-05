<?
require ('../../phpincludes/db.php');
require ('../../phpincludes/Auth.php');
require ('../../phpincludes/User.class.php');
require ('../../phpincludes/Matrix.php');

if(!isset($_GET['pid']) || !isset($_GET['uid']) || !isset($_GET['ppass']) || !isset($_GET['picnum'])) {
	die('ERR_FAILURE\\Missing required parameter');
}

$pid = intval($_GET['pid']);
$profile = new Profile($pid);
if(!$profile->found()) {
	die('ERR_FAILURE\\Profile non-existant');
}
?>
