<?php
require_once('db.php');
class ServiceFile
{
	private $serviceid;
	private $fileid;
	private $name;
	private $data;
	private $modified;
	private $active;
	private $filefound;
	private $length;
	
	private $fsvid;
	private $num1;
	private $num2;
	public function __construct($serviceid, $fileid) {
		$this->filefound = false;
		if(is_object($serviceid)) {
			$this->serviceid = $serviceid->getID();
		} else if(is_int($serviceid)) {
			$this->serviceid = $serviceid;
		}
		if(is_int($fileid)) {
			$this->loadByFileID($fileid);
		} else if(is_string($fileid)) {
			$this->loadByName($fileid);
		}
	}
	private function loadByFileID($id) {
			$query = "SELECT `fileid`,`serviceid`,`name`,`fsvid`,`num1`,`num2`,`data`,unix_timestamp(`modified`),`active`,octet_length(`data`) FROM `MOTDWeb`.`files` WHERE `fileid` = $id";
			$result = gs_query($query);
			while($row = mysqli_fetch_row($result)) {
				$this->fileid = intval($row[0]);
				$this->serviceid = intval($row[1]);
				$this->name = $row[2];
				$this->fsvid = $row[3];
				$this->num1 = $row[4];
				$this->num2 = $row[5];
				$this->data = $row[6];
				$this->modified = $row[7];
				$this->active = intval($row[8]);
				$this->length = intval($row[9]);
				$this->filefound = true;
			}
	}
	private function loadByName($filename) {
		$filename = mysqli_real_escape_string($GLOBALS['confDBLink'],$filename);
		$query = "SELECT `fileid` FROM `MOTDWeb`.`files` WHERE `name` = '$filename' AND `serviceid` = $this->serviceid LIMIT 0, 1";
		$result = gs_query($query);
		while($row = mysqli_fetch_row($result)) {
			$this->loadByFileID(intval($row[0]));
		}
	}
	public function getData() {
		return $this->data;
	}
	public function found() {
		return $this->filefound;
	}
	public function isActive() {
		return $this->active;
	}
	public function getName() {
		return $this->name;
	}
	public function getNum($number) {
		switch($number) {
			default:
			case 1:
			return $this->num1;
			case 2:
			return $this->num2;
		}

	}
	public function getMD5Hash() {
		return md5($this->data);
	}
	public function getfsvid() {
		return $this->fsvid;
	}
	public function getModified() {
		return $this->modified;
	}
	public function getLength() {
		return $this->length;
	}
	public function getID() {
		return $this->fileid;
	}
	public function getServiceID() {
		return $this->serviceid;
	}
	
}
class Service
{

	private $id;
	private $name;
	private $servicefound;
	private $keys;
	private $modified;
	private $active;
	private $files;
	private $filesloaded;
	private $detect;
	private $detectmodified;
	private $productids;//array of productids which this service is visible for
	private $loadedProducts;
	
	public function __construct($id) {
		$this->files = array();
		$this->loadedProducts = false;
		$this->productids = array();
		$this->keys = array();
		$this->servicefound = false;
		$this->filesloaded = false;
		 if(is_int($id)) { //load by id
			$this->loadByID($id);
		 } else if(is_string($id)) { //load by name
			$this->loadByName($id);
		 }
	}
	
	public function loadFiles() { //must be called before any file can be loaded, the reason this isn't called inside the class is for efficency, so if you need the file data you must load it first!
		$query = "SELECT `fileid` FROM `MOTDWeb`.`files` WHERE `serviceid` = ".$this->id;
		$result = gs_query($query);
		$this->filesloaded = true;
		while($row = mysqli_fetch_row($result)) {
			$this->files[] = new ServiceFile($this,intval($row[0]));
		}
	}
	private function loadByID($id) {
			$query = "SELECT `id`,`name`,`keys`,unix_timestamp(`modified`),`active` FROM `MOTDWeb`.`services` WHERE `id` = $id";
			$result = gs_query($query);
			while($row = mysqli_fetch_row($result)) {
				$this->id = intval($row[0]);
				$this->name = $row[1];
				$keydata = $row[2];
				$this->modified = $row[3];
				$this->active = intval($row[4]);
				$this->servicefound = true;
			}
			$keys = explode("\n",$keydata);
			foreach($keys as $key) {
				if(strlen($key) > 0) 
					$this->keys[] = $key;
			}
			$query = "SELECT `keys`,unix_timestamp(`modified`) FROM `MOTDWeb`.`detection` WHERE `id` = ".$this->getID();
			$result = gs_query($query);
			while($row = mysqli_fetch_row($result)) {
				$this->detect = $row[0];
				$this->detectmodified = intval($row[1]);
			}
	}
	private function loadByName($name) {
		$name = mysqli_real_escape_string($GLOBALS['confDBLink'],$name);
		$query = "SELECT id FROM `MOTDWeb`.`services` WHERE `name` = '$name' LIMIT 0, 1";
		$result = gs_query($query);
		while($row = mysqli_fetch_row($result)) {
			$this->loadByID(intval($row[0]));
		}
	}
	public function found() {
		return $this->servicefound == true;
	}
	public function getKeys() {
		return $this->keys;
	}
	public function getModified() {
		return $this->modified;
	}
	public function isActive() {
		return $this->active;
	}
	public function getID() {
		return $this->id;
	}
	public function getName() {
		return $this->name;
	}
	public function getFile($name,$fsvid) {
		if(!$this->filesloaded) {
			$this->loadFiles();
		}
		foreach($this->files as $file) {
			if($file->getName() == $name && $file->getfsvid() == $fsvid) {
				return $file;
			}
		}
		return NULL;
	}
	public function getFiles() {
		if(!$this->filesloaded) {
			$this->loadFiles();
		}
		return $this->files;
	}
	public function getLastModifiedFileTime() {
		$time = 0;
		if(!$this->filesloaded) {
			$this->loadFiles();
		}
		foreach($this->files as $file) {
			if($file->getModified() > $time) {
				$time = $file->getModified();
			}			
		}
		return $time;
	}
	public function getfsvid() {
		if(!$this->filesloaded) {
			$this->loadFiles();
		}
		foreach($this->files as $file) {
				return $file->getfsvid();
		}
		return 0;
	}
	public function getDetect() {
		return $this->detect;
	}
	public function getDetectModified() {
		return $this->detectmodified;
	}
	public function filesLoaded() {
		return $this->filesloaded;
	}
	private function loadProducts() {
		$query = "SELECT `productid` FROM `MOTDWeb`.`visibleservices` WHERE `serviceid` = ".$this->id;
		$result = gs_query($query);
		$this->loadedProducts = true;
		while($row = mysqli_fetch_row($result)) {
			$this->productids[] = intval($row[0]);
		}
	}
	public function isVisibleForProductID($id) {
		if(!$this->loadedProducts) {
			$this->loadProducts();
		}
		foreach($this->productids as $pid) {
			if($pid == $id) {
				return true;
			}
		}
		return false;
	}
}
?>