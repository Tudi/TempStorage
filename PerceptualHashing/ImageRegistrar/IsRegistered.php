<?php
require 'settings.php';
require 'DebugMsg.func.php';
require 'DB_pgsql.func.php';	
initGlobalDBConnection();

// check if a valid access token was provide
include("session.incl.php");
defaultRegisterErrorHandler(($SESSION_USER_ID == 0), "IsRegistered", "Unable to validate APIAccessToken");

require 'ec2hash.func.php';
require 'UploadImg.func.php';

$hashes = getImageHashes();
defaultRegisterErrorHandler(($hashes == "" || !isset($hashes["PHASH"])), "IsRegistered", "Failed to obtain hash for ImgFile");
DebugPring("Using hashes a=".$hashes["AHASH"]." d=".$hashes["DHASH"]." p=".$hashes["PHASH"] );

$registerdJSON = checkRegistered($hashes["AHASH"], $hashes["DHASH"], $hashes["PHASH"], $HASH_LOW_LIMIT_IMAGE_MATCH, $HASH_COUNT_RETURN_MATCHING_IMAGES);

echo $registerdJSON;

closeGlobalDBConnection();
?>