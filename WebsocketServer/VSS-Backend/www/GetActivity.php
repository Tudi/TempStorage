<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

force_gzip_page_content();

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("UserId");
$genericRequiredFields=array("AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

// check for action rights
if($errId === NO_ERR)
{
	$TargetUserId = (int)$_POST["UserId"]; // if this is 0, it will be auto filled with session ID
	$TargetUserOrganizationId = 0; // if this is 0, it will be auto filled with organization ID from DB
	CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
		USER_RIGHT_ID_VIEW_ACTIVITY_LOG, ORGANIZATION_RIGHT_VIEW_ACTIVITY, 
		USER_RIGHT_VIEW_OTHER_USER, USER_RIGHT_VIEW_OTHER_ORGANIZATIONS );
}

// fetch all activity info
if($errId === NO_ERR)
{
	$limitQuery = "";
	$targetUserId = $session_UserId;
	if((int)$_POST["UserId"] > 0) // later might need to support all organization logs
	{
		$targetUserId = (int)$_POST["UserId"];
	}
	if(isset($_POST["RowLimit"]) && (int)$_POST["RowLimit"] > 0)
	{
		$limitQuery = " limit 0,".(int)$_POST["RowLimit"];
	}

	$rows = executeMySQLQuery("SELECT UNIX_TIMESTAMP(LogClientStamp) as LogClientStamp, 
		LogDetails, UserId from Logs WHERE UserId=$targetUserId and LogClientStamp>0 order by LogClientStamp desc $limitQuery");
}

// set returned data
if($errId === NO_ERR)
{
	$ret['Activity'] = $rows;
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Fetched Activity Logs. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
