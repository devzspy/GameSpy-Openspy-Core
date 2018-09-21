<?php 
require_once("config.php");

$email = $_POST['email'];
$password = $_POST['password'];
$nickname = $_POST['nickname'];
$fname = $_POST['fname'];
$lname = $_POST['lname'];

mysqli_select_db($connection,'GameTracker');

//Insert into the users table
$user_query = "INSERT INTO users(email,password,subscription,emailverified,publicmask) VALUES('".$email."','".$password."',2,1,63)";
if (mysqli_query($connection, $user_query)) {
        $last_id = mysqli_insert_id($connection);
} else {
        die("Error: " . $user_query . "" . mysqli_error($connection));
}

//Insert into the profile table
$profile_query = "INSERT INTO profiles(profileid,userid,nick,uniquenick,firstname,lastname,deleted,pic) VALUES(".$last_id.",".$last_id.",'".$nickname."','".$nickname."','".$fname."','".$lname."',0,729030)";
if (!mysqli_query($connection, $profile_query)) {
        die("Error: " . $profile_query . "" . mysqli_error($connection));
}

mysqli_close($connection);
?>

<html>
<head>
<title>OpenSpy Registration Page</title>
<link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css" integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO" crossorigin="anonymous">
<script src="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js" integrity="sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy" crossorigin="anonymous"></script>
<link rel="stylesheet" href="styles.css">
</head>

<body>
<div id="container">
<div id="top"><img id="gslogo" src="Gamespy_arcade_logo.png"></div>
<div id="middle"><h1 id="welcome">OpenSpy Account Registration</h1>
<p>You have successfully registered. Please load up GameSpy Arcade and login using your email/password provided.</p>
<p>If you have not already done so, please make changes to your hosts file located in C:\Windows\System32\drivers\etc\hosts</p>
</div>
<div id="bottom"></div>
</div>
</body>
</html>