<?php
foreach($_REQUEST as $foreachname=>$foreachvalue)
{
	$$foreachname = $foreachvalue;
//	echo $foreachname."=$foreachvalue,";
}
	
$dbhost = "localhost";
$dbname = "LordsMobile";
$dbuname = "root";
$dbupass = "";
$dbtype = "MySQL";

//phpinfo();

//create global DB connection. Should not forget to close it :P
$dbim = mysql_connect($dbhost, $dbuname, $dbupass,true) or die("Couldn't connect to database server!");
$ret = mysql_select_db($dbname, $dbim) or die("Q 201602091125");
$dbi = $dbim;

if (!empty($_SERVER['HTTP_CLIENT_IP'])) {
    $ip = $_SERVER['HTTP_CLIENT_IP'];
} elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR'])) {
    $ip = $_SERVER['HTTP_X_FORWARDED_FOR'];
} else {
    $ip = $_SERVER['REMOTE_ADDR'];
}

$query1 = "insert into access_logs (script,URI,IP,stamp)values('".mysql_real_escape_string($_SERVER['SCRIPT_FILENAME'])."','".mysql_real_escape_string($_SERVER['REQUEST_URI'])."','$ip','".mysql_real_escape_string($_SERVER['REQUEST_TIME'])."')";		
$result1 = mysql_query($query1,$dbi) or die("Error : 20170220041630 <br>".$query1." <br> ".mysql_error($dbi));

include("functions.php");
?>