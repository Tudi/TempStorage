<?php
$AWS_DB_PG_URL = "";
$AWS_DB_PG_USER = "";
$AWS_DB_PG_PASSW = "";
$AWS_DB_PG_DB = "";
$AWS_DB_TABLE_REGISTERED = "";
$AWS_DB_TABLE_ACCESS_TOKENS = "";
$AWS_DB_TABLE_TXN_QUEUE = "";

$HASH_FUNC_VER = "C_impl_v1";
$HASH_MAX_FILE_SIZE = 10 * 1024 * 1024;
$HASH_MAX_RESOLUTION = 4024*4024; // Anything to avoid crazy values
$HASH_DB_BITCOUNT = 64; // this is base check value, additional filtering needs additional settings
$HASH_LOW_LIMIT_IMAGE_MATCH = 52; // value is in bit count
$HASH_COUNT_RETURN_MATCHING_IMAGES = 5; // There is a chance that hash will consider very different images to be same. These need to be investigated by a human
							// CVP will include an additional check to confirm the images indeed match
$HASH_APP_PATH = "./ImageHash/bin/imgHash";

$WATERMARK_APP_PATH = "./WaterMark/bin/WaterMark";
$REV3AL_ID_SEPARATOR_TOKEN = "#";
							
$EBFS_PATH = "./data/";

$IPFS_BASE_URL = 'https://gateway.pinata.cloud/ipfs/';
$PINATA_API_KEY = '';
$PINATA_API_SECRET_KEY = '';

$BLOCKCHAINTOKENBURNCMD = "node ./BlockChain/EtherBurn.js";

$ShowDebugMessages = 1; // set to 1 to show error messages

// report errors if we are debugging the API
if(isset($_REQUEST['ShowDebugMessages']) && $_REQUEST['ShowDebugMessages']==1)
{
	$ShowDebugMessages = 1;
}

if($ShowDebugMessages)
{
	ini_set('display_errors', 1); 
	ini_set('display_startup_errors', 1); 
	error_reporting(E_ALL);
}
header("Access-Control-Allow-Origin: *");
?>