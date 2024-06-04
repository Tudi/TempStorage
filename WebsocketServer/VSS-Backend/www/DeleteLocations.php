<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

force_gzip_page_content();

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("TargetId");
$genericRequiredFields=array("AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

// check for action rights
if($errId === NO_ERR)
{
	$TargetUserId = 0; // if this is 0, it will be auto filled with session ID
	$TargetUserOrganizationId = 0; // if this is 0, it will be auto filled with organization ID from DB
	CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
		USER_RIGHT_ID_DELETE_LOCATION, ORGANIZATION_RIGHT_DELETE_LOCATIONS, 
		USER_RIGHT_VIEW_OTHER_USER, USER_RIGHT_VIEW_OTHER_ORGANIZATIONS );
}

// fetch requested data
if($errId === NO_ERR)
{
	$LocationId = (int)$_POST["TargetId"];
	$rowsLocation = executeMySQLQuery("update GeographicLocations set IsDeleted=1 where LocationID=$LocationId");		
}

// set returned data
if($errId === NO_ERR)
{
	$ret['ActionError'] = 0;
}
else
{
	$ret['ActionError'] = 1;
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Query locations. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
