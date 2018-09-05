<?
include ('../../../phpincludes/config.php');
include ('../../../phpincludes/db.php');
include ('../../../phpincludes/Service.class.php');
if(!isset($_GET['gamename']) || !isset($_GET['filename']) || !isset($_GET['fsvid'])) {
	die('Missing required parameter!');
}
$gamename = mysqli_real_escape_string($GLOBALS['confDBLink'],$_GET['gamename']);
$filename =  mysqli_real_escape_string($GLOBALS['confDBLink'],$_GET['filename']);
$fsvid = intval($_GET['fsvid']);

$service = new Service($gamename);
$file = NULL;
if($service->found()) {
	$service->loadFiles();
	$file = $service->getFile($filename,$fsvid);
}
if(!$service->found() || $file == NULL) {
	header("Status: 404 Not Found");
	die('File not found');
}
$finfo = new finfo(FILEINFO_MIME);
//header("content-disposition: blah; filename=".$_GET['filename']."");
header("Content-Type: ".$finfo->buffer($file->getData()).";");
header("Content-length: ".$file->getLength());
echo $file->getData();
?>
