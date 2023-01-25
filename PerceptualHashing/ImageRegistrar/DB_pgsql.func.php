<?php
function initGlobalDBConnection()
{
	global $AWS_DB_PG_URL,$AWS_DB_PG_USER,$AWS_DB_PG_PASSW,$AWS_DB_PG_DB;
	global $db_con;

	$con_str = "host=".$AWS_DB_PG_URL." port=5432 dbname=".$AWS_DB_PG_DB." user=".$AWS_DB_PG_USER." password=".$AWS_DB_PG_PASSW." connect_timeout=5 sslmode=require";
	$db_con = pg_connect($con_str);
	// Check connection
	if ($db_con == false)
	{
		DebugPring("<br><br>Con str: ".$con_str );
		DebugPring("Failed to connect to PgSQL: ". pg_last_error() );
		return -1;
	}
	return 0;
}

function closeGlobalDBConnection()
{
	global $db_con;
	pg_close($db_con);
	$db_con = false;
}

function getUserId($APIAccessToken)
{
	global $AWS_DB_TABLE_ACCESS_TOKENS, $SESSION_USER_ID;
	global $db_con;
	
	$sql = "select UserId from ".$AWS_DB_TABLE_ACCESS_TOKENS." where
	APIAccesToken='".pg_escape_string($db_con, $APIAccessToken)."'
	 limit 1";
	$result = pg_query($db_con, $sql);
	if($result != false)
	{
		if($row = pg_fetch_row($result))
		{
			return $row[0];
		}
	}
	else
	{
		DebugPring("There was an sql error :".pg_last_error($db_con));
	}
	return 0;
}

function checkRegistered($AHash, $DHash, $PHash, $match_pct, $max_matches)
{
	global $AWS_DB_TABLE_REGISTERED, $HASH_DB_BITCOUNT, $HASH_LOW_LIMIT_IMAGE_MATCH;
	global $IPFS_BASE_URL;
	global $db_con;
	$ret = "";
	
	$sql = "select * from (select IPFSWaterMarkHash,  
									bit_count(numeric_to_bit64(".$PHash.") # PHash) as pDist,  
									bit_count(numeric_to_bit64(".$AHash.") # AHash) as aDist, 
									bit_count(numeric_to_bit64(".$DHash.") # DHash) as dDist 
									from ".$AWS_DB_TABLE_REGISTERED.") res 
						where
						res.pDist<=".($HASH_DB_BITCOUNT-$HASH_LOW_LIMIT_IMAGE_MATCH)." and
						res.aDist<=".($HASH_DB_BITCOUNT-$HASH_LOW_LIMIT_IMAGE_MATCH)." and
						res.dDist<=".($HASH_DB_BITCOUNT-$HASH_LOW_LIMIT_IMAGE_MATCH)." 
						order by res.pDist asc limit $max_matches";
	$result = pg_query($db_con, $sql);
	if($result != false)
	{
		$ret = "";
		while($row = pg_fetch_row($result))
		{
			DebugPring("checkRegistered : Adding values :".$row[0]." ".$row[1]);
			$MatchPCT = 100 - (int)($row[1] * 100 / $HASH_DB_BITCOUNT); // number of bits represent distinct blocks
			$ret .= "{ \"file\":\"".$IPFS_BASE_URL.$row[0]."\",\"MatchPCT\":$MatchPCT},";
		}
		if($ret != "" )
		{
			$ret = substr($ret,0,-1);
			$ret = "{ \"IsRegistered\":\"True\",\"SimilarImages\":[".$ret."] }";
		}
		else
		{
			$ret = "{ \"IsRegistered\":\"False\",\"SimilarImages\":[] }";
		}
	}
	else
	{
		DebugPring("There was an sql error :".pg_last_error($db_con)." <br> sql : $sql");
		$ret = "";
	}
	
	return $ret;
}

function checkRev3alIdUnique($Rev3alId)
{
	global $AWS_DB_TABLE_REGISTERED;
	global $db_con;
	
	$sql = "select count(id) from ".$AWS_DB_TABLE_REGISTERED." where 'Rev3alId'='".pg_escape_string($db_con, $Rev3alId)."' limit 1";
	$result = pg_query($db_con, $sql);
	if($result != false)
	{
		$row = pg_fetch_row($result);
		$ret = $row[0];
	}
	else
	{
		DebugPring("There was an sql error :".pg_last_error($db_con)." <br> sql : $sql");
		$ret = 1;
	}
	
	return $ret;
}

function getPeerIP()
{
	if(!empty($_SERVER['HTTP_CLIENT_IP'])) {  
        return $_SERVER['HTTP_CLIENT_IP'];  
    }  
    //if user is from the proxy  
    elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR'])) {  
        return $_SERVER['HTTP_X_FORWARDED_FOR'];  
    }  
    //if user is from the remote address  
    else{  
        return $_SERVER['REMOTE_ADDR'];  
    }
	return "";
}

function createDBRowForRegister($filename, $ArtistFullName, $ArtistAlias, $ArtTitle, $AHash, $DHash, $PHash, $MHash, 
	$IPFS_WaterMark_Hash, $IPFS_WaterMark_TimeStamp, $Rev3al_ID, $Enc1, $WalletTXN, $Rev3alTXN, $Rev3alTimeStamp, $CopyRightInfo, $ProcessingDuration)
{
	global $AWS_DB_TABLE_REGISTERED;
	global $SESSION_USER_ID;
	global $db_con;
	$ret = -1;
	
	$sql = "insert into ".$AWS_DB_TABLE_REGISTERED." (FileName, PeerIp, ArtistFullName, ArtistAlias, ArtTitle, AHash, PHash, DHash, MHash, 
	UserId, WalletTXN, Rev3alTXN, Rev3alTimeStamp, IPFSWaterMarkHash, IPFSWaterMarkTimeStamp, Rev3alId, EncKey1, CopyrightInfo, CreateDuration) values (
	'".pg_escape_string($db_con, $filename)."',
	'".pg_escape_string($db_con, getPeerIP())."',
	'".pg_escape_string($db_con, $ArtistFullName)."',
	'".pg_escape_string($db_con, $ArtistAlias)."',
	'".pg_escape_string($db_con, $ArtTitle)."',
	numeric_to_bit64(".$AHash."),
	numeric_to_bit64(".$PHash."),
	numeric_to_bit64(".$DHash."),
	numeric_to_bit64(".$MHash."),
	'".pg_escape_string($db_con, $SESSION_USER_ID)."',
	'".pg_escape_string($db_con, $WalletTXN)."',
	'".pg_escape_string($db_con, $Rev3alTXN)."',
	'".$Rev3alTimeStamp."',
	'".pg_escape_string($db_con, $IPFS_WaterMark_Hash)."',
	'".pg_escape_string($db_con, $IPFS_WaterMark_TimeStamp)."',
	'".pg_escape_string($db_con, $Rev3al_ID)."',
	'".pg_escape_string($db_con, $Enc1)."',
	'".pg_escape_string($db_con, $CopyRightInfo)."',
	'".pg_escape_string($db_con, $ProcessingDuration)."'
	) RETURNING id";
	$result = pg_query($db_con, $sql);
	if($result != false)
	{
		$ret = pg_fetch_row($result)[0];
	}
	else
	{
		DebugPring("There was an sql error :".pg_last_error($db_con)."<br><br> sql :".$sql);
		$ret = -2;
	}
	
	return $ret;
}


function getProcesingTime()
{
	global $AWS_DB_TABLE_REGISTERED;
	global $db_con;
	$ret = "";
	
	$sql = "select min(CreateDuration),max(CreateDuration),avg(CreateDuration) from ".$AWS_DB_TABLE_REGISTERED;
	$result = pg_query($db_con, $sql);
	if($result != false)
	{
		$ret = "";
		if($row = pg_fetch_row($result))
		{
			DebugPring("getProcesingTime : Adding values :".$row[0]." ".$row[1]." ".$row[2]);
			$ret .= "{ \"min\":\"".$row[0]."\",\"max\":\"".$row[1]."\",\"avg\":\"".$row[2]."\"}";
		}
		else
		{
			$ret = "{ \"min\":\"0\",\"max\":\"0\",\"avg\":\"0\" }";
		}
	}
	else
	{
		DebugPring("There was an sql error :".pg_last_error($db_con)." <br> sql : $sql");
		$ret = "";
	}
	
	return $ret;
}

function tryGetQueuedTXN()
{
	global $AWS_DB_TABLE_TXN_QUEUE;
	global $db_con;
	$ret['TXN'] = "";
	$ret['TimeStamp'] = "";
	
	$sql = "delete from ".$AWS_DB_TABLE_TXN_QUEUE." 
					where id in ( select id from ".$AWS_DB_TABLE_TXN_QUEUE." limit 1) 
				returning *";
	$result = pg_query($db_con, $sql);
	if($result != false)
	{
		if($row = pg_fetch_row($result))
		{
			DebugPring("Got queued TXN :".$row[1]." timestamp ".$row[2]);
			$ret['TXN'] = $row[1];
			$ret['TimeStamp'] = $row[2];
		}
	}
	else
	{
		DebugPring("There was an sql error :".pg_last_error($db_con)." <br> sql : $sql");
	}
	
	return $ret;
}

function AddQueuedTXN($TXN)
{
	global $AWS_DB_TABLE_TXN_QUEUE;
	global $db_con;
	$ret = "";
	
	$sql = "insert into ".$AWS_DB_TABLE_TXN_QUEUE."  (TXN) values (
		'".pg_escape_string($db_con, $TXN)."'
	)";
	$result = pg_query($db_con, $sql);
	if($result == false)
	{
		DebugPring("There was an sql error :".pg_last_error($db_con)." <br> sql : $sql");
	}
	else
	{
		DebugPring("Successfully executed sql : ".$sql);
	}
	
	return $ret;
}

function printAnyTableContent($tableName)
{
	global $db_con;
	$result = pg_query($db_con,"select * from ".$tableName."");
	if($result != false)
	{
		printAnyTableContent_($result,$tableName);
	}
	else
	{
		echo "There was an sql error :".pg_last_error($db_con);
	}
}

function printAnyTableContent_($result, $table)
{
	echo "Content of the $table table<br>";
	echo "<table border=1>";
	$rows_printed = 0;
	while($row = pg_fetch_row($result, null, PGSQL_ASSOC))
	{
		if($rows_printed==0)
		{
			echo "<tr>";
			foreach($row as $key => $val)
			{
				echo "<td>".$key."</td>";
			}
			echo "</tr>";
		}
		$values = array_values($row);
		echo "<tr>";
		for($i=0;$i<count($values);$i+=1)
		{
			echo "<td>".$values[$i]."</td>";
		}
		echo "</tr>";
		$rows_printed++;
	}
	echo "</table>";
}
?>