<?
include ('../../../phpincludes/config.php');
include ('../../../phpincludes/db.php');
include ('../../../phpincludes/Service.class.php');
if(isset($_GET['services'])) {
$services = explode("\\",$_GET['services']);
} else {
	$services = array();
}
header('content-disposition: blah; filename=info.txt');
$product = -1;
if(isset($_GET['productid'])) {
$product = intval($_GET['productid']);
}
switch($_GET['mode']) {
	case "files":
	sendFiles($services);
	break;
	case "detect":
	sendDetect();
	break;
	case "full":
	sendFull($services);
	break;
	case "modified":
	default:
	sendModified();
	break;
	//used for retrieving games
	//format: gamename - detection modified time | full modified time | files modified time
}
function productVisibleForService($serviceid,$productid) {
		if($productid == -1) return true;
		$query = "SELECT 1 FROM `MOTDWeb`.`visibleservices` WHERE `serviceid` = ".intval($serviceid)." AND `productid` =".intval($productid);
		$result = gs_query($query);
		return mysqli_num_rows($result) > 0;
}
function sendFull($services) {
	$sendstr = "";
	foreach($services as $servicename) {
		$service = new Service($servicename);
		if(!$service->found()) continue;
		$sendstr .= "[".$service->getName()."]\n";
		$sendstr .= "fpmt=".$service->getModified()."\n";
		$keys = $service->getKeys();
		foreach($keys as $key) {
			$sendstr .= $key."\n";
		}
	}
	header('Content-Type: text/plain; charset=Windows-1252');
	header('Content-Length: '.strlen($sendstr));
	echo $sendstr;
}
function sendDetect() {
	$query = "SELECT `services`.`name`,`detection`.`keys`,unix_timestamp(`detection`.`modified`),`services`.`id` FROM `MOTDWeb`.`services` INNER JOIN `MOTDWeb`.`detection` ON `detection`.`id` = `services`.`id` WHERE `services`.`active` = 1  ORDER BY `MOTDWeb`.`detection`.`modified` DESC";
	$result = gs_query($query);
	$sendstr = "";
	$since = 0;
	if(isset($_GET['since'])) {
		$since = intval($_GET['since']);
	}
	while($row = mysqli_fetch_row($result)) {
		if(productVisibleForService(intval($row[3]),$GLOBALS['product'])) {
			if(intval($row[2]) > $since) {
				$sendstr .= "[".$row[0]."]\n";
				$sendstr .= "dmt=".$row[2]."\n";
				$sendstr .= $row[1];
			}
		}
	}
	header('Content-Type: text/plain; charset=Windows-1252');
	header('Content-Length: '.strlen($sendstr));
	echo $sendstr;
}
function sendModified() {
	$query = "SELECT `services`.`name`,unix_timestamp(`services`.`modified`),unix_timestamp(`detection`.`modified`),unix_timestamp(`detection`.`modified`),`services`.`id` FROM `MOTDWeb`.`services` LEFT JOIN `MOTDWeb`.`detection` ON `MOTDWeb`.`detection`.`id` = `MOTDWeb`.`services`.`id` WHERE `services`.`active` = 1 ORDER BY `MOTDWeb`.`services`.`modified` DESC";
	$result = gs_query($query);
	$sendstr = "";
	//$service = new Service(100);
	while($row = mysqli_fetch_row($result)) {
//		$service = new Service(intval($row[0]));
		//if(!$service->found()) continue; //should be impossible...
		if(isset($_GET['since'])) {
			$timeval = intval($_GET['since']);
			if(intval($row[1]) < $timeval) continue;
			if(intval($row[2]) < $timeval) continue;
			if(intval($row[3]) < $timeval) continue;
		}
		if(!productVisibleForService(intval($row[4]),$GLOBALS['product'])) continue;
		//detection modified time | full modified time | files modified time
		$dmt = intval($row[1]);
		$fmt = intval($row[2]);
		$fimt = intval($row[3]);
		$sendstr .= $row[0]." - ".$dmt." ".$fmt." ".$fimt."\n";
	}
	header('Content-Type: text/plain; charset=Windows-1252');
	header('Content-Length: '.strlen($sendstr));
	echo $sendstr;
}
function sendFiles($services) {
	foreach($services as $servicename) {
		$service = new Service($servicename);
		if(!$service->found()) continue;
		$service->loadFiles();
		$sendstr .= "[".$service->getName()."]\n";
		$sendstr .= "fmt=".$service->getLastModifiedFileTime()."\n";
		$sendstr .= "fsvid=".$service->getfsvid()."\n";
		$files = $service->getFiles();
		foreach($files as $file) {
			$sendstr.=$file->getName()."=".$file->getNum(1)."\\".$file->getNum(2)."\\".$file->getMD5Hash()."\n";
		}
	}
	header('Content-Type: text/plain; charset=Windows-1252');
	header('Content-Length: '.count($sendstr));
	echo $sendstr;
}
?>
