<?
require_once('db.php');
class Auth {
public static function tryLogin($userid,$pass) {
	if(Auth::tryPass($userid,$pass)) {
		$_SESSION['userid'] = $userid;
		$_SESSION['user'] = new User($userid);
		return 1;
	} else {
		return 0;
	}
}
public static function tryPass($userid,$pass) {
	$escapepass = mysqli_real_escape_string($GLOBALS['confDBLink'],$pass);
	$escapeuser = mysqli_real_escape_string($GLOBALS['confDBLink'],$userid);
	$query = "SELECT 1 FROM `GameTracker`.`users` WHERE `userid` = ".$escapeuser." AND `password` = '".$escapepass."'";
	$result = gs_query($query);
	if(mysqli_num_rows($result) > 0) {
		return 1;
	}
	return 0;
}
public static function getUseridFromEmail($email) {
	$escapeemail = mysqli_real_escape_string($GLOBALS['confDBLink'],$email);
	$query = "SELECT `userid` FROM `GameTracker`.`users` WHERE `email` = '".$escapeemail."'";
	$result = gs_query($query);
	if(mysqli_num_rows($result) == 0) {
		return 0;
	}
	while ($row = mysqli_fetch_row($result)) {
		return $row[0];
	}
	
}
public static function getUserID() {
	if(isset($_SESSION['userid'])) {
		return $_SESSION['userid'];
	}
	return 0;
}
public static function setLoggedInProfile($profileid) {
	$user = new User($_SESSION['userid']);
	if($user->getUserid() != 0) {
		$profile = new Profile($profileid);
		if($profile->getUserID() == $user->getUserid()) {
			$_SESSION['profileid'] = $profileid;
		}
	}
}
public static function isUserLoggedIn() {
	return Auth::getUserID() != 0;
}

public static function getUser() {
	if(!isset($_SESSION['user'])) return NULL;
	$user = $_SESSION['user'];
	if($user->found()) {
		return $user;
	}
	return NULL;
}
}
?>