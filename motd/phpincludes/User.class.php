<?php
require_once('db.php');
define("EUserAdminFlags_None",0);
define("EUserAdminFlags_MatrixManage",1<<0);
define("EUserAdminFlags_UserManage",1<<1);
define("EUserAdminFlags_ServiceManage",1<<2);
define("EUserAdminFlags_AdminManage",1<<3);
define("EUserAdminFlags_MasterManage",1<<4);

class User
{
	/*
		this just loads the data, you must use UserAuth.class.php to check passwords, etc
	*/
	public function __construct($userid) {
		$this->found = false;
		$this->userid = intval($userid);
		 $this->loadUserInfo();
	}
	private $userid;
	private $email;
	private $password;
	private $lastip;
	private $createdate;
	private $useddate;
	private $streetaddr;
	private $streetaddr2;
	private $city;
	private $subscription;
	private $publicmask;
	private $adminrights;
	private $found;
	private $deleted;
	private function loadUserInfo() {
		$query = "SELECT email,lastip,createddate,useddate,streetaddr,streetaddr2,city,subscription,`deleted`,`publicmask`,`adminrights`,`password` FROM `GameTracker`.`users` WHERE `userid` = ".$this->userid."";
		$result = gs_query($query);
		while ($row = mysqli_fetch_row($result)) {
			$this->email = $row[0];
			$this->lastip = $row[1];
			$this->createdate = $row[2];
			$this->useddate = $row[3];
			$this->streetaddr = $row[4];
			$this->streetaddr2 = $row[5];
			$this->city = $row[6];
			$this->subscription = $row[7];
			$this->deleted = intval($row[8]);
			$this->publicmask = intval($row[9]);
			$this->adminrights = intval($row[10]);
			$this->password = $row[11];
			$this->found = true;
		}
	}
	public function getAdminRights() {
		return $this->adminrights;
	}
	public function getPublicMask() {
		return $this->publicmask;
	}
	private function updateUsedDate($ip) {
		if(!is_int($ip)) {
			$ip = ip2long($ip);
		}
		$query = "UPDATE `GameTracker`.`users` SET lastip = $ip, useddate = CURRENT_TIMESTAMP WHERE `userid` = ".$this->userid."";
	}
	public function getEmail() {
		return $this->email;
	}
	public function getUserid() {
		return $this->userid;
	}
	public function getLastIP() {
		return $this->lastip;
	}
	public static function getUserIDByEmail($email) {
		$escapedemail = mysqli_real_escape_string($email);
		$query = "SELECT userid FROM `GameTracker`.`users` WHERE 'email' = '".$escapedemail."";
	}
	public function getSubscription() {
		return $this->subscription;
	}
	public function isDeleted() {
		return $this->deleted;
	}
	public function found() {
		return $this->found;
	}
	public function getPassword() {
		return $this->password;
	}
	public function getPasswordMD5() {
		return md5($this->password);
	}
}
?>
