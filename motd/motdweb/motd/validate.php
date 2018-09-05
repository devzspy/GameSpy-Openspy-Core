<?
include('../../phpincludes/config.php');
include('../../phpincludes/db.php');
include('../../phpincludes/User.class.php');
$user = new User($_GET['userid']);
if($user->getUserID() == intval($_GET['userid']) && $user->getEmail() == $_GET['email'] && !$user->isDeleted() && $user->getSubscription()) {
	$valid = $user->getSubscription();
	if($valid > 0) $valid = 1;
	$retstr = "\\isreg\\".$valid;
	if(isset($_GET['cookie'])) {
		$cookie = getCookie(intval($_GET['cookie']));
		$retstr .= "\\cookie\\".$cookie."\\";
	}
	echo $retstr;
} else echo "\\isreg\\0";
function getCookie($cookie) {
	$retval = ($cookie*10)-239;
	$retval -= (16*(floor($cookie/4)));
	if(shouldInc($cookie)) {
		$retval += 256;
	}
	return $retval;
}
function shouldInc($cookie) {
	$x = floor($cookie/43)-1; //estimate, can be off(on the low end) by a bit
	$multiplier = 43;
	while(true) {
		$lastinc = floor((61+($multiplier*$x)-($x/3)));
		if(($cookie-$lastinc >= 0 && $cookie-$lastinc <= 21)) {
			return true;
		} else if($cookie-$lastinc < 0) {
			break;
		}
		$x++;
	}
	return false;
}
?>
