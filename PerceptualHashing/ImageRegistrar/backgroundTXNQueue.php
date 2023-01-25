<?php
require 'settings.php';
require 'DebugMsg.func.php';
require 'BlockChain.func.php';
require 'DB_pgsql.func.php';

initGlobalDBConnection();

$ret = BurnBlockChainTokenNow();
if($ret['TXN'] != "")
{
	AddQueuedTXN($ret['TXN']);
}
defaultRegisterErrorHandler($ret['TXN'] == "", "QueueTXN", "Expected 'TXN' generated is empty." );

closeGlobalDBConnection();

?>