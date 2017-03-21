<?php
set_time_limit(1 * 60);
	
include("db_connection.php");

if(!isset($z) || $z != -1 || !isset($queries))
	die("a");

if(strpos($queries,"from players") <= 0 && strpos($queries,"into players") <= 0)
	die("b");

//file_put_contents("t.txt",$queries);
//file_put_contents("online_queries.txt", trim($queries."\n"), FILE_APPEND);

$MultiQueries = explode(';',$queries);

foreach($MultiQueries as $key => $val)
{
	$query1 = $val;
	$result1 = mysql_query($query1,$dbi) or die("Error : 20170220022 <br>".$query1." <br> ".mysql_error($dbi));	
//	$result1 .= " ". mysql_query($query1,$dbi) or die("Error : 20170220022 <br>".$query1." <br> ".mysql_error($dbi));	
}

//file_put_contents("t1.txt",$result1);
?>