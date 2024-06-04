<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("RowLimit");
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
		USER_RIGHT_ID_CREATE_MODULE, ORGANIZATION_RIGHT_CREATE_MODULES, 
		USER_RIGHT_VIEW_OTHER_USER, USER_RIGHT_VIEW_OTHER_ORGANIZATIONS );
}

// fetch requested data
if($errId === NO_ERR)
{
	$limitQuery = "";
	$whereQuery = "where isDeleted!=1";
	if((int)$_POST["RowLimit"] > 0)
	{
		$limitQuery = " limit 0,".(int)$_POST["RowLimit"];
	}
		
	// special case when we want all the modules, even those that do not belong to this target organization
	$rows = executeMySQLQuery("SELECT MD.ModuleDefineID, MD.ModuleName, MD.ModuleTag, MI.ModuleStatusID, 
		GL.LocationName, GL.LocationID, MD.ModuleDeveloper, MI.ModuleInstanceID   
		FROM ModuleInstances MI 
		INNER JOIN moduledefines MD ON MD.ModuleDefineID = MI.ModuleDefineID 
		INNER JOIN GeographicLocations GL ON MI.ModuleLocationID = GL.LocationID 		
		$whereQuery $limitQuery");		
}

// set returned data
if($errId === NO_ERR)
{
	$ret['ModuleInstances'] = $rows;
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Fetched modules. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
