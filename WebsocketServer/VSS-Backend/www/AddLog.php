<?php
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("msg","stamp","LogVer","source");
$genericRequiredFields=array("AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

if($errId === NO_ERR)
{
	$sourceGroup = $_POST["source"];
	if($_POST["LogVer"] == "1.0.0.1")
	{
		$parts = explode(":", $_POST["msg"]);
		$source = $parts[4];
		$severity = $parts[3];
		AddGenericDBLog($_POST["stamp"], $sourceGroup, $source, $severity, $_POST["msg"] );
	}
	else
	{
		AddGenericDBLog($_POST["stamp"], $sourceGroup, 0, 0, $_POST["msg"] );
	}
	$ret[] = array("res","1");
}
else
{
	LogAPIUsage(LOG_SOURCE_LOG, LOG_SEVERITY_NORMAL, "User tried to add log, but it's considered logged out :".$_POST["msg"] );
}	

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
