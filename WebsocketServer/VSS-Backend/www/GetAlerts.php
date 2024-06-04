<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

force_gzip_page_content();

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("AlertId","LocationId");
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
		USER_RIGHT_ID_VIEW_ALERTS, ORGANIZATION_RIGHT_VIEW_ALERTS, 
		USER_RIGHT_VIEW_OTHER_USER, USER_RIGHT_VIEW_OTHER_ORGANIZATIONS );
}

// fetch requested data
if($errId === NO_ERR)
{
	$whereQuery = "";
	$limitQuery = "";
	// this is used for polling and should be removed once websockets are implemented
	if(isset($_POST["NewerId"]) && (int)$_POST["NewerId"] > 0)
	{
		$whereQuery = " and A.AlertId>".(int)$_POST["NewerId"];
	}
	if((int)$_POST["AlertId"] > 0)
	{
		$whereQuery = "and A.AlertId=".(int)$_POST["AlertId"];
	}
	if((int)$_POST["LocationId"] > 0)
	{
		$whereQuery = " and A.LocationId=".(int)$_POST["LocationId"];
	}
	if(isset($_POST["RowLimit"]) && (int)$_POST["RowLimit"] > 0)
	{
		$limitQuery = " limit 0,".(int)$_POST["RowLimit"];
	}

	$rows = executeMySQLQuery("SELECT A.AlertId, AD.AlertDefinitionName, GL.LocationName, GL.LocationId,
		UNIX_TIMESTAMP(A.CreatedTimestamp) as CreatedTimestamp, A.AlertStatusTypeId,
		AD.AlertTypeId, AT.AlertTypeName, AT.AlertTypeDescription		
		FROM Alerts A 
		INNER JOIN AlertDefintion AD ON A.AlertDefinitionId = AD.AlertDefinitionId 
		INNER JOIN AlertType AT ON AD.AlertTypeId = AT.AlertTypeId 
		INNER JOIN GeographicLocations GL ON A.LocationId = GL.LocationID 
		WHERE (A.OwnerOrganizationId = $session_OrganizationId AND ( A.OwnerUserId = 0 or A.OwnerUserId = $session_UserId))
		$whereQuery $limitQuery");
}

// set returned data
if($errId === NO_ERR)
{
	$ret['Alerts'] = $rows;
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Fetched alerts. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
