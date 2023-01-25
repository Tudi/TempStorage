<?php

function GUID()
{
    if (function_exists('com_create_guid') === true)
    {
        return trim(com_create_guid(), '{}');
    }

    return sprintf('%04X%04X-%04X-%04X-%04X-%04X%04X%04X', mt_rand(0, 65535), mt_rand(0, 65535), mt_rand(0, 65535), mt_rand(16384, 20479), mt_rand(32768, 49151), mt_rand(0, 65535), mt_rand(0, 65535), mt_rand(0, 65535));
}

function generateRev3alID($CopyrightInfo, $PHash, $MHash, $NearTXN, $NearTimestamp)
{
	global $REV3AL_ID_SEPARATOR_TOKEN;
	$ret['id'] = $CopyrightInfo.$REV3AL_ID_SEPARATOR_TOKEN.$PHash.$REV3AL_ID_SEPARATOR_TOKEN.$MHash.$REV3AL_ID_SEPARATOR_TOKEN.$NearTXN.$REV3AL_ID_SEPARATOR_TOKEN.$NearTimestamp;
	$is_unique_ID = 0;
	$antiDeadlock = 10;
	while($is_unique_ID == 0 && $antiDeadlock > 0)
	{
		$ret['enc_1'] = create_encryption_key();
//		$ret['id_enc'] = encrypt($ret['enc_1'], $ret['id']); // 2 way encryption
		$ret['id_enc'] = md5($ret['enc_1'].$ret['id']); // destructive encryption. Changed from 2 way encryption. Before you say md5 is not safe, this is not a password, this is an ID shared publicly
		$is_unique_ID = 1 - checkRev3alIdUnique($ret['id_enc']);
		$antiDeadlock = $antiDeadlock - 1;
	}
	if($antiDeadlock == 0 && $is_unique_ID == 0)
	{
		DebugPring("Failed to generate unique Rev3alId in 10 retries");
		$antiDeadlock = 10;
		while($is_unique_ID == 0 && $antiDeadlock > 0)
		{
			$ret['enc_1'] = GUID();
//			$ret['id_enc'] = encrypt($ret['enc_1'], $ret['id']); // 2 way encryption
			$ret['id_enc'] = md5($ret['enc_1'].$ret['id']); // destructive encryption. Changed from 2 way encryption. Before you say md5 is not safe, this is not a password, this is an ID shared publicly
			$is_unique_ID = 1 - checkRev3alIdUnique($ret['id_enc']);
			$antiDeadlock = $antiDeadlock - 1;
		}
		if($antiDeadlock == 0 && $is_unique_ID == 0)
		{
			DebugPring("Second attempt : Failed to generate unique Rev3alId in 10 retries");
		}
	}
	return $ret;
}

function watermarkImageOnEC2($FileNamePath, $WaterMarkString, $EncryptionString)
{
	global $WATERMARK_APP_PATH;
	$outputFileName = $FileNamePath."_watermarked";
	$startSingleProcess = Time_ms();
	$cmd = $WATERMARK_APP_PATH." 3 ".$FileNamePath." ".$outputFileName." $WaterMarkString $EncryptionString";
	DebugPring("Executing : ".$cmd);
	$ret = shell_exec($cmd);
	DebugPring("WaterMark generation took ".(Time_ms() - $startSingleProcess)." ms");
	return $outputFileName;
}

function watermarkAndUploadImageToIPFS($FileNamePath, $WaterMarkString, $EncryptionString, $GenBase64Content)
{
	$resultFileName = watermarkImageOnEC2($FileNamePath, $WaterMarkString, $EncryptionString);
	$watermarkInfo = uploadImageToIPFS($resultFileName);
	if($GenBase64Content)
	{
		$watermarkInfo['fileContent'] = getFileContentAsBase64( $resultFileName );
	}
	else
	{
		$watermarkInfo['fileContent'] = "";
	}
	unlink($resultFileName);
	return $watermarkInfo;
}
?>