<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$InsertUpdateColumns=array("KPIData");
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
		USER_RIGHT_ID_SAVE_KPI_DATA, ORGANIZATION_RIGHT_SAVE_KPI_DATA, 
		USER_RIGHT_UPDATE_OTHER_USER, USER_RIGHT_UPDATE_OTHER_ORGANIZATIONS );

}

// create / update data
if($errId === NO_ERR)
{
	$JSONfields = json_decode($_POST["KPIData"], true);
	
	$JSONfields['ClientEndpoint'] = $_POST["ClientEndpoint"];
	
	// JSON field name as index, DB field name as value
	$saveFields['ClientEndpoint'] = "ClientEndpoint";
	$saveFields['FPS_MIN'] = "FPS_MIN";
	$saveFields['FPS_AVG'] = "FPS_AVG";
	$saveFields['FPS_MAX'] = "FPS_MAX";
	$saveFields['LAG_AVG'] = "LAG_AVG";
	$saveFields['APIDUR_AVG'] = "APIDUR_AVG";
	$saveFields['OnlineTime'] = "OnlineTime";
	
	$colsAsString = "";
	$colsAsValues = "";
	$valsArr = array();
	foreach($saveFields as $key => $val)
	{
		if(!isset($JSONfields[$val]))
		{
			continue;
		}
		$colsAsString .= $val.",";
		$colsAsValues .= "?,";
		$valsArr[] = $JSONfields[$val];
	}
	$colsAsString = substr($colsAsString,0,-1);
	$colsAsValues = substr($colsAsValues,0,-1);
	$CreateUpdateSql = "replace into KPIData ($colsAsString) values ($colsAsValues)";
	
	// create/update location info
	$rows = executeMySQLQuery($CreateUpdateSql, ...$valsArr);		
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Set KPI info. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
