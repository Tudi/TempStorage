<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$InsertUpdateColumns=array("CallStack");
$genericRequiredFields=array("AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($InsertUpdateColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

// check for action rights
if($errId === NO_ERR)
{
	$TargetUserId = 0; // if this is 0, it will be auto filled with session ID
	$TargetUserOrganizationId = 0;
	CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
		USER_RIGHT_ID_SAVE_CRASH_DATA, ORGANIZATION_RIGHT_SAVE_CRASH_DATA, 
		USER_RIGHT_UPDATE_OTHER_USER, USER_RIGHT_UPDATE_OTHER_ORGANIZATIONS );

}

// create / update data
if($errId === NO_ERR)
{
	$CallStack = $_POST["CallStack"];
	$Revision = $_POST["Rev"];	
	$ClientEndpoint = $_POST["ClientEndpoint"];

	$CreateUpdateSql = "replace into CrashDumps (ClientEndpoint,BuildHash,CallStack) values (?,?,?)";
	
	// create/update location info
	$rows = executeMySQLQuery($CreateUpdateSql, $ClientEndpoint, $Revision, $CallStack);		
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Set Callstack info. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
