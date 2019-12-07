<?php

include('class.upload.php');
//ppost.asp?pid=10001&uid=10001&ppass=insert_password&picnum=01000 HTTP/1.1

$pid = $_REQUEST['pid'];
$uid = $_REQUEST['uid'];
$password = $_REQUEST['ppass'];
$picture = $_REQUEST['picnum'];

/*
pid: 312 (3)                    #   Octets: 312             #   folder: /0/0/312/
pid: 1002 (4)                   #   Octets: 1|002           #   folder: /0/1/2/
pid: 10001 (5)                  #   Octets: 10|001          #   folder: /0/10/1/
pid: 10001002 (8)               #   Octets: 10|001|002      #   folder: /10/1/2/
pid: 492845 (6)                 #   Octets: 492|845         #   folder: /0/492/845/
pid: 85675663 (8)               #   Octets: 85|675|663      #   folder: /85/675/663/
pid: 2147483647 (10)            #   Octets: 2|147|483|647   #   folder: /2147/483/647/

Observations from above:
  * if a pid contains leading 0s in an octet of "3" the 0s are truncated
      - pid of 10001 has a folder of /0/10/1/
          + contains 2 leading 0s in "2nd" octet (001)
      - pid of 10001002 has a folder of /10/1/2/.
          + contains 2 leading 0s in the "2nd" (001) and "3rd" octet (002)
  * A pid folder must contain 3 levels
      - if a pid is not >= 7 long, pre-pends /0/ to front.
*/

if(strlen($pid) <= 3) {
   $temp = str_split($pid);
   $folder = join("/", $temp);
}
else if(strlen($pid) <= 4) {
  $temp = str_split($pid);
  $folder = join("", array_slice($temp,0,1))."/".join("",array_slice($temp,1,3));
}
else if(strlen($pid) <= 5) {
  $temp = str_split($pid);
  $folder = join("", array_slice($temp,0,2))."/".join("",array_slice($temp,2,3));
}
else if(strlen($pid) <= 6) {
  $temp = str_split($pid);
  $folder = join("", array_slice($temp,0,3))."/".join("",array_slice($temp,3,3));
}
else if(strlen($pid) <= 7) {
  $temp = str_split($pid);
  $folder = join("", array_slice($temp,0,1))."/".join("",array_slice($temp,1,3))."/".join("",array_slice($temp,4,3));
}
else if(strlen($pid) <= 8) {
  $temp = str_split($pid);
  $folder = join("", array_slice($temp,0,2))."/".join("",array_slice($temp,2,3))."/".join("",array_slice($temp,5,3));
}
else if(strlen($pid) <= 9) {
  $temp = str_split($pid);
  $folder = join("", array_slice($temp,0,3))."/".join("",array_slice($temp,3,3))."/".join("",array_slice($temp,6,3));
}
else if(strlen($pid) <= 10) {
  $temp = str_split($pid);
  $folder = join("", array_slice($temp,0,1))."/".join("", array_slice($temp,1,3))."/".join("",array_slice($temp,4,3))."/".join("",array_slice($temp,7,3));
}

if(isset($_FILES['FILE1'])){
  $errors= array();
  $file_name = $_FILES['FILE1']['name'];
  $file_size = $_FILES['FILE1']['size'];
  $file_tmp = $_FILES['FILE1']['tmp_name'];
  $file_type = $_FILES['FILE1']['type'];
  $fname = strtolower(reset(explode('.',$_FILES['FILE1']['name'])));
  $file_ext=strtolower(end(explode('.',$_FILES['FILE1']['name'])));
  $full_new_file = $fname.".jpg";
  
  $expensions= array("jpeg","jpg","png", "gif","tga","bmp","tif","pcx");
  
  if(in_array($file_ext,$expensions)=== false){
     $errors[]="extension not allowed, please choose a JPEG, PNG, GIF, TGA, BMP, TIF, or PCX file.";
  }
  
  if($file_size > 2097152) {
     $errors[]='File size must be exactly 2 MB';
  }

  if (!is_dir("/var/www/software/imglib/portraits/user/$folder")) {
    mkdir("/var/www/software/imglib/portraits/user/$folder", 0777, true);
}
  
  if(empty($errors)==true) {
     move_uploaded_file($file_tmp,"/var/www/software/imglib/portraits/user/$folder/".$full_new_file);
     print "success";
  }#else{
  #   print_r($errors);
  #}
}

$image = "./portraits/user/$folder/$full_new_file";

$handle = new upload($image);
if ($handle->uploaded) {
  $handle->file_new_name_body   = $picture;
  $handle->image_resize         = true;
  $handle->image_x              = 96;
  $handle->image_ratio_y        = 96;
  $handle->process('./portraits/user/'.$folder.'/');
  if ($handle->processed) {
    #echo 'image resized';
    $handle->clean();
  } else {
    echo 'error : ' . $handle->error;
  }
}

try 
{
  $database = "GameTracker";
  $db_user = "openspy";
  $db_pass = "P7LjdYy8HKY7CLtu";
  $host = "localhost";
  $connection = new PDO("mysql:host=$host; dbname=$database", $db_user, $db_pass);

  $sql = "SELECT userid FROM users where userid = ? and password = ?";
  $query = $connection->prepare($sql);
  $query->execute([$uid, $password]);
  if($query->fetch())
  {
    $sql = "SELECT pic FROM profiles where pid = ?";
    $query = $connection->prepare($sql);
    $query->execute([$pid]);
    $temp = $query->fetch();
    $temp = $temp + 1;
    $sql = "UPDATE profiles SET 'pic' = ? WHERE profileid = ?";
    $update = $connection->prepare($sql);
    $update->execute([$temp, $pid]);
  }
  $connection = null;
} 
catch (PDOException $e)
{
  echo "Error: " . $e->getMessage() . "<br/>";
  die();
}
?>  
