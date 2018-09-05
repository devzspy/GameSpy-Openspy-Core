<?php
require_once('db.php');
class Profile
{
	/*
		this just loads the data, you must use UserAuth.class.php to check passwords, etc
	*/
	public function __construct($profileid) {
		$this->found = false;
		$this->profileid = intval($profileid);
		$this->loadUserInfo();
	}
	private $userid;
	private $profileid;
	private $nick;
	private $uniquenick;
	private $firstname;
	private $lastname;
	private $icquin;
	private $deleted;
	private $user;	
	private $found;
	
	private function loadUserInfo() {
		$query = "SELECT userid,nick,uniquenick,firstname,lastname,icquin,deleted,aimname,pic,createddate,useddate FROM `GameTracker`.`profiles` WHERE `profileid` = ".strval($this->profileid)."";
		$result = gs_query($query);
		while ($row = mysqli_fetch_row($result)) {
			$this->userid = $row[0];
			$this->nick = $row[1];
			$this->uniquenick = $row[2];
			$this->firstname = $row[3];
			$this->lastname = $row[4];
			$this->icquin = $row[5];
			$this->deleted = $row[6];
			$this->aimname = $row[7];
			$this->pic = $row[8];
			$this->createddate = $row[9];
			$this->useddate = $row[10];
			$this->found = true;
		}
		$this->user = new User($this->getUserID());
	}
	public function hasUnique() {
		return strlen($this->uniquenick) > 0;
	}
	public function getUserID() {
		return $this->userid;
	}
	public function found() {
		return $this->found;
	}
	public function getProfileID() {
		return $this->userid;
	}
	public function getUser() {
		return $this->user;
	}
	public function getAdminRights() {
		return $this->user->getAdminRights();
	}
	public function getNick($unique) {
		if($unique) {
			return $this->uniquenick;
		} else {
			return $this->nick;
		}
	}
	public function getPassword() {
		$user = $this->getUser();
		return $user->getPassword();
	}
	public function getPasswordMD5() {
		$user = $this->getUser();
		return $user->getPasswordMD5();
	}
	public function getFirstName() {
		return $this->firstname;
	}
	public function getLastName() {
		return $this->lastname;
	}
	
}
?>