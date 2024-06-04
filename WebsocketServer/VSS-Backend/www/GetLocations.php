<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

force_gzip_page_content();

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("TargetId", "ResultFormat");
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
		USER_RIGHT_ID_VIEW_LOCATION, ORGANIZATION_RIGHT_VIEW_LOCATIONS, 
		USER_RIGHT_VIEW_OTHER_USER, USER_RIGHT_VIEW_OTHER_ORGANIZATIONS );
}

// fetch requested data
if($errId === NO_ERR)
{
	$fields = "";
	if((int)$_POST["ResultFormat"] == 1) // used for showing locations list for a specific organization
	{
		$fields = "LocationID, LocationName, LocationCity, LocationState";
	}
	else if((int)$_POST["ResultFormat"] == 2) // used when viewing a location
	{
		$fields = "LocationID, LocationName, LocationDescription, LocationAddressLine1, LocationAddressLine2, 
		LocationCity, LocationState, LocationCountyOrRegion, LocationCountry";
	}	
	else if((int)$_POST["ResultFormat"] == 3) // used for dropdowns
	{
		$fields = "LocationID, LocationName";
	}	
	else // used when editing a location
	{
		$fields = "LocationID, LocationName, LocationDescription, LocationAddressLine1, LocationAddressLine2,
			LocationCity, LocationState, LocationCountyOrRegion, LocationCountry, LocationX,
			LocationY, LocationZ, LocationSize";
	}
	
	$whereQuery = "";
	$orderQuery = "";
	if((int)$_POST["ResultFormat"] == 1)
	{
		$whereQuery = "where LocationID>1 ";
		$orderQuery = "order by LocationID desc";
	}
	if((int)$_POST["TargetId"] > 0)
	{
		if(strlen($whereQuery) > 0)
		{
			$whereQuery .= " and LocationID=".(int)$_POST["TargetId"];
		}
		else
		{
			$whereQuery .= "where LocationID=".(int)$_POST["TargetId"];
		}
	}
	if(strlen($whereQuery) > 0)
	{
		$whereQuery .= " and IsDeleted=0";
	}
	else
	{
		$whereQuery .= "where IsDeleted=0";
	}
	$rowsLocation = executeMySQLQuery("Select $fields from GeographicLocations $whereQuery $orderQuery");		
}

// add module info if this request if for 1 specific location
if($errId === NO_ERR)
{
	if(isset($_POST["TargetId"]) && (int)$_POST["TargetId"] > 0)
	{
		$TargetLocationId = (int)$_POST["TargetId"];
	
		$rowsModules = executeMySQLQuery("SELECT MD.ModuleDefineID, MD.ModuleName 
			FROM OrganizationModules OM 
			INNER JOIN moduleinstances MI ON MI.ModuleInstanceID = OM.ModuleInstanceID 
			INNER JOIN moduledefines MD ON MD.ModuleDefineID = MI.ModuleDefineID 
			WHERE (OM.OrganizationID = $session_OrganizationId and 
			MI.ModuleLocationID = $TargetLocationId and 
			MI.ModuleStatusID=1)");
	}
}

// set returned data
if($errId === NO_ERR)
{
	$ret['Locations'] = $rowsLocation;
	if(isset($rowsModules))
	{
		$ret['LocationModules'] = $rowsModules;
	}
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Query locations. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
