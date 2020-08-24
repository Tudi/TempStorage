<?php
require_once("db_connection.php");

echo "Jump to stats : &nbsp;&nbsp;&nbsp;";
echo "<a href=\"#yesterday\">Yesterday</a>&nbsp;&nbsp;&nbsp; / &nbsp;&nbsp;&nbsp;";
echo "<a href=\"#pastweek\">Past week</a>&nbsp;&nbsp;&nbsp; / &nbsp;&nbsp;&nbsp;";
echo "<a href=\"#pastmonth\">Past month</a>";
echo "<br>";

include("HuntStatistics.html");

//echo "Webserver time now : ".GetCompensatedDate();
$MinutesPassedToday = intval(date('H', GetCompensatedTime()))*60 + intval(date('i', GetCompensatedTime()));
$MinutesRemain = 24*60 - $MinutesPassedToday;
$differenceHours = (int)($MinutesRemain / 60);
$differenceMinutes = (int)($MinutesRemain % 60);
//echo "Server time : ".date("Y-m-d H:i", GetCompensatedTime())."<br>";
echo "Remaning time until tomorrow : <u><b>${differenceHours}:${differenceMinutes}</b></u><br>";

echo "Today = ";
$start=0;
$end=0;
include("ShowData.php");

echo "<br><br>";
echo "<a id=\"yesterday\">Yesterday = </a>";
$start=-1;
$end=-1;
include("ShowData.php");/**/

echo "<br><br>";
echo "<a id=\"pastweek\">Past 1 week (not counting today)= </a>";
$start=-8;
$end=-1;
include("ShowData.php");

echo "<br><br>";
echo "<a id=\"pastmonth\">Past 1 month = </a>";
$start=-31;
$end=0;
include("ShowData.php");

$changelog = file_get_contents("changelog");
$changelog = str_replace("\n","\n<br>", $changelog);
echo "<br>$changelog";
?>