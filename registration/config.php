<?php
$confDBServ = "localhost";
$confDBUser = "openspy";
$confDBPass = "P7LjdYy8HKY7CLtu";
$connection = mysqli_connect($confDBServ,$confDBUser,$confDBPass);
if(!$connection) {
	die('Could not connect: '.mysqli_error($connection));
}
?>
