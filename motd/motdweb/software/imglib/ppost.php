<?php

include('class.upload.php');
//ppost.asp?pid=10001&uid=10001&ppass=insert_password&picnum=01000 HTTP/1.1

$pid = $_REQUEST['pid'];
$uid = $_REQUEST['uid'];
$password = $_REQUEST['ppass'];
$picture = $_REQUEST['picnum'];

/*
10001 = /0/10/1/
492845 = /0/492/845/
85675663 = /85/675/663
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
?>  
