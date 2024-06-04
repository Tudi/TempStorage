<?php
require_once("config.php");
require_once("SharedDefines.php");
require_once("CustomFunctions.php");
require_once("mysq_con.php");

// force API delay to see how UI behaves ( no deadlocks )
if(isset($DebugForcedLatency) && $DebugForcedLatency > 0 )
{
	Sleep($DebugForcedLatency);
}

$err = "";
$errId = NO_ERR;
$ret = [];
?>