<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

// compress the output of this page. It can get up to 16k
force_gzip_page_content();

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("Lang");
$genericRequiredFields=array("AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

// rights table
if($errId === NO_ERR)
{
	// Get all the user info, we might return only some
	$rows = executeMySQLQuery("Select RightID,RightName,RightDescription from RoleRightDefines");
		
	// check if user exists
	if( !isset($rows[0]['RightID']))
	{
		$err = "Could not find rights table";
		$errId = INVALID_TABLE_NAME;
	}
	// prepare data to be returned to the UI App
	else
	{
		$ret['Rights'] = $rows;
	}
}

// rights table
if($errId === NO_ERR)
{
	// Get all the user info, we might return only some
	$rows = executeMySQLQuery("Select RoleID,RoleName,RoleDescription from UserRoleDefines");
		
	// check if user exists
	if( !isset($rows[0]['RoleID']))
	{
		$err = "Could not find roles table";
		$errId = INVALID_TABLE_NAME;
	}
	// prepare data to be returned to the UI App
	else
	{
		$ret['Roles'] = $rows;
	}
}

// rights table
if($errId === NO_ERR)
{
	// Get all the user info, we might return only some
	$rows = executeMySQLQuery("Select UserActiveID,UserActiveName from UserActiveStates");
		
	// check if user exists
	if( !isset($rows[0]['UserActiveID']))
	{
		$err = "Could not find active states table";
		$errId = INVALID_TABLE_NAME;
	}
	// prepare data to be returned to the UI App
	else
	{
		$ret['ActiveStates'] = $rows;
	}
}

// Alert status types
if($errId === NO_ERR)
{
	// Get all the user info, we might return only some
	$rows = executeMySQLQuery("Select AlertStatusTypeId,AlertStatusTypeName from AlertStatusType");
		
	// check if user exists
	if( !isset($rows[0]['AlertStatusTypeId']))
	{
		$err = "Could not find alert status types table";
		$errId = INVALID_TABLE_NAME;
	}
	// prepare data to be returned to the UI App
	else
	{
		$ret['AlertStatusType'] = $rows;
	}
}

// Alert Types
if($errId === NO_ERR)
{
	// Get all the user info, we might return only some
	$rows = executeMySQLQuery("Select AlertTypeId,AlertTypeName from AlertType");
		
	// check if user exists
	if( !isset($rows[0]['AlertTypeId']))
	{
		$err = "Could not find alert status types table";
		$errId = INVALID_TABLE_NAME;
	}
	// prepare data to be returned to the UI App
	else
	{
		$ret['AlertType'] = $rows;
	}
}

// Module status Types
if($errId === NO_ERR)
{
	// Get all the user info, we might return only some
	$rows = executeMySQLQuery("Select ModuleStatusID,ModuleStatusName from ModuleStatusTypes");
		
	// check if user exists
	if( !isset($rows[0]['ModuleStatusID']))
	{
		$err = "Could not find module status types table";
		$errId = INVALID_TABLE_NAME;
	}
	// prepare data to be returned to the UI App
	else
	{
		$ret['ModuleStatusTypes'] = $rows;
	}
}

// Module Types
if($errId === NO_ERR)
{
	// Get all the user info, we might return only some
	$rows = executeMySQLQuery("Select ModuleTypeID,ModuleTypeName from ModuleTypes");
		
	// check if user exists
	if( !isset($rows[0]['ModuleTypeID']))
	{
		$err = "Could not find module types table";
		$errId = INVALID_TABLE_NAME;
	}
	// prepare data to be returned to the UI App
	else
	{
		$ret['ModuleTypes'] = $rows;
	}
}

// Module Types
if($errId === NO_ERR)
{
	// Get all the user info, we might return only some
	$rows = executeMySQLQuery("Select LocationID, LocationName from GeographicLocations");
		
	// check if user exists
	if( !isset($rows[0]['LocationID']))
	{
		$err = "Could not find GeographicLocations types table";
		$errId = INVALID_TABLE_NAME;
	}
	// prepare data to be returned to the UI App
	else
	{
		$ret['LocationNames'] = $rows;
	}
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Fetched constant strings. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
