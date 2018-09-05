<?
include ('../../../phpincludes/config.php');
include ('../../../phpincludes/db.php');
include ('../../../phpincludes/Service.class.php');
$service = new Service("gsarcadetour");
$key = $service->getKeys();
$blah = "";
foreach($key as $k) {
$blah .= $k."<br>";
}
echo $blah;
?>
