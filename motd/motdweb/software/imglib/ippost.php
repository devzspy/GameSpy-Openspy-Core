<?php

include('class.upload.php');
//ippost.asp?pid=10001&uid=10001&ppass=insert_password&picnum=01000 HTTP/1.1

$pid = $_POST['pid'];
$uid = $_POST['uid'];
$password = $_POST['ppass'];
$picture = $_POST['picnum'];
$folder = "00000";

if(isset($_FILES['image'])){
  $errors= array();
  $file_name = $_FILES['image']['name'];
  $file_size = $_FILES['image']['size'];
  $file_tmp = $_FILES['image']['tmp_name'];
  $file_type = $_FILES['image']['type'];
  $file_ext=strtolower(end(explode('.',$_FILES['image']['name'])));
  
  $expensions= array("jpeg","jpg","png", "gif","tga","bmp","tif","pcx");
  
  if(in_array($file_ext,$expensions)=== false){
     $errors[]="extension not allowed, please choose a JPEG, PNG, GIF, TGA, BMP, TIF, or PCX file.";
  }
  
  if($file_size > 2097152) {
     $errors[]='File size must be exactly 2 MB';
  }

  if (!is_dir("/var/www/software/imglib/portraits/$folder")) {
    mkdir("/var/www/software/imglib/portraits/$folder", 0777, true);
}
  
  if(empty($errors)==true) {
     move_uploaded_file($file_tmp,"/var/www/software/imglib/icons/$folder/".$file_name);
     echo "Success";
  }else{
     print_r($errors);
  }
}

$image = "./portraits/$folder/$file_name";

$handle = new upload($image);
if ($handle->uploaded) {
  $handle->file_new_name_body   = $picture;
  $handle->image_resize         = true;
  $handle->image_x              = 20;
  $handle->image_ratio_y        = 16;
  $handle->process('./portraits/'.$folder.'/');
  if ($handle->processed) {
    echo 'image resized';
    $handle->clean();
  } else {
    echo 'error : ' . $handle->error;
  }
}
?>  