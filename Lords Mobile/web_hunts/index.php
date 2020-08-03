<?php
require_once("db_connection.php");

echo "Jump to stats :";
echo "<a href=\"#yesterday\">Yesterday&nbsp;&nbsp;&nbsp;</a>";
echo "<a href=\"#pastweek\">Past week&nbsp;&nbsp;&nbsp;</a>";
echo "<a href=\"#pastmonth\">Past month&nbsp;&nbsp;&nbsp;</a>";
echo "<br>";

//echo "Webserver time now : ".GetCompensatedDate();

$timezone	= new DateTimeZone( 'GMT' );
$today      = new DateTime('now', $timezone);
$tomorrow   = new DateTime('tomorrow', $timezone);
$difference = $today->diff($tomorrow);
$TimeLeft = $difference->format('<br>Remaning time until tomorrow : <u><b>%h hours %i minutes </b></u>');
echo "$TimeLeft<br>";

echo "Hunting score for today:<br>";
$start=0;
$end=0;
include("ShowData.php");

echo "<br><br>";
echo "<a id=\"yesterday\">Hunting score for yesterday:</a><br>";
$start=-1;
$end=-1;
include("ShowData.php");

echo "<br><br>";
echo "<a id=\"pastweek\">Hunting score for past 1 week:</a><br>";
$start=-6;
$end=0;
include("ShowData.php");

echo "<br><br>";
echo "<a id=\"pastmonth\">Hunting score for past 1 month:</a><br>";
$start=-31;
$end=0;
include("ShowData.php");

$changelog = file_get_contents("changelog");
$changelog = str_replace("\n","\n<br>", $changelog);
echo "<br>$changelog";
?>