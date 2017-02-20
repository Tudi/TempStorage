<?php
foreach($_REQUEST as $foreachname=>$foreachvalue)
	$$foreachname = $foreachvalue;
	
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
?>