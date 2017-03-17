<?php
set_time_limit(2 * 60 * 60);
	
include("db_connection.php");

if(!isset($z) || $z != -1 || !isset($queries))
	die();

if(strpos($queries,"from players") <= 0 && strpos($queries,"into players") )
	die();

$MultiQueries = explode(';',$queries);

//file_put_contents("t.txt",$queries);

foreach($MultiQueries as $key => $val)
{
	$query1 = $val;
	$result1 = mysql_query($query1,$dbi) or die("Error : 20170220022 <br>".$query1." <br> ".mysql_error($dbi));	
//	$result1 .= " ". mysql_query($query1,$dbi) or die("Error : 20170220022 <br>".$query1." <br> ".mysql_error($dbi));	
}

//file_put_contents("t1.txt",$result1);
?>