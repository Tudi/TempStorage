<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$InsertUpdateColumns=array("ModuleInstanceID","LocationID");
$genericRequiredFields=array("LocationID", "AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($InsertUpdateColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

// check for action rights
if($errId === NO_ERR)
{
	$TargetUserId = 0; // if this is 0, it will be auto filled with session ID
	$TargetUserOrganizationId = 0;
	CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
		USER_RIGHT_ID_UPDATE_MODULES, ORGANIZATION_RIGHT_UPDATE_MODULES, 
		USER_RIGHT_UPDATE_OTHER_USER, USER_RIGHT_UPDATE_OTHER_ORGANIZATIONS );
}

// create / update data
if($errId === NO_ERR)
{
	// create/update location info
	$ModuleInstanceID = (int)$_POST["ModuleInstanceID"];
	$ModuleLocationID = (int)$_POST["LocationID"];
	$rows = executeMySQLQuery("update ModuleInstances set ModuleLocationID=$ModuleLocationID 
		where ModuleInstanceID=$ModuleInstanceID");		
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Updated Module ".$_POST["ModuleID"]." locationId info. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
