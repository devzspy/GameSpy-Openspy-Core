<?
$confDBServ = "localhost";
$confDBUser = "openspy";
$confDBPass = "P7LjdYy8HKY7CLtu";
$confDBLink = mysqli_connect($confDBServ,$confDBUser,$confDBPass);
if(!$confDBLink) {
	die('Could not connect: '.mysqli_error($confDBLink));
}
$confDebugMode = 1;
if($confDebugMode == 0) {
	error_reporting(0);
} else {
	error_reporting(E_ALL ^ E_NOTICE);
	ini_set('display_errors', 'On');
}
?>
