<?
require_once ('../../phpincludes/db.php');
require_once ('../../phpincludes/Auth.php');
require_once ('../../phpincludes/User.class.php');
require_once ('../../phpincludes/Matrix.php');
require_once ('header.php');

function isLoggedIn() {
if(!isset($_SESSION['user'])) return false;
	$user = $_SESSION['user'];
	if($user->found()) {
		return true;
	}
	return false;
}
function getAdminRights() {
	if(!isLoggedIn()) {
		return 0;
	}
	$user = $_SESSION['user'];
	return $user->getAdminRights();
}
function getUser() {
	if(!isset($_SESSION['user'])) return NULL;
	$user = $_SESSION['user'];
	if($user->found()) {
		return $user;
	}
	return NULL;
}
?>