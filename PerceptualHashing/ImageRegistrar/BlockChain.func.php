<?php

function BurnBlockChainTokenNow()
{
	global $BLOCKCHAINTOKENBURNCMD;

	$ret['TXN'] = "";
//	$ret['TimeStamp'] = date('m/d/Y h:i:s a', time());
	$ret['TimeStamp'] = time();
	$ret['ErrorReason'] = "";

	$cmd = $BLOCKCHAINTOKENBURNCMD;
	DebugPring("Executing : ".$cmd);
	$ExecRet = shell_exec($cmd);
	DebugPring("Exec result : ".$ExecRet);
	if(strpos($ExecRet,"error") == false )
	{
		$values = json_decode($ExecRet, true);
		if(isset($values["hash"]))
		{
			$ret['TXN'] = $values["hash"];
			DebugPring("There were no errors in token burn. Hash : ".$ret['TXN']);
		}
		else
		{
			$ret['ErrorReason'] = "Failed to parse JSON from BC burn";
			DebugPring("There were no errors in token burn. But hash was missing !");
		}
	}
	if($ret['TXN'] == "")
	{
		try{
			$ExecRet2 = str_replace(" ", '', $ExecRet);
			$parts = explode("hash:", $ExecRet2 );
			$parts = explode("'", $parts[1] );
			$ret['TXN'] = $parts[1];
			$ret['ErrorReason'] = "";
			DebugPring("There were errors in token burn. Recovered Hash : ".$ret['TXN']);
		}
		catch(Exception $e)
		{
			DebugPring("Exception ocured : ".$e->getMessage());
			$ret['ErrorReason'] = "Failed to extract transaction from Token Burn";
		}
	}
	
	return $ret;
}

function BurnBlockChainTokenQueued()
{
	$cmd = 'php ./backgroundTXNQueue.php >/dev/null 2>/dev/null &';
	$ret = shell_exec($cmd);
	DebugPring("Executed ".$cmd." <br>\n returned :".$ret);
}

function BurnBlockChainToken()
{
	$ret['TXN'] = "";
	$ret['TimeStamp'] = 0;
	$ret['ErrorReason'] = "";
	
	// are there queued up tokens left ?
	$queuedTXN = tryGetQueuedTXN();
	if($queuedTXN['TXN'] == "")
	{
		// burn a token right now
		$ret = BurnBlockChainTokenNow();
	}
	else
	{
		DebugPring("Using a queued up TXN for BC burn");
		$ret['TXN'] = $queuedTXN['TXN'];
		$ret['TimeStamp'] = $queuedTXN['TimeStamp'];
	}
	
	// on next call, have a TXN in the queue
	BurnBlockChainTokenQueued();
	
	return $ret;
}

?>