<?
require ('../../../phpincludes/db.php');
require ('../../../phpincludes/Auth.php');
require ('../../../phpincludes/User.class.php');
require ('../../../phpincludes/Matrix.php');
$maxclubs = 10;
/*ERR_NODESC
ERR_TITLEINUSE
ERR_CLUBLIMIT
ERR_NOTOWNER
ERR_NOTSUB
ERR_FAILURE
OK_CREATE
OK_MODIFY
OK_DESTROY
OK_GETPW
OK_GETDESC
 
=getpw&chan=#gsp!cc_10053_E1EF2F
*/
if(!isset($_GET['cmd']) || !isset($_GET['pid']) || !isset($_GET['passmd5'])) {
	die('ERR_FAILURE\\Missing required parameter');
}
$pid = intval($_GET['pid']);
$profile = new Profile($pid);
if(!$profile->found()) {
	die('ERR_FAILURE\\Profile non-existant');
}
if($profile->getPasswordMD5() != $_GET['passmd5']) {
	die('ERR_FAILURE\\Incorrect Password');
}
switch($_GET['cmd']) {
	case "create":
		handleCreate($pid);
	break;
	case "destroy":
		handleDestroy($pid);
	break;
	case "modify":
		handleModify($pid);
	break;
	case "getpw":
		handleGetPW($pid);
	break;
}
function checkIfChanExistsByTitle($title) { //its assumed title is escaped
	$query = "SELECT 1 FROM `Matrix`.`chanprops` WHERE `topic` = \"$title\" AND `chanmask` LIKE \"#gsp!cc_%\"";
	$result = gs_query($query);
	return mysqli_num_rows($result) > 0;
}
function getNumChans($profileid) {
	if(!is_int($profileid)) return 0;
	$query = "SELECT 1 FROM `Matrix`.`chanprops` WHERE `chanmask` LIKE \"#gsp!cc_$profileid_%\"";
	$result = gs_query($query);
	return mysqli_num_rows($result);
}
function chanExists($name) {
	$query = "SELECT 1 FROM `Matrix`.`chanprops` WHERE `chanmask` = \"$name\"";
	$result = gs_query($query);
	return mysqli_num_rows($result) > 0;
}
function getRandomChan($profileid) {
	
	do {
		$ran[0] = rand(0,255);
		$ran[1] = rand(0,255);
		$ran[2] = rand(0,255);
		$name = "#gsp!cc_".$profileid."_";
		$name .= strtoupper(dechex($ran[0]));
		$name .= strtoupper(dechex($ran[1]));
		$name .= strtoupper(dechex($ran[2]));
	} while(chanExists($name));
	return $name;
	
}
function handleCreate($pid) {
	$maxclubs = $GLOBALS['maxclubs'];
	$title = mysqli_real_escape_string($GLOBALS['confDBLink'],$_GET['title']);	
	$profile = new Profile($pid);
	$user = $profile->getUser();
	if($user->getSubscription() <= 1) { //founders and above can make user-rooms
		die('ERR_NOTSUB\\You must be a founder club member or above to make a userroom.');
	}
	if(checkIfChanExistsByTitle($title)) {
		die('ERR_TITLEINUSE\\This title is already in use');
	}
	if(getNumChans($profile->getProfileID()) >= $maxclubs) {
		die('ERR_CLUBLIMIT\\You have reached the maximum limit of $maxclubs userrooms');
	}
	$key = "";
	if(isset($_GET['privpw'])) {
		$key = $_GET['privpw'];
	}
	$name = getRandomChan($pid);
	setChanProp($name,$key,"User-Created room",$title,"","chatclub",0,1,"tnz","0",$profile->getNick(true),$pid,$_SERVER['REMOTE_ADDR'],0);
	AddUserMode($name,"Chat Club Owner","0","","","O",$pid,$_SERVER['REMOTE_ADDR'],$profile->getNick(true),$pid);
	echo "OK_CREATE\\$name\\$title";
	
}
function checkifOwns($chan,$pid) {
	$chaninfo = explode("_",$chan);
	if($chaninfo[0] == "#gsp!cc") {
		if($chaninfo[1] == $pid) {
			return true;
		}
	}
	return false;
}
function handleDestroy($pid) {
	$chan = explode("chan=",$_SERVER['REQUEST_URI']);
	if(!isset($chan[1])) {
		die('ERR_FAILURE\\Missing required parameter');
	} else {
		if(strstr($chan[1],"&") != false) {
			$channame = strstr($chan[1],"&",true);
		} else {
			$channame = $chan[1];
		}
	}
	$channame = mysqli_real_escape_string($GLOBALS['confDBLink'],$channame);	
	if(!checkIfOwns($channame,$pid)) {
		die('ERR_NOTOWNER\\You do not own this channel.');
	}
	if(!chanExists($channame)) {
		die('ERR_NOTOWNER\\You do not own this channel.');
	}
	$query = "SELECT `usermodeid` FROM `Matrix`.`chanusermodes` WHERE `chanmask` = \"$channame\"";
	$result = gs_query($query);
	while($row = mysqli_fetch_row($result)) {
		deleteUserMode(intval($row[0]));
	}
	deleteChanProp($channame,1);
	echo "OK_DESTROY";
}
function handleModify($pid) {
	$chan = explode("chan=",$_SERVER['REQUEST_URI']);
	if(!isset($chan[1])) {
		die('ERR_FAILURE\\Missing required parameter');
	} else {
		if(strstr($chan[1],"&") != false) {
			$channame = strstr($chan[1],"&",true);
		} else {
			$channame = $chan[1];
		}
	}
	
	$temptitle = explode("title=",$_SERVER['REQUEST_URI']);
	if(!isset($temptitle[1])) {
		die('ERR_FAILURE\\Missing required parameter');
	} else {
		if(strstr($temptitle[1],"&") != false) {
			$title = strstr($temptitle[1],"&",true);
		} else {
			$title = $temptitle[1];
		}
	}
	
	$temppass = explode("privpw=",$_SERVER['REQUEST_URI']);
	if(!isset($temppass[1])) {
		$pass = "";
		//die('ERR_FAILURE\\Missing required parameter');
	} else {
		if(strstr($temppass[1],"&") != false) {
			$pass = strstr($temppass[1],"&",true);
		} else {
			$pass = $temppass[1];
		}
	}
	$profile = new Profile($pid);
	$channame = mysqli_real_escape_string($GLOBALS['confDBLink'],$channame);	
	if(!checkIfOwns($channame,$pid)) {
		die('ERR_NOTOWNER\\You do not own this channel.'.$channame);
	}
	if(!chanExists($channame)) {
		die('ERR_NOTOWNER\\You do not own this channel.'.$channame);
	}
	if($pass != "") {
		$pass = mysqli_real_escape_string($GLOBALS['confDBLink'],$pass);
	}
	$title = urldecode($title);
	$title = mysqli_real_escape_string($GLOBALS['confDBLink'],$title);
	setChanProp($channame,$pass,"User-Created room",$title,"","chatclub",0,1,"tnz","0",$profile->getNick(true),$pid,$_SERVER['REMOTE_ADDR'],0);
	echo "OK_MODIFY\\$channame\\$title\\$pass";
}
function handleGetPW($pid) {
	$chan = explode("chan=",$_SERVER['REQUEST_URI']);
	if(!isset($chan[1])) {
		die('ERR_FAILURE\\Missing required parameter');
	} else {
		if(strstr($chan[1],"&") != false) {
			$channame = strstr($chan[1],"&",true);
		} else {
			$channame = $chan[1];
		}
	}
	$profile = new Profile($pid);
	$channame = mysqli_real_escape_string($GLOBALS['confDBLink'],$channame);	
	if(!checkIfOwns($channame,$pid)) {
		die('ERR_NOTOWNER\\You do not own this channel.'.$channame);
	}
	if(!chanExists($channame)) {
		die('ERR_NOTOWNER\\You do not own this channel.'.$channame);
	}
	$chanprop = new ChanProp($channame);
	$pass = $chanprop->chankey;
	echo "OK_GETPW\\$pass";
}
?>
