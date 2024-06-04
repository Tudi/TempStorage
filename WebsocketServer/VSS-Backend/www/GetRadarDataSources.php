<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("OrganizationId");
$genericRequiredFields=array("AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

// check for action rights
if($errId === NO_ERR)
{
	$TargetUserId = 0; // if this is 0, it will be auto filled with session ID
	$TargetUserOrganizationId = (int)$_POST["OrganizationId"]; // if this is 0, it will be auto filled with organization ID from DB
	CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
		USER_RIGHT_ID_FETCH_DPS_SERVERS, ORGANIZATION_RIGHT_ID_FETCH_DPS_SERVERS, 
		USER_RIGHT_VIEW_OTHER_USER, USER_RIGHT_VIEW_OTHER_ORGANIZATIONS );
}

// fetch requested data
if($errId === NO_ERR)
{
	$whereQuery = "";

	$Target_OrganizationId = (int)$_POST["OrganizationId"];
	$whereQuery = "WHERE (OM.OrganizationID = $Target_OrganizationId)";
		
	// special case when we want all the modules, even those that do not belong to this target organization
	$rows = executeMySQLQuery("SELECT DPSI.DPSID, DPSI.ConnectionURL, OM.ModuleInstanceID   
		FROM OrganizationModules OM 
		INNER JOIN DPS_Modules DPSR ON DPSR.ModuleInstanceID = OM.ModuleInstanceID 
		INNER JOIN DPS_Instances DPSI ON DPSI.DPSID = DPSR.DPSID 
		$whereQuery");		
}

// set returned data
if($errId === NO_ERR)
{
	$ret['DPSInstances'] = $rows;
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Fetched Organization DPSs. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
