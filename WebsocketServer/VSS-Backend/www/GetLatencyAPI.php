<?php
require_once('NotHosted/header_include.php');

CheckRequiredFields($_POST,array("AppVer", "stamp", "ClientEndpoint"));

if($errId === NO_ERR)
{
	// test the speed of a simple query
	$res = executeMySQLQuery("Select UserId from UserClientEndpoints where ClientEndpoint=?", $_POST['ClientEndpoint']);

	// this user never logged in and asking for latency ? 
	if($res[0]['UserId'] != 0)
	{
		// send back the stamp
		$ret['stamp'] = $_POST['stamp'];
	}
}

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>