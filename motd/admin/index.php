<?
require_once ('../../phpincludes/db.php');
require_once ('../../phpincludes/Auth.php');
require_once ('../../phpincludes/User.class.php');
require_once ('../../phpincludes/Matrix.php');
require_once ('header.php');
require_once ('core.php');
echo "<title>OpenSpy Admin Panel</title>";
echo "<center>";
if(isLoggedIn()) {
	showMenu();
} else if(!isset($_SESSION['user']) && isset($_POST['email'])) {
	if(!Auth::tryLogin(Auth::getUseridFromEmail($_POST['email']),$_POST['password'])) {
		echo "Incorrect email or password.<br>";
	} else showMenu();
} else {
	echo "<form name=\"loginform\" method=\"post\">
	Email: <input type=\"text\" name=\"email\" /><br />
	Password: <input type=\"password\" name=\"password\" /><br>
	<input type=\"submit\" value=\"Submit\" /><br>
	</form>";
}
function showMenu() {
	$user = getUser();
	$rights = getAdminRights();
	$email = $user->getEmail();
	echo "Welcome $email<br>";
	if($rights & EUserAdminFlags_MatrixManage) {
		echo "<a href=\"Matrix/\">Matrix Manage</a><br>";
	}
	if($rights & EUserAdminFlags_UserManage) {
		echo "<a href=\"#\">User Manage</a><br>";
	}
	if($rights & EUserAdminFlags_ServiceManage) {
		echo "<a href=\"servicemanage.php\">Services Manage</a><br>";
	}
	if($rights & EUserAdminFlags_AdminManage) {
		echo "<a href=\"#\">Admin Manage</a><br>";
	}
	if($rights & EUserAdminFlags_MasterManage) {
		echo "<a href=\"#\">Master Server Manage</a><br>";
	}
}
echo "</center>";
?>