<?php
require 'settings.php';
require 'DebugMsg.func.php';
require 'DB_pgsql.func.php';	
initGlobalDBConnection();

// check if a valid access token was provide
include("session.incl.php");
defaultRegisterErrorHandler(($SESSION_USER_ID == 0), "HasTimes", "Unable to validate APIAccessToken");

$processingTimeJSON = getProcesingTime();

echo $processingTimeJSON;

closeGlobalDBConnection();
?>