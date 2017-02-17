<?php
/*
//force all post / get / request variables to become global
foreach($_REQUEST as $foreachname=>$foreachvalue)
	$$foreachname = $foreachvalue;
foreach($_SESSION as $foreachname=>$foreachvalue)
	$$foreachname = $foreachvalue;
*/	
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

$f = fopen("Players.txt","rt");
if(!$f)
	exit("Could not open file");

while (($line = fgets($f)) !== false) 
	if(strlen($line)>5)
	{
		$parts = explode(" \t ",$line);
		foreach($parts as $key => $val)
			echo "$val ";
		echo "<br>";
		//backup old
		//insert new
		$query1 = "replace into players ( k,x,y,name,guild,kills,might)values(";
		$query1 = $query1."'".mysql_real_escape_string($parts[0])."','".mysql_real_escape_string($parts[1])."','".mysql_real_escape_string($parts[2])."','".mysql_real_escape_string($parts[3])."','".mysql_real_escape_string($parts[4])."','".mysql_real_escape_string($parts[5])."','".mysql_real_escape_string($parts[6])."')";
		echo "$query1<br>";
		$result1 = mysql_query($query1,$dbi) or die("201602101600".$query1);
	}
?>