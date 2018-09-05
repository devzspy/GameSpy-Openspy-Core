<?
require_once('db.php');
require_once('Profile.class.php');
define("OPERPRIVS_NONE",0);
define("OPERPRIVS_INVISIBLE",1<<0);
define("OPERPRIVS_BANEXCEMPT",1<<1);
define("OPERPRIVS_GETOPS",1<<2);
define("OPERPRIVS_GLOBALOWNER",1<<3);
define("OPERPRIVS_GETVOICE",1<<4);
define("OPERPRIVS_OPEROVERRIDE",1<<5);
define("OPERPRIVS_WALLOPS",1<<6);
define("OPERPRIVS_KILL",1<<7);
define("OPERPRIVS_FLOODEXCEMPT",1<<8);
define("OPERPRIVS_LISTOPERS",1<<9);
define("OPERPRIVS_CTCP",1<<10); //and ATM, etc
define("OPERPRIVS_HIDDEN",1<<11);
define("OPERPRIVS_SEEHIDDEN",1<<12);
define("OPERPRIVS_MANIPULATE",1<<13); //can manipulate other peoples keys, etc
define("OPERPRIVS_SERVMANAGE",1<<14);
define("OPERPRIVS_WEBPANEL",1<<15); //permitted to log in to the web panel(doesn't serve any use on the actual server)
define("OPERPRIVS_ALL",-1); 

define("EModeFlags_None",0);
define("EModeFlags_Voice",1<<0);
define("EModeFlags_HalfOp",1<<1);
define("EModeFlags_Op",1<<2);
define("EModeFlags_Owner",1<<3);
define("EModeFlags_Gag",1<<4);
define("EModeFlags_Ban",1<<5);
define("EModeFlags_BanExcempt",1<<6);
define("EModeFlags_Invited", 1<<7);
class UserMode {
	public $chanmask;
	public $comment;
	public $expires;
	public $hostmask;
	public $machineid;
	public $modeflags;
	public $profileid;
	public $setbyhost;
	public $setbynick;
	public $setbypid;
	public $uesrmodeid;
	public $setondate;
	public function __construct($id) {
		$this->usermodeid = strval($id);
		$this->loadInfo();
	}
	private function loadInfo() {
		$query = "SELECT `chanmask`,`comment`,unix_timestamp(`expires`),`hostmask`,`machineid`,`modeflags`,`profileid`,`setbyhost`,`setbynick`,`setbypid`,unix_timestamp(`setondate`) FROM `Matrix`.`chanusermodes` WHERE `usermodeid` = ".$this->usermodeid."";
		$result = gs_query($query);
		while ($row = mysqli_fetch_row($result)) {
			$this->chanmask = $row[0];
			$this->comment = $row[1];
			$this->expires = $row[2];
			$this->hostmask = $row[3];
			$this->machineid = $row[4];
			$this->modeflags = $row[5];
			$this->profileid = $row[6];
			$this->setbyhost = $row[7];
			$this->setbynick = $row[8];
			$this->setbypid = $row[9];
			$this->setondate = $row[10];
		}
	}
};
	class Oper {
		public function __construct($profileid) {
			$this->profileid = strval($profileid);
			$this->loadUserInfo();
		}
		private $userid;
		private $profileid;
		private $profile;
		private $rightsmask;
		private $name;
		private $email;
		private $setondate;
		
		public static function getOperByUserID($userid) {
			//finds the oper with the highest rightsmask
			$query = "SELECT `Matrix`.`globalopers`.`profileid` FROM `Matrix`.`globalopers` INNER JOIN `GameTracker`.`profiles` ON `GameTracker`.`profiles`.`profileid` = `Matrix`.`globalopers`.`profileid` WHERE `GameTracker`.`profiles`.`userid` = '".strval($userid)."' ORDER BY `rightsmask` DESC LIMIT 0,1";
			$result = gs_query($query);
			if(mysqli_num_rows($result) == 0) return NULL;
			while ($row = mysqli_fetch_row($result)) {
				$retpro = $row[0];
			}
			return new Oper($retpro);
		}
		private function loadUserInfo() {
			$query = "SELECT `email`,`nick`,`rightsmask`,unix_timestamp(`setondate`) FROM `Matrix`.`globalopers` WHERE `profileid` = '".$this->profileid."'";
			$result = gs_query($query);
			while ($row = mysqli_fetch_row($result)) {
				$this->email = $row[0];
				$this->name = $row[1];
				$this->rightsmask = $row[2];
				//2011-9-21- 23:41:42 date("o-n-d G:i:s",
				$this->setondate = $row[3];
			}
			$this->profile = new Profile($this->profileid);
		}
		public function hasRight($right) {
			return $right & $this->rightsmask;
		}
		public function getRightsMask() {
			return $this->rightsmask;
		}
		public function getName() {
			return $this->name;
		}
		public function getEmail() {
			return $this->email;
		}
		public function getProfile() {
			return $this->profile;
		}
		public function getProfileID() {
			return $this->profileid;
		}
		public function getSetOnDate() {
			return $this->setondate;
		}
	}
	class ChanClient {
		public $chanmask;
		public $gameid;
		public function __construct($chanmask,$gameid) {
			$this->chanmask = $chanmask;
			$this->gameid = $gameid;
		}
	};
	class ChanProp {
		public $chanmask;
		public $chankey;
		public $comment;
		public $entrymsg;
		public $expires;
		public $groupname;
		public $limit;
		public $mode;
		public $onlyowner;
		public $setbynick;
		public $setbypid;
		public $setbyhost;
		public $setondate;
		public $topic;
		public function __construct($chanmask) {
			$this->chanmask = mysqli_real_escape_string($GLOBALS['confDBLink'],$chanmask);
			$this->loadInfo();
		}
		private function loadInfo() {
		$query = "SELECT `comment`,`chankey`,`entrymsg`,unix_timestamp(`expires`),`groupname`,`limit`,`mode`,`onlyowner`,`setbynick`,unix_timestamp(`setondate`),`topic`,`setbyhost` FROM `Matrix`.`chanprops` WHERE `chanmask` = '".$this->chanmask."'";
		$result = gs_query($query);
		while ($row = mysqli_fetch_row($result)) {
			$this->comment = $row[0];
			$this->chankey = $row[1];
			$this->entrymsg = $row[2];
			$this->expires = $row[3];
			$this->groupname = $row[4];
			$this->limit = $row[5];
			$this->mode = $row[6];
			$this->onlyowner = $row[7];
			$this->setbynick = $row[8];
			$this->setondate = $row[9];
			$this->topic = $row[10];
			$this->setbyhost = $row[11];
		}
			
		}
	}
function getProfileRights($profileid) {
	
	$profile = new Profile($profileid);
	if($profile->found()) {
		if($profile->getAdminRights() & EUserAdminFlags_MatrixManage) {
			return OPERPRIVS_ALL;
		}
	}
	$query = "SELECT `rightsmask` FROM `Matrix`.`globalopers` WHERE `profileid` = ".intval($profileid)."";
	$result = gs_query($query);
	if(mysqli_num_rows($result) == 0) {
		return 0;
	}
	while ($row = mysqli_fetch_row($result)) {
		return $row[0];
	}
	return 0;
}
function listUserModes() {
	$list = array();
	$query = "SELECT `usermodeid` FROM `Matrix`.`chanusermodes`";
	$result = gs_query($query);
	while ($row = mysqli_fetch_row($result)) {
		$id = $row[0];
		$usermode = new UserMode($id);
		$list[] = $usermode;		
	}
	return $list;
}
function listChanProps() {
	$list = array();
	$query = "SELECT `chanmask` FROM `Matrix`.`chanprops`";
	$result = gs_query($query);
	while ($row = mysqli_fetch_row($result)) {
		$chanmask = $row[0];
		$chanprop = new ChanProp($chanmask);
		$list[] = $chanprop;		
	}
	return $list;
}
function listOpers() {
	$list = array();
	$query = "SELECT `profileid` FROM `Matrix`.`globalopers`";
	$result = gs_query($query);
	while ($row = mysqli_fetch_row($result)) {
		$id = $row[0];
		$usermode = new Oper($id);
		$list[] = $usermode;		
	}
	return $list;
}
function listChanClients() {
	$list = array();
	$query = "SELECT `gameid`,`chanmask` FROM `Matrix`.`chanclients`";
	$result = gs_query($query);
		while ($row = mysqli_fetch_row($result)) {
		$chanclient = new ChanClient($row[1],$row[0]);
		$list[] = $chanclient;
		
	}
	return $list;
}
function modeStrToFlags($str) {
	$flags = 0;
	for($i=0;$i<strlen($str);$i++) {
		switch($str[$i]) {
		case 'v':
			$flags |= EModeFlags_Voice;
			break;
		case 'h':
			$flags |= EModeFlags_HalfOp;
			break;
		case 'o':
			$flags |= EModeFlags_Op;
			break;
		case 'O':
			$flags |= EModeFlags_Owner;
			break;
		case 'b':
			$flags |= EModeFlags_Ban;
			break;
		case 'g':
			$flags |= EModeFlags_Gag;
			break;
		case 'I':
			$flags |= EModeFlags_Invited;
			break;
		case 'B':
			$flags |= EModeFlags_BanExcempt;
			break;
		}
	}
	return $flags;
}
function modeFlagstoStr($flags) {
$flags = strval($flags);
	$str = "";
	if(($flags & EModeFlags_Voice)) {
		$str .= "v";
	}
	if($flags & EModeFlags_HalfOp) {
		$str .= "h";
	}
	if($flags & EModeFlags_Op) {
		$str .= "o";
	}
	if($flags & EModeFlags_Owner) {
		$str .= "O";
	}
	if($flags & EModeFlags_Ban) {
		$str .= "b";
	}
	if($flags & EModeFlags_Gag) {
		$str .= "g";
	}
	if($flags & EModeFlags_Invited) {
		$str .= "I";
	}
	if($flags & EModeFlags_BanExcempt) {
		$str .= "B";
	}
	return $str;
}
function setGlobalOper($profileid, $rightsmask, $name, $email) {
	$pid = mysqli_real_escape_string($GLOBALS['confDBLink'],$profileid);
	$rmask = mysqli_real_escape_string($GLOBALS['confDBLink'],$rightsmask);
	$escapedemail = mysqli_real_escape_string($GLOBALS['confDBLink'],$email);
	$escapednick = mysqli_real_escape_string($GLOBALS['confDBLink'],$name);
	$query = "CALL Matrix.SetGlobalOper($pid,$rmask,\"".$escapedemail."\",\"".$escapednick."\")";
	multi_query_and_free($query);
}
function deleteGlobalOper($profileid) {
	$pid = mysqli_real_escape_string($GLOBALS['confDBLink'],$profileid);
	$query = "CALL Matrix.DelGlobalOper(".$pid.")";
	multi_query_and_free($query);
}
function AddUserMode($chanmask,$comment,$expires,$hostmask,$machineid,$modeflags,$profileid,$setbyhost,$setbynick,$setbypid) {
	if(!is_int($modeflags)) {
		$modeflags = modeStrToFlags($modeflags);
	}
	$machineid = mysqli_real_escape_string($GLOBALS['confDBLink'],$machineid);
	$hostmask = mysqli_real_escape_string($GLOBALS['confDBLink'],$hostmask);
	$chanmask = mysqli_real_escape_string($GLOBALS['confDBLink'],$chanmask);
	$comment = mysqli_real_escape_string($GLOBALS['confDBLink'],$comment);
	$setbynick = mysqli_real_escape_string($GLOBALS['confDBLink'],$setbynick);
	$setbypid = mysqli_real_escape_string($GLOBALS['confDBLink'],$setbypid);
	$setbyhost = mysqli_real_escape_string($GLOBALS['confDBLink'],$setbyhost);
	$expires = mysqli_real_escape_string($GLOBALS['confDBLink'],$expires);
	if($expires != "0") {
		$expiresstr = "TIMESTAMP(\"$expires\")";
	} else {
		$expiresstr = "0";
	}
	if(!isset($profileid) || strlen($profileid) == 0) {
		$profileid = 0;
	} else {
		$profileid = mysqli_real_escape_string($GLOBALS['confDBLink'],$profileid);
	}
	$query = "CALL Matrix.SetUserMode(\"$chanmask\",\"$comment\",$expiresstr,\"$hostmask\",\"$machineid\",$modeflags,$profileid,\"".$setbyhost."\",\"$setbynick\",".$setbypid.");";
	multi_query_and_free($query);
}
function deleteUserMode($usermodeid) {
	$id = mysqli_real_escape_string($GLOBALS['confDBLink'],$usermodeid);
	$query = "CALL Matrix.DelUserMode(".$id.")";
	multi_query_and_free($query);
}
function deleteChanProp($chanmask,$kickexisting) {
	$mask = mysqli_real_escape_string($GLOBALS['confDBLink'],$chanmask);
	$kick = mysqli_real_escape_string($GLOBALS['confDBLink'],$kickexisting);
	$query = "CALL Matrix.DelChanProps(\"".$mask."\",$kick)";
	multi_query_and_free($query);
}
function setChanProp($chanmask,$chankey,$comment,$topic,$entrymsg,$groupname,$limit,$onlyowner,$mode,$expires,$setbynick,$setbypid,$setbyhost,$kickexisting) {
	$chanmask = mysqli_real_escape_string($GLOBALS['confDBLink'],$chanmask);
	$chankey = mysqli_real_escape_string($GLOBALS['confDBLink'],$chankey);
	$comment = mysqli_real_escape_string($GLOBALS['confDBLink'],$comment);
	$topic = mysqli_real_escape_string($GLOBALS['confDBLink'],$topic);
	$entrymsg = mysqli_real_escape_string($GLOBALS['confDBLink'],$entrymsg);
	$groupname = mysqli_real_escape_string($GLOBALS['confDBLink'],$groupname);
	$limit = intval($limit);
	$onlyowner = intval($onlyowner);
	$mode = mysqli_real_escape_string($GLOBALS['confDBLink'],$mode);
	$expires = mysqli_real_escape_string($GLOBALS['confDBLink'],$expires);
	$setbynick = mysqli_real_escape_string($GLOBALS['confDBLink'],$setbynick);
	$setbyhost = mysqli_real_escape_string($GLOBALS['confDBLink'],$setbyhost);
	$setbypid = intval($setbypid);
	$kickexisting = intval($kickexisting);
	if($expires != "0") {
		$expiresstr = "TIMESTAMP(\"$expires\")";
	} else {
		$expiresstr = "0";
	}
	$query = "CALL Matrix.SetChanProps(\"$chanmask\",\"$chankey\",\"$comment\",\"$entrymsg\",$expiresstr,\"$groupname\",$limit,\"$mode\",$onlyowner,\"$setbynick\",\"$setbyhost\",$setbypid,\"$topic\",$kickexisting)";
	multi_query_and_free($query);
}
function deleteChanClient($chanmask,$gameid) {
	$chanmask = mysqli_real_escape_string($GLOBALS['confDBLink'],$chanmask);
	$gameid = intval($gameid);
	$query = "CALL Matrix.DelChanClient(\"$chanmask\",$gameid)";
	multi_query_and_free($query);
}
function setChanClient($chanmask,$gameid) {
	$chanmask = mysqli_real_escape_string($GLOBALS['confDBLink'],$chanmask);
	$gameid = intval($gameid);
	$query = "CALL Matrix.SetChanClient(\"$chanmask\",$gameid)";
	multi_query_and_free($query);
}
function getUsermodeChanMask($usermodeid) {
	$usermodeid = intval($usermodeid);
	$query = "SELECT `chanmask` FROM `Matrix`.`chanusermodes` WHERE `usermodeid` = '$usermodeid'";
	$result = gs_query($query);
	while ($row = mysqli_fetch_row($result)) {
		$chanmask = $row[0];
	}
	return $chanmask;
}
?>