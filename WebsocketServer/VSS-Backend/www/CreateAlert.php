<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("UserID", "OrganizationID", "ModuleInstanceID");
$genericRequiredFields=array("AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

// check for action rights
if($errId === NO_ERR)
{
	$TargetUserId = (int)$_POST["UserID"]; // if this is 0, it will be auto filled with session ID
	$TargetUserOrganizationId = (int)$_POST["OrganizationID"]; // if this is 0, it will be auto filled with organization ID from DB
	CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
		USER_RIGHT_ID_CREATE_ALERT, ORGANIZATION_RIGHT_CREATE_ALERT, 
		USER_RIGHT_UPDATE_OTHER_USER, USER_RIGHT_UPDATE_OTHER_ORGANIZATIONS );
}

// fetch requested data
if($errId === NO_ERR)
{
	$OrganizationId = (int)$_POST["OrganizationID"];
	$ModuleID = (int)$_POST["ModuleInstanceID"];
	$UserId = (int)$_POST["UserID"];
	$rows = executeMySQLQuery("INSERT INTO Alerts (AlertDefinitionId,AlertStatusTypeId,ModuleId,LocationId,OwnerOrganizationId,OwnerUserId) values (
	1, 1, $ModuleID, 2, $OrganizationId, $UserId)");		
}

// set returned data
if($errId === NO_ERR)
{
	$ret['res'] = $rows[0]['res'];
	$ret['id'] = $rows[0]['id'];
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Created organization alert. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
