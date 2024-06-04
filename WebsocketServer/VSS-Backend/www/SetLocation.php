<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$InsertUpdateColumns=array("OrganizationID", "LocationName","LocationDescription",
	"LocationAddressLine1","LocationAddressLine2","LocationCity","LocationState",
	"LocationCountyOrRegion","LocationCountry","LocationX","LocationY","LocationZ");
$genericRequiredFields=array("LocationID", "AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($InsertUpdateColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

// check for action rights
if($errId === NO_ERR)
{
	$TargetUserId = 0; // if this is 0, it will be auto filled with session ID
	$TargetUserOrganizationId = (int)$_POST["OrganizationID"];
	if((int)$_POST["LocationID"] == 0)
	{
		CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
			USER_RIGHT_ID_CREATE_LOCATION, ORGANIZATION_RIGHT_CREATE_LOCATIONS, 
			USER_RIGHT_UPDATE_OTHER_USER, USER_RIGHT_UPDATE_OTHER_ORGANIZATIONS );
	}
	else
	{
		CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
			USER_RIGHT_ID_UPDATE_LOCATION, ORGANIZATION_RIGHT_UPDATE_LOCATIONS, 
			USER_RIGHT_UPDATE_OTHER_USER, USER_RIGHT_UPDATE_OTHER_ORGANIZATIONS );
	}
}

// create / update data
if($errId === NO_ERR)
{
	if($_POST["LocationID"] == 0)
	{
		$colsAsString = "";
		$colsAsValues = "";
		$valsArr = array();
		foreach($InsertUpdateColumns as $key => $val)
		{
			$colsAsString .= $val.",";
			$colsAsValues .= "?,";
			$valsArr[] = $_POST[$val];
		}
		$colsAsString = substr($colsAsString,0,-1);
		$colsAsValues = substr($colsAsValues,0,-1);
		$CreateUpdateSql = "insert into GeographicLocations ($colsAsString) values ($colsAsValues)";
	}
	else
	{
		$colsAsString = "";
		$valsArr = array();
		foreach($InsertUpdateColumns as $key => $val)
		{
			$colsAsString .= $val."=?,";
			$valsArr[] = $_POST[$val];
		}
		$colsAsString = substr($colsAsString,0,-1);
		$valsArr[] = $_POST["LocationID"];
		$CreateUpdateSql = "update GeographicLocations set $colsAsString where LocationID=?";
	}
	// create/update location info
	$rows = executeMySQLQuery($CreateUpdateSql, ...$valsArr);		
}

// log API usage 
if((int)$_POST["LocationID"] == 0)
{
	LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Create location ".$_POST["LocationName"]." info. Err = $errId" );
}
else
{
	LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Updated location ".$_POST["LocationName"]." info. Err = $errId" );
}

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
