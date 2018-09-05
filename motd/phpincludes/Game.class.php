<?php
require_once('db.php');
class Game
{
	public function __construct($gameid) {
		$this->gamefound = false;
		 if(is_int($gameid)) { //load by gameid
			$this->loadByGameID($gameid);
		 } else if(is_string($gameid)) { //load by gamename
			$this->loadByGameName($gameid);
		 }
	}
	private $gameid;
	private $gamename;
	private $secretkey;
	private $description;
	private $queryport;
	private $backendflags;
	private $disabledservices;
	private $gamefound;
	
	private function loadByGameID($gameid) {
			$gameid = intval($gameid);
			$query = "SELECT id,gamename,secretkey,description,queryport,backendflags,disabledservices FROM `Gamemaster`.`games` WHERE `id` = ".$gameid."";
			$result = gs_query($query);
			while($row = mysqli_fetch_row($result)) {
				$this->gameid = intval($row[0]);
				$this->gamename = $row[1];
				$this->secretkey = $row[2];
				$this->description = $row[3];
				$this->queryport = intval($row[4]);
				$this->backendflags = intval($row[5]);
				$this->disabledservices = intval($row[6]);
				$this->gamefound = true;
			}
	}
	private function loadByGameName($gamename) {
		$gamename = mysqli_real_escape_string($GLOBALS['confDBLink'],$gamename);
		$query = "SELECT id FROM `Gamemaster`.`games` WHERE `gamename` = '$gamename' LIMIT 0, 1";
		$result = gs_query($query);
		while($row = mysqli_fetch_row($result)) {
			$this->loadByGameID(intval($row[0]));
		}
	}
	public function found() {
		return $this->gamefound == true;
	}
	public function getBackendFlags() {
		return $this->backendflags;
	}
	public function getGameID() {
		return $this->gameid;
	}
	public function getGameName() {
		return $this->gamename;
	}
	public function getSecretKey() {
		return $this->secretkey;
	}
	public function getDescription() {
		return $this->description;
	}
	public function getQueryPort() {
		return $this->queryport;
	}
	public function getDisabledServices() {
		return $this->disabledservices;
	}
}
?>