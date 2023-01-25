<?php
require 'settings.php';
require 'DebugMsg.func.php';

// monitor strange behavior. If processing time is strangely large, investigate what is special about the image
$startStamp = Time_ms();

defaultRegisterErrorHandler((!isset($_REQUEST['ArtistFullName']) || strlen($_REQUEST['ArtistFullName'])==0), 
	"Registered", "Expected 'ArtistFullName' param is missing value." );

defaultRegisterErrorHandler((!isset($_REQUEST['ArtTitle']) || strlen($_REQUEST['ArtTitle'])==0), 
	"Registered", "Expected 'ArtTitle' param is missing value." );

// !! TODO : should check if wallet is a valid valet address. Javascript ?
defaultRegisterErrorHandler((!isset($_REQUEST['UserWalletAddress']) || strlen($_REQUEST['UserWalletAddress'])==0), 
	"Registered", "Expected 'UserWalletAddress' param is missing value." );

defaultRegisterErrorHandler((!isset($_REQUEST['UserCopyrightInfo']) || strlen($_REQUEST['UserCopyrightInfo'])==0), 
	"Registered", "Expected 'UserCopyrightInfo' param is missing value.");

if(!isset($_REQUEST['ArtistAlias']) || strlen($_REQUEST['ArtistAlias'])==0)
{
	$_REQUEST['ArtistAlias'] = ""; // no longer "required"
}

if(!isset($_REQUEST['ReplyWithImage']) || strlen($_REQUEST['ReplyWithImage'])==0)
{
	$_REQUEST['ReplyWithImage'] = 0; // optional
}

require 'UploadImg.func.php';
require 'ec2hash.func.php';
require 'WaterMark.func.php';
require 'crypto.func.php';
require 'BlockChain.func.php';
require 'DB_pgsql.func.php';
initGlobalDBConnection();

// check if a valid access token was provided. Check rate limitations...
include("session.incl.php");
defaultRegisterErrorHandler(($SESSION_USER_ID == 0), "Registered", "Unable to validate APIAccessToken");

// check if this file is already registered
$startSingleProcess = Time_ms();
$hashes = getImageHashes();
defaultRegisterErrorHandler(($hashes == "" || !isset($hashes["PHASH"])), "Registered", "Could not hash Image file." );
DebugPring("Hashing took ".(Time_ms() - $startSingleProcess)." ms");

$startSingleProcess = Time_ms();
$registerdJSON = checkRegistered($hashes["AHASH"], $hashes["DHASH"], $hashes["PHASH"], $HASH_LOW_LIMIT_IMAGE_MATCH, 1);
defaultRegisterErrorHandler((strpos($registerdJSON,"\"IsRegistered\":\"False\"") == false), 
	"Registered", "Already registered", "IsRegistered check returned :".$registerdJSON);
DebugPring("DB registry check took ".(Time_ms() - $startSingleProcess)." ms");

$PersistedFiles = check_Save_Process_PostedFile($hashes["AHASH"], $hashes["DHASH"], $hashes["PHASH"], $hashes["MHASH"]);
defaultRegisterErrorHandler(($PersistedFiles['ErrorReason'] != ""), "Registered", $PersistedFiles['ErrorReason'], "Failed to save the file.");

$res = "{ \"Registered\":\"True\",\"Rev3alId\":\"".$PersistedFiles['Rev3alID']."\",\"URLWatermark\":\"".$PersistedFiles['WatermarkedURL']."\"";
// got removed at the end of MVP. I sense this will be readded at some point
if(isset($PersistedFiles['URL']))
{
	$res .= ",\"URL\":\"".$PersistedFiles['URL']."\"";
}
// only present if requested
if($_REQUEST['ReplyWithImage'] && isset($PersistedFiles['WatermarkedFile']))
{
	$res .= ",\"WaterMarkedImage\":\"".$PersistedFiles['WatermarkedFile']."\"";
}

$res .= "}";

echo $res;

closeGlobalDBConnection();

function check_Save_Process_PostedFile($AHash, $DHash, $PHash, $MHash)
{
	global $_REQUEST, $HASH_USE_LAMBDA_HASHING, $startStamp;
	
	$ret['ErrorReason']="";
	// make sure POST params are OK
	$TempFileNameFullPath = getUploadedFileName();
	if(!isset($TempFileNameFullPath) || $TempFileNameFullPath == "")
	{
		DebugPring("Failed to obtain uploaded file path. Can't register");
		$ret['ErrorReason']="Failed to obtain uploaded file path. Can't register";
		return $ret;
	}
	
	$RealFileName = getUploadedRealFileName();
	if(!isset($RealFileName) || $RealFileName == "")
	{
		DebugPring("Failed to obtain real file name. Can't register");
		$ret['ErrorReason']="Failed to obtain real file name. Can't register";
		return $ret;
	}
	
	// burn Near token and get TXN and Timestamp
	$startSingleProcess = Time_ms();
	$bc_ret = BurnBlockChainToken();
	$Rev3alTXN = $bc_ret['TXN'];
	$Rev3alTimeStamp = $bc_ret['TimeStamp'];
	if($bc_ret['ErrorReason'] != "" && $bc_ret['TXN'] == "")
	{
		DebugPring("Failed to burn blockchain token");
		$ret['ErrorReason']="Failed to burn blockchain token";
		return $ret;
	}
	DebugPring("Burning blockchain token took ".(Time_ms() - $startSingleProcess)." ms");
	
	// generate Rev3al_ID to be burned as watermark
	$startSingleProcess = Time_ms();
	$Rev3al_ID_details = generateRev3alID($_REQUEST['UserCopyrightInfo'], $PHash, $MHash, $Rev3alTXN, $Rev3alTimeStamp);
	$ret['Rev3alID'] = $Rev3al_ID_details['id_enc'];
	DebugPring("Rev3al id generation took ".(Time_ms() - $startSingleProcess)." ms");
	
	// burn watermark and upload image to IPFS
	$startSingleProcess = Time_ms();
	$watermark_details = watermarkAndUploadImageToIPFS($TempFileNameFullPath, $Rev3al_ID_details['id'], $Rev3al_ID_details['enc_1'], $_REQUEST['ReplyWithImage']);
	if($watermark_details == "")
	{
		DebugPring("Failed to watermark image");
		$ret['ErrorReason']="Failed to watermark image";
		return $ret;
	}
	$ret['WatermarkedURL'] = $watermark_details['URL'];
	$ret['WatermarkedFile'] = $watermark_details['fileContent'];
	DebugPring("WaterMark generation + upload watermarked image to IPFS took ".(Time_ms() - $startSingleProcess)." ms");
	
	$endStamp = Time_ms();
	$processindDuration = $endStamp - $startStamp;

	// !!!!! TODO !!!!! This needs to be split into 2 parts : Add hashes as soon as possible. Update row after additional data 
	// Create the Rev3al registry and get the unique ID for this file
	$startSingleProcess = Time_ms();
	$dbRowId = createDBRowForRegister($RealFileName, $_REQUEST['ArtistFullName'], $_REQUEST['ArtistAlias'], $_REQUEST['ArtTitle'], 
		$AHash, $DHash, $PHash, $MHash, $watermark_details['IpfsHash'], $watermark_details['Timestamp'], $Rev3al_ID_details['id_enc'], $Rev3al_ID_details['enc_1'],
		$_REQUEST['UserWalletAddress'], $Rev3alTXN, $Rev3alTimeStamp, $_REQUEST['UserCopyrightInfo'], $processindDuration);
	if($dbRowId<0)
	{
		DebugPring("Failed to create entry in DB");
		$ret['ErrorReason']="Failed to create entry in DB";
		return $ret;
	}
	DebugPring("Registration to DB took ".(Time_ms() - $startSingleProcess)." ms");
	
	// save the original image so we can regenerate hashes/make comparisons on it
	$startSingleProcess = Time_ms();
	$savedOriginalFile = moveUploadedToMountedVolume($TempFileNameFullPath, $dbRowId);
	if( $savedOriginalFile == "" )
	{
		DebugPring("Failed to move file to Volume");
		$ret['ErrorReason']="Failed to move file to Volume";
		return $ret;
	}
	$ret['LocalPath'] = $savedOriginalFile;
	DebugPring("Adding to persistent storage took ".(Time_ms() - $startSingleProcess)." ms");
	
	return $ret;
}
?>
