<?
require ('../../../phpincludes/db.php');
require ('../../../phpincludes/Auth.php');
require ('../../../phpincludes/User.class.php');
require ('../../../phpincludes/Matrix.php');
require ('header.php');
echo "<head><script language=\"javascript\" type=\"text/javascript\" src=\"datetimepicker.js\"></script></head><center>";
if(!isset($_SESSION['user']) && !isset($_POST['email'])) {
echo "<form name=\"loginform\" method=\"post\">
Email: <input type=\"text\" name=\"email\" /><br />
Password: <input type=\"password\" name=\"password\" /><br>
<input type=\"submit\" value=\"Submit\" /><br>
</form>";
} else if(!isset($_SESSION['user']) && isset($_POST['email'])) {
	if(!Auth::tryLogin(Auth::getUseridFromEmail($_POST['email']),$_POST['password'])) {
		echo "Incorrect email or password.<br>";
	} else showMenu();
} else if(isset($_SESSION['user'])) {
	showMenu();
} else {
}
function displayOptions($oper) {
	if(isUserAdmin()) {
		$rightsmask = OPERPRIVS_ALL;
		echo "Welcome ".$_SESSION['user']->getEmail()."<br>";
	} else {
		$rightsmask = $oper->getRightsMask();
		echo "Welcome ".$oper->getName()."<br>";
	}
	if($rightsmask & OPERPRIVS_SERVMANAGE) {
		echo "<a href=\"?action=opermanage\">Add/Remove Opers</a><br>";
	}
	if($rightsmask & OPERPRIVS_GLOBALOWNER) {
		echo "<a href=\"?action=chanpropmanage\">Add/Delete ChanProps</a><br>";
	}
	if($rightsmask & OPERPRIVS_GLOBALOWNER) {
		echo "<a href=\"?action=usermodemanage\">Add/Delete Usermodes</a><br>";
	}
	if($rightsmask & OPERPRIVS_SERVMANAGE) {
		echo "<a href=\"?action=chanclientmanage\">Add/Delete ChanClients</a><br>";
	}
}
function showAction($action) { //prints out the available options and
if($action == "usermodemanage" && !isset($_GET['type'])) {
	echo "<a href=\"?action=usermodemanage&type=setmode\">Add User Mode</a><br>";
		$modes = listUserModes();
		echo "Server usermodes: 
		<form name=\"input\" method=\"get\">
		<table border=\"1\">
	<tr>
	<th>Channel Mask</th>
	<th>Comment</th>
	<th>Host Mask</th>
	<th>Machine ID</th>
	<th>Profile ID</th>
	<th>Mode Flags</th>
	<th>Set by nick</th>
	<th>Set by PID</th>
	<th>Set by host</th>
	<th>Set on date</th>
	<th>Expires</th>
	<th>User Mode ID</th>
	<th>Delete</th>
	</tr>
	";
		foreach($modes as $usermode) {
		if($usermode->expires != 0) {
			$expiresstr = date("o-n-d G:i:s",$usermode->expires);
		} else {
			$expiresstr = "";
		}
		if($usermode->setondate != 0) {
			$setonstr = date("o-n-d G:i:s",$usermode->setondate);
		} else {
			$setonstr = "";
		}
		$oper = Oper::getOperByUserID($_SESSION['user']->getUserid());
		if(strtolower($usermode->chanmask) == "x") {
			if(!($oper->getRightsMask() & OPERPRIVS_KILL)) { //don't list klines, etc if they can't set it
				continue;
			}
		}
			echo 
			"<tr><th>".$usermode->chanmask."</th>
			<th>".$usermode->comment."</th>
			<th>".$usermode->hostmask."</th>
			<th>".$usermode->machineid."</th>
			<th>".$usermode->profileid."</th>
			<th>".modeFlagstoStr(intval($usermode->modeflags))."</th>			
			<th>".$usermode->setbynick."</th>
			<th>".$usermode->setbypid."</th>
			<th>".$usermode->setbyhost."</th>
			<th>".$setonstr."</th>
			<th>".$expiresstr."</th>
			<th>".$usermode->usermodeid."</th>
			<th><input type=\"checkbox\" name=\"usermodeid[]\" value=\"".$usermode->usermodeid."\" /></th>
			</tr>";
		}
		echo "</table><br><input type=\"hidden\" name=\"action\" value=\"usermodemanage\"><input type=\"hidden\" name=\"do\" value=\"delete\"><input type=\"submit\" value=\"Submit\" /><br></form>";
	} else if($action == "usermodemanage" && $_GET['type'] == "setmode") {
			echo "<form method=\"get\">
		  Channel Mask: <input type=\"text\" name=\"chanmask\" /><br />
		  Comment: <input type=\"text\" name=\"comment\" /><br />
		  Host Mask: <input type=\"text\" name=\"hostmask\" /><br />
		  Machine ID: <input type=\"text\" name=\"machineid\" /><br />
		  Profile ID: <input type=\"text\" name=\"profileid\" /><br />
		  Mode Flags: <input type=\"text\" name=\"modeflags\" /><br />
		  Expires: <input id=\"expires\" type=\"text\" size=\"25\" name=\"expires\" value=\"0000-0-00 00:00:00\"><a href=\"javascript:NewCal('expires','YYYYMMDD',true,24)\"><img src=\"cal.gif\" width=\"16\" height=\"16\" border=\"0\" alt=\"Pick a date\"></a>
		  <input type=\"hidden\" name=\"action\" value=\"usermodemanage\"><input type=\"hidden\" name=\"do\" value=\"set\"><br><input type=\"submit\" value=\"Submit\" /><br></form>";
	} else if($action == "opermanage") {
		checkPermissions(OPERPRIVS_SERVMANAGE);
		$opers = listOpers();
		echo "Server Operators: <br>
		<a href=\"?action=addoper\">Add Oper</a><br>
		<form name=\"input\" method=\"get\">
		<table border=\"1\">
	<tr>
	<th>Profile ID</th>
	<th>Rights Mask</th>
	<th>Name</th>
	<th>Email</th>
	<th>Set on date</th>
	<th>Edit</th>
	<th>Delete</th>
	</tr>
	";
		foreach($opers as $oper) {
		if($oper->getSetOnDate() != 0) {
			$expiresstr = date("o-n-d G:i:s",$oper->getSetOnDate());
		} else {
			$expiresstr = "";
		}

			echo 
			"<tr><th>".$oper->getProfileID()."</th>
			<th>".$oper->getRightsMask()."</th>
			<th>".$oper->getName()."</th>
			<th>".$oper->getEmail()."</th>
			<th>".$expiresstr."</th>
			<th><a href=\"?action=operedit&profileid=".$oper->getProfileID()."\">Edit</a></th>
			<th><a href=\"?action=opermanage&do=delete&profileid=".$oper->getProfileID()."\">Delete</a></th>
			</tr>";
		}
	} else if($action == "operedit" || $action == "addoper") {
			checkPermissions(OPERPRIVS_SERVMANAGE);
			if(!isset($_GET['profileid'])) {
				$profileid = 0;
			} else {
				$profileid = $_GET['profileid'];
			}
				showOperEdit($profileid);
	} else if($action == "chanpropmanage") {
		$props = listChanProps();
		echo "Server channel props: <br>
		<a href=\"?action=chanpropedit\">Add Chan Prop</a><br>
		<form name=\"input\" method=\"get\">
		<table border=\"1\">
	<tr>
	<th>Channel Mask</th>
	<th>Channel Key</th>
	<th>Comment</th>
	<th>Topic</th>
	<th>Entry Message</th>
	<th>Group Name</th>
	<th>Limit</th>
	<th>Modes</th>
	<th>Only Owner</th>
	<th>Expires</th>
	<th>Set by nick</th>
	<th>Set by PID</th>
	<th>Set by host</th>
	<th>Set on date</th>
	<th>Edit</th>
	<th>Delete</th>
	</tr>
	";
		foreach($props as $prop) {
			$urlmask = str_replace("#","%23",$prop->chanmask);
		if($prop->expires != 0) {
			$expiresstr = date("o-n-d G:i:s",$prop->expires);
		} else {
			$expiresstr = "";
		}
		$setondate = date("o-n-d G:i:s",$prop->setondate);
			echo 
			"<tr><th>".$prop->chanmask."</th>
			<th>".$prop->chankey."</th>
			<th>".$prop->comment."</th>
			<th>".$prop->topic."</th>
			<th>".$prop->entrymsg."</th>
			<th>".$prop->groupname."</th>
			<th>".$prop->limit."</th>
			<th>".$prop->mode."</th>
			<th>".$prop->onlyowner."</th>
			<th>".$expiresstr."</th>
			<th>".$prop->setbynick."</th>
			<th>".$prop->setbypid."</th>
			<th>".$prop->setbyhost."</th>
			<th>".$setondate."</th>
			<th><a href=\"?action=chanpropedit&chanmask=".$urlmask."\">Edit</a></th>
			<th><a href=\"?action=chanpropmanage&do=delete&chanmask=".$urlmask."\">Delete</a></th>
			</tr>";
		}
	} else if($action == "chanpropedit") {
		showChanPropEdit($_GET['chanmask']);
	} else if($action == "chanclientmanage") {
		$chanclients = listChanClients();
		echo "Chan Clients: <br>
		<a href=\"?action=chanclientadd\">Add Chan Client</a><br>
		<form name=\"input\" method=\"get\">
		<table border=\"1\">
	<tr>
	<th>Channel Mask</th>
	<th>Game ID</th>
	<th>Delete</th>
	</tr>
	";
	foreach($chanclients as $chanclient) {
		$urlmask = str_replace("#","%23",$chanclient->chanmask);
			echo 
			"<tr><th>".$chanclient->chanmask."</th>
			<th>".$chanclient->gameid."</th>
			<th><a href=\"?action=chanclient&do=delete&chanmask=".$urlmask."&gameid=".$chanclient->gameid."\">Delete</a></th>
			</tr>";
	}
	} else if($action == "chanclientadd") {
			echo "<form method=\"get\">
		  Channel Mask: <input type=\"text\" name=\"chanmask\" /><br />
		  Game ID: <input type=\"text\" name=\"gameid\" /><br />
		  <input type=\"hidden\" name=\"action\" value=\"chanclient\"><input type=\"hidden\" name=\"do\" value=\"set\"><br><input type=\"submit\" value=\"Submit\" /><br></form>";
	}
}
function handleAction() {
	if($_GET['action'] == "opermanage" && $_GET['do'] == "delete") {
		checkPermissions(OPERPRIVS_SERVMANAGE);
		deleteGlobalOper($_GET['profileid']);
		echo "<meta http-equiv=\"refresh\" content=\"5; url=?action=opermanage\">Deleted!<br>";
	} else if($_GET['action'] == "opermanage" && $_GET['do'] == "set") {
		checkPermissions(OPERPRIVS_SERVMANAGE);
		setGlobalOper($_GET['profileid'],$_GET['rightsmask'],$_GET['name'],$_GET['email']);
		echo "<meta http-equiv=\"refresh\" content=\"5; url=?action=opermanage\"> Oper Added!<br>";
	}else if($_GET['action'] == "usermodemanage" && $_GET['do'] == "delete") {
		checkPermissions(OPERPRIVS_GLOBALOWNER);
		$oper = Oper::getOperByUserID($_SESSION['user']->getUserid());
		foreach($_GET['usermodeid'] as $userid) {
			if(strtolower(getUsermodeChanMask($userid)) == "x") {
				if(!($oper->getRightsMask() & OPERPRIVS_KILL)) { //don't list klines, etc if they can't set it
					echo "<meta http-equiv=\"refresh\" content=\"5; url=?action=usermodemanage\">Permission Denied!<br>";
					return 0;
				}
			}
			deleteUserMode($userid);
		}
	echo "<meta http-equiv=\"refresh\" content=\"5; url=?action=usermodemanage\">Deleted!<br>";
	} else if($_GET['action'] == "usermodemanage" && $_GET['do'] == "set") {
		checkPermissions(OPERPRIVS_GLOBALOWNER);
		$oper = Oper::getOperByUserID($_SESSION['user']->getUserid());
		$opername = mysqli_real_escape_string($GLOBALS['confDBLink'],$oper->getName());
		if(strtolower($_GET['chanmask']) == "x") {
			if(!($oper->getRightsMask() & OPERPRIVS_KILL)) { //don't list klines, etc if they can't set it
				echo "<meta http-equiv=\"refresh\" content=\"5; url=?action=usermodemanage\">Permission Denied!<br>";
				return 0;
			}
		
		}
		AddUserMode($_GET['chanmask'],$_GET['comment'],$_GET['expires'],$_GET['hostmask'],$_GET['machineid'],$_GET['modeflags'],$_GET['profileid'],$_SERVER['REMOTE_ADDR'],$opername,$oper->getProfileID());
		echo "<meta http-equiv=\"refresh\" content=\"5; url=?action=usermodemanage\">Usermode Added!<br>";
	} else if($_GET['action'] == "chanpropmanage" && $_GET['do'] == "delete") {
		checkPermissions(OPERPRIVS_GLOBALOWNER);
		deleteChanProp($_GET['chanmask'],true);
		echo "<meta http-equiv=\"refresh\" content=\"5; url=?action=chanpropmanage\">Channel Props deleted!<br>";
	} else if($_GET['action'] == "chanpropmanage" && $_GET['do'] == "set") {
		checkPermissions(OPERPRIVS_GLOBALOWNER);
		$oper = Oper::getOperByUserID($_SESSION['user']->getUserid());
		$opername = mysqli_real_escape_string($GLOBALS['confDBLink'],$oper->getName());
		if(isset($_GET['kickexisting'])) {
			$kickexisting = $_GET['kickexisting'];
		} else {
			$kickexisting = 0;
		}
		if(isset($_GET['onlyowner'])) {
			$onlyowner = $_GET['onlyowner'];
		} else {
			$onlyowner = 0;
		}
		if(isset($_GET['expires']) && strlen($_GET['expires'])>0) {
			$expires = $_GET['expires'];
		} else {
			$expires = "0";
		}
		setChanProp($_GET['chanmask'],$_GET['chankey'],$_GET['comment'],$_GET['topic'],$_GET['entrymsg'],$_GET['groupname'],$_GET['limit'],$onlyowner,$_GET['mode'],$expires,$opername,$oper->getProfileID(),$_SERVER['REMOTE_ADDR'],$kickexisting);
		echo "<meta http-equiv=\"refresh\" content=\"5; url=?action=chanpropmanage\">Channel Props set!<br>";
	} else if($_GET['action'] == "chanclient" && $_GET['do'] == "delete") {
		deleteChanClient($_GET['chanmask'],$_GET['gameid']);
		echo "<meta http-equiv=\"refresh\" content=\"5; url=?action=chanclientmanage\">Chan Client Deleted!<br>";
	} else if($_GET['action'] == "chanclient" && $_GET['do'] == "set") {
		setChanClient($_GET['chanmask'],$_GET['gameid']);
		echo "<meta http-equiv=\"refresh\" content=\"5; url=?action=chanclientmanage\">Chan Client Set!<br>";
	}
}
function showMenu() {
	$operprivs = Oper::getOperByUserID($_SESSION['user']->getUserid());
	if(($operprivs == NULL || (~$operprivs->getRightsMask() & OPERPRIVS_WEBPANEL)) && !isUserAdmin()) {
		die('You\'re not an admin.');
	} else if (!isset($_GET['action'])) {
		displayOptions($operprivs);
	} else {
		if(!isset($_GET['do'])) {
			showAction($_GET['action']);
		} else {
			handleAction();
		}
	}
}
function showOperEdit($profileid) {
echo "<script type=\"text/javascript\">
var OPERPRIVS_NONE = 0;
var OPERPRIVS_INVISIBLE = 1<<0;
var OPERPRIVS_BANEXCEMPT = 1<<1;
var OPERPRIVS_GETOPS = 1<<2;
var OPERPRIVS_GLOBALOWNER = 1<<3;
var OPERPRIVS_GETVOICE = 1<<4;
var OPERPRIVS_OPEROVERRIDE = 1<<5;
var OPERPRIVS_WALLOPS = 1<<6;
var OPERPRIVS_KILL = 1<<7;
var OPERPRIVS_FLOODEXCEMPT = 1<<8;
var OPERPRIVS_LISTOPERS = 1<<9;
var OPERPRIVS_CTCP = 1<<10; //and ATM, etc
var OPERPRIVS_HIDDEN = 1 << 11;
var OPERPRIVS_SEEHIDDEN = 1 << 12;
var OPERPRIVS_MANIPULATE = 1 << 13; //can manipulate other peoples keys, etc
var OPERPRIVS_SERVMANAGE = 1 << 14;
var OPERPRIVS_WEBPANEL = 1 << 15;
function setRights(x) {
	document.getElementById('rightsmask').value = x;
	updateChecks();
}
function updateInput() {
	var currights = document.getElementById('rightsmask').value;
	if(document.getElementById('canbe_invisible').checked) {
		currights |= OPERPRIVS_INVISIBLE;
	} else {
		currights &= ~OPERPRIVS_INVISIBLE;
	}
	if(document.getElementById('ban_excempt').checked) {
		currights |= OPERPRIVS_BANEXCEMPT;
	} else {
		currights &= ~OPERPRIVS_BANEXCEMPT;
	}
	if(document.getElementById('get_ops').checked) {
		currights |= OPERPRIVS_GETOPS;
	} else {
		currights &= ~OPERPRIVS_GETOPS;
	}
	if(document.getElementById('global_owner').checked) {
		currights |= OPERPRIVS_GLOBALOWNER;
	} else {
		currights &= ~OPERPRIVS_GLOBALOWNER;
	}
	if(document.getElementById('get_voice').checked) {
		currights |= OPERPRIVS_GETVOICE;
	} else {
		currights &= ~OPERPRIVS_GETVOICE;
	}
	if(document.getElementById('oper_override').checked) {
		currights |= OPERPRIVS_OPEROVERRIDE;
	} else {
		currights &= ~OPERPRIVS_OPEROVERRIDE;
	}
	if(document.getElementById('can_wallops').checked) {
		currights |= OPERPRIVS_WALLOPS;
	} else {
		currights &= ~OPERPRIVS_WALLOPS;
	}
	if(document.getElementById('can_kill').checked) {
		currights |= OPERPRIVS_KILL;
	} else {
		currights &= ~OPERPRIVS_KILL;
	}
	if(document.getElementById('flood_excempt').checked) {
		currights |= OPERPRIVS_FLOODEXCEMPT;
	} else {
		currights &= ~OPERPRIVS_FLOODEXCEMPT;
	}
	if(document.getElementById('list_opers').checked) {
		currights |= OPERPRIVS_LISTOPERS;
	} else {
		currights &= ~OPERPRIVS_LISTOPERS;
	}
	if(document.getElementById('can_ctcp').checked) {
		currights |= OPERPRIVS_CTCP;
	} else {
		currights &= ~OPERPRIVS_CTCP;
	}
	if(document.getElementById('is_hidden').checked) {
		currights |= OPERPRIVS_HIDDEN;
	} else {
		currights &= ~OPERPRIVS_HIDDEN;
	}
	if(document.getElementById('see_hidden').checked) {
		currights |= OPERPRIVS_SEEHIDDEN;
	} else {
		currights &= ~OPERPRIVS_SEEHIDDEN;
	}
	if(document.getElementById('can_manipulate').checked) {
		currights |= OPERPRIVS_MANIPULATE;
	} else {
		currights &= ~OPERPRIVS_MANIPULATE;
	}
	if(document.getElementById('serv_manage').checked) {
		currights |= OPERPRIVS_SERVMANAGE;
	} else {
		currights &= ~OPERPRIVS_SERVMANAGE;
	}
	if(document.getElementById('web_panel').checked) {
		currights |= OPERPRIVS_WEBPANEL;
	} else {
		currights &= ~OPERPRIVS_WEBPANEL;
	}
	
	document.getElementById('rightsmask').value = currights;
}
function updateChecks() {
	var rightsmask = document.getElementById('rightsmask').value;
	if(rightsmask & OPERPRIVS_INVISIBLE) {
		document.getElementById('canbe_invisible').checked = true;
	} else {
		document.getElementById('canbe_invisible').checked = false;
	}
	if(rightsmask & OPERPRIVS_BANEXCEMPT) {
		document.getElementById('ban_excempt').checked = true;
	} else {
		document.getElementById('ban_excempt').checked = false;
	}
	if(rightsmask & OPERPRIVS_GETOPS) {
		document.getElementById('get_ops').checked = true;
	} else {
		document.getElementById('get_ops').checked = false;
	}
	if(rightsmask & OPERPRIVS_GLOBALOWNER) {
		document.getElementById('global_owner').checked = true;
	} else {
		document.getElementById('global_owner').checked = false;
	}
	if(rightsmask & OPERPRIVS_GETVOICE) {
		document.getElementById('get_voice').checked = true;
	} else {
		document.getElementById('get_voice').checked = false;
	}
	if(rightsmask & OPERPRIVS_OPEROVERRIDE) {
		document.getElementById('oper_override').checked = true;
	} else {
		document.getElementById('oper_override').checked = false;
	}
	if(rightsmask & OPERPRIVS_WALLOPS) {
		document.getElementById('can_wallops').checked = true;
	} else {
		document.getElementById('can_wallops').checked = false;
	}
	if(rightsmask & OPERPRIVS_KILL) {
		document.getElementById('can_kill').checked = true;
	} else {
		document.getElementById('can_kill').checked = false;
	}
	if(rightsmask & OPERPRIVS_FLOODEXCEMPT) {
		document.getElementById('flood_excempt').checked = true;
	} else {
		document.getElementById('flood_excempt').checked = false;
	}
	if(rightsmask & OPERPRIVS_LISTOPERS) {
		document.getElementById('list_opers').checked = true;
	} else {
		document.getElementById('list_opers').checked = false;
	}
	if(rightsmask & OPERPRIVS_CTCP) {
		document.getElementById('can_ctcp').checked = true;
	} else {
		document.getElementById('can_ctcp').checked = false;
	}
	if(rightsmask & OPERPRIVS_HIDDEN) {
		document.getElementById('is_hidden').checked = true;
	} else {
		document.getElementById('is_hidden').checked = false;
	}
	if(rightsmask & OPERPRIVS_SEEHIDDEN) {
		document.getElementById('see_hidden').checked = true;
	} else {
		document.getElementById('see_hidden').checked = false;
	}
	if(rightsmask & OPERPRIVS_MANIPULATE) {
		document.getElementById('can_manipulate').checked = true;
	} else {
		document.getElementById('can_manipulate').checked = false;
	}
	if(rightsmask & OPERPRIVS_SERVMANAGE) {
		document.getElementById('serv_manage').checked = true;
	} else {
		document.getElementById('serv_manage').checked = false;
	}
	if(rightsmask & OPERPRIVS_WEBPANEL) {
		document.getElementById('web_panel').checked = true;
	} else {
		document.getElementById('web_panel').checked = false;
	}
}
</script>";
	if(isset($profileid) && $profileid != 0) {
		$oper = new Oper($profileid);
		echo "<body onload=\"setRights(".$oper->getRightsMask().")\"><form method=\"get\">
		Profile ID: <input type=\"text\" name=\"profileid\" value=\"".$oper->getProfileID()."\" disabled=\"disabled\"/><br />
		Rightsmask: <input type=\"text\" id=\"rightsmask\" name=\"rightsmask\"/ onchange=\"updateChecks()\"><br />
		Name: <input type=\"text\" name=\"name\" value=\"".$oper->getName()."\"/><br />
		Email: <input type=\"text\" name=\"email\" value=\"".$oper->getEmail()."\"/><br />
		Can be invisible: <input id=\"canbe_invisible\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Ban Excempt: <input id=\"ban_excempt\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Get Ops: <input id=\"get_ops\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Global Owner(can set global chan props, etc): <input id=\"global_owner\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Get Voice: <input id=\"get_voice\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Oper Override: <input id=\"oper_override\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Can use /wallops: <input id=\"can_wallops\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Can use /kill, set modes on X, etc: <input id=\"can_kill\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Flood Excempt: <input id=\"flood_excempt\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Can /listopers, /listusers, set modes +cjp: <input id=\"list_opers\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Can send CTCP requests, and use /ATM: <input id=\"can_ctcp\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Hidden from /whois, /getkey: <input id=\"is_hidden\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Can see those who are hidden(have above flag): <input id=\"see_hidden\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Can /setckey on other people, /setkey other people: <input id=\"can_manipulate\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Server manager(can add opers, change chan clients): <input id=\"serv_manage\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		Can access the web panel: <input id=\"web_panel\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		<input type=\"hidden\" name=\"action\" value=\"opermanage\"> <input type=\"hidden\" name=\"profileid\" value=\"".$oper->getProfileID()."\"><input type=\"hidden\" name=\"do\" value=\"set\"><input type=\"submit\" value=\"Submit\" /><br></form>";
	} else {
			echo "<body onload=\"setRights(0)\"><form method=\"get\">
		  Profile ID: <input type=\"text\" name=\"profileid\"/><br />
		  Rightsmask: <input type=\"text\" id=\"rightsmask\" name=\"rightsmask\"\" onchange=\"updateChecks()\"/><br />
		  Name: <input type=\"text\" name=\"name\"\"/><br />
		  Email: <input type=\"text\" name=\"email\"\"/><br />
		  Can be invisible: <input id=\"canbe_invisible\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Ban Excempt: <input id=\"ban_excempt\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Get Ops: <input id=\"get_ops\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Global Owner(can set global chan props, etc): <input id=\"global_owner\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Get Voice: <input id=\"get_voice\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
	      Oper Override: <input id=\"oper_override\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Can use /wallops: <input id=\"can_wallops\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Can use /kill, set modes on X, etc: <input id=\"can_kill\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Flood Excempt: <input id=\"flood_excempt\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Can /listopers, /listusers, set modes +cjp: <input id=\"list_opers\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Can send CTCP requests, and use /ATM: <input id=\"can_ctcp\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Hidden from /whois, /getkey: <input id=\"is_hidden\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Can see those who are hidden(have above flag): <input id=\"see_hidden\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Can /setckey on other people, /setkey other people: <input id=\"can_manipulate\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Server manager(can add opers, change chan clients): <input id=\"serv_manage\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  Can access the web panel: <input id=\"web_panel\" type=\"checkbox\" onclick=\"updateInput()\" /><br>
		  <input type=\"hidden\" name=\"action\" value=\"opermanage\"><input type=\"hidden\" name=\"do\" value=\"set\"><input type=\"submit\" value=\"Submit\" /><br></form>";
	}
}
function showChanPropEdit($chanmask) {
	if(isset($chanmask)) {
		$prop = new ChanProp($chanmask);
		if($prop->expires != 0) {
			$expiresstr = date("o-n-d G:i:s",$prop->expires);
		} else {
			$expiresstr = "";
		}
		echo "<form method=\"get\">
		Channel Mask: <input type=\"text\" name=\"chanmask\" value=\"".$prop->chanmask."\" disabled=\"disabled\"/><br />
		Channel Key: <input type=\"text\" name=\"chankey\" value=\"".$prop->chankey."\"/><br />
		Comment: <input type=\"text\" name=\"comment\" value=\"".$prop->comment."\"/><br />
		Topic: <input type=\"text\" name=\"topic\" value=\"".$prop->topic."\"/><br />
		Entry Message: <input type=\"text\" name=\"entrymsg\" value=\"".$prop->entrymsg."\"/><br />
		Group Name: <input type=\"text\" name=\"groupname\" value=\"".$prop->groupname."\"/><br />
		Limit: <input type=\"text\" name=\"limit\" value=\"".$prop->limit."\"/><br />
		Modes: <input type=\"text\" name=\"mode\" value=\"".$prop->mode."\"/><br />
		Only Owner: <input type=\"checkbox\" name=\"onlyowner\" ".$prop->onlyowner==true?"checked":""."/><br />
		Expires: <input id=\"expires\" type=\"text\" size=\"25\" name=\"expires\" value=\"".$expiresstr."\"><a href=\"javascript:NewCal('expires','YYYYMMDD',true,24)\"><img src=\"cal.gif\" width=\"16\" height=\"16\" border=\"0\" alt=\"Pick a date\"></a><br>
		Kick Existing: <input type=\"checkbox\" name=\"kickexisting\"/><br />
		<input type=\"hidden\" name=\"chanmask\" value=\"".$prop->chanmask."\"><br>
		<input type=\"hidden\" name=\"action\" value=\"chanpropmanage\">
		<input type=\"hidden\" name=\"do\" value=\"set\">
		<br><input type=\"submit\" value=\"Submit\" />
		";
	} else {
		echo "<form method=\"get\">
		Channel Mask: <input type=\"text\" name=\"chanmask\"/><br />
		Channel Key: <input type=\"text\" name=\"chankey\"/><br />
		Comment: <input type=\"text\" name=\"comment\"/><br />
		Topic: <input type=\"text\" name=\"topic\"/><br />
		Entry Message: <input type=\"text\" name=\"entrymsg\"/><br />
		Group Name: <input type=\"text\" name=\"groupname\"/><br />
		Limit: <input type=\"text\" name=\"limit\"/><br />
		Modes: <input type=\"text\" name=\"mode\"\"/><br />
		Only Owner: <input type=\"checkbox\" name=\"onlyowner\"/><br />
		Expires: <input id=\"expires\" type=\"text\" size=\"25\" name=\"expires\"><a href=\"javascript:NewCal('expires','YYYYMMDD',true,24)\"><img src=\"cal.gif\" width=\"16\" height=\"16\" border=\"0\" alt=\"Pick a date\"></a><br>
		Kick Existing: <input type=\"checkbox\" name=\"kickexisting\" /><br />
		<input type=\"hidden\" name=\"action\" value=\"chanpropmanage\">
		<input type=\"hidden\" name=\"do\" value=\"set\">
		<br><input type=\"submit\" value=\"Submit\" />
		";
	}
}
function checkPermissions($rightsmask) {
	$operprivs = Oper::getOperByUserID($_SESSION['user']->getUserid());
	if(isUserAdmin()) {
		return;
	}
	if($operprivs == NULL || !($operprivs->getRightsMask() & $rightsmask)) {
		die('You\'re not an admin.');
	}
}
function isUserAdmin() {
	if($_SESSION['user']->getAdminRights() & EUserAdminFlags_MatrixManage) {
		return true;
	}
	return false;
}
echo "</center>";
?>
