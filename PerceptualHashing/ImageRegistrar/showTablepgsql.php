<?php
ini_set('display_errors', 1); ini_set('display_startup_errors', 1); error_reporting(E_ALL);
require 'settings.php';
require 'DB_pgsql.func.php';
require 'DebugMsg.func.php';
$ShowDebugMessages = 1;

initGlobalDBConnection();
#phpinfo();
if($db_con == false)
	die("failed to connect to the DB");


$try_create=0;
if($try_create)
{
	$create_user_table=0;
	$create_registrar_table=0;
	$create_numeric_convert_function=0;
	$create_blockchain_TXN_QUEUE=0;
	if($create_blockchain_TXN_QUEUE)
	{
		$result = pg_query($db_con, "drop table ".$AWS_DB_TABLE_TXN_QUEUE."");
		$create_sql = "CREATE TABLE ".$AWS_DB_TABLE_TXN_QUEUE." (
			Id SERIAL PRIMARY KEY,
			TXN VARCHAR(256) NOT NULL DEFAULT '',
			TXNTimeStamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
		)";
		$result = pg_query($db_con, $create_sql);
		if($result == false)
		{
			echo pg_last_error($db_con);
			echo "<br> sql : $create_sql <br>";
		}
	}
	if($create_registrar_table)
	{
		$result = pg_query($db_con, "drop table ".$AWS_DB_TABLE_REGISTERED."");
		$create_sql = "CREATE TABLE ".$AWS_DB_TABLE_REGISTERED." (
			Id SERIAL PRIMARY KEY,
			FileName VARCHAR(1000) NOT NULL DEFAULT '',
			ArtistFullName VARCHAR(1000) NOT NULL DEFAULT '',
			ArtistAlias VARCHAR(1000) NOT NULL DEFAULT '',
			ArtTitle VARCHAR(1000) NOT NULL DEFAULT '',
			CopyrightInfo VARCHAR(2000) NOT NULL DEFAULT '',
			WalletTXN VARCHAR(128) NOT NULL DEFAULT '',
			Rev3alTXN VARCHAR(128) NOT NULL DEFAULT '',
			Rev3alTimeStamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
			IPFSWaterMarkHash VARCHAR(128) NOT NULL DEFAULT '',
			IPFSWaterMarkTimeStamp VARCHAR(64) NOT NULL DEFAULT '',
			Rev3alId VARCHAR(128) NOT NULL DEFAULT '',
			EncKey1 VARCHAR(1024) NOT NULL DEFAULT '',
			AHash BIT(64),
			DHash BIT(64),
			PHash BIT(64),
			MHash BIT(64),
			PeerIp VARCHAR(64) NULL DEFAULT NULL,
			UserId INT  NOT NULL DEFAULT 0,
			CreateDuration INT  NOT NULL DEFAULT 0
		)";
		$result = pg_query($db_con, $create_sql);
		if($result == false)
		{
			echo pg_last_error($db_con);
			echo "<br> sql : $create_sql <br>";
		}
	}
	if($create_user_table)
	{
		$result = pg_query($db_con, "drop table ".$AWS_DB_TABLE_ACCESS_TOKENS."");
		$create_sql = "CREATE TABLE ".$AWS_DB_TABLE_ACCESS_TOKENS." (
			Id SERIAL PRIMARY KEY,
			FirstName VARCHAR(2000) NOT NULL DEFAULT '0',
			LastName VARCHAR(2000) NOT NULL DEFAULT '0',
			CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
			Email VARCHAR(2000) NOT NULL DEFAULT '0',
			UserId INT  NOT NULL DEFAULT 0,
			RequestRateLimit INT  NULL DEFAULT 0,
			RequestRateLimitInterval INT  NULL DEFAULT 0,
			APIAccesToken VARCHAR(2000),
			APIAccesTokenPrivate VARCHAR(2000),
			RequestsMade BIGINT  NULL DEFAULT 0,
			MaxTotalRequests BIGINT  NULL DEFAULT 0,
			ParallelRequestsNow BIGINT  NULL DEFAULT 0,
			MaxParallelRequests BIGINT  NULL DEFAULT 0,
			DisabledToken INT  NOT NULL DEFAULT 0,
			TokenExpiresAt TIMESTAMP DEFAULT '2034-12-01 00:00:00'
		)";
		$result = pg_query($db_con, $create_sql);
		if($result == false)
		{
			echo pg_last_error($db_con);
			echo "<br> sql : $create_sql <br>";
		}
//		$sql = "insert into ".$AWS_DB_TABLE_ACCESS_TOKENS."(FirstName,LastName,Email,UserId,RequestRateLimit,RequestRateLimitInterval,APIAccesToken,MaxParallelRequests,MaxTotalRequests) 
//			values ('dev','test','nobody@nowhere.at',1,5000,1,".getSQLGenerateUniqueString($AWS_DB_API_TOKEN_LEN).",1,0x7FFFFFFF)";
		$sql = "insert into ".$AWS_DB_TABLE_ACCESS_TOKENS."(FirstName,LastName,Email,UserId,RequestRateLimit,RequestRateLimitInterval,APIAccesToken,MaxParallelRequests,MaxTotalRequests) 
			values ('dev','test','nobody@nowhere.at',1,5000,1,'70de25ace207dfe638074b6082179b9a',1,x'7FFFFFFF'::bigint)";
		$result = pg_query($db_con, $sql);
		if($result == false)
		{
			echo pg_last_error($db_con)."<br> $sql";
			echo "<br> sql : $sql <br>";
		}
	}
	if($create_numeric_convert_function)
	{
		$sql = "CREATE OR REPLACE FUNCTION numeric_to_bit64(NUMERIC)
			  RETURNS BIT VARYING AS $$
			DECLARE
			  num ALIAS FOR $1;
			  -- 1 + largest positive BIGINT --
			  max_bigint NUMERIC := '9223372036854775808' :: NUMERIC(19, 0);
			  result BIT VARYING;
			BEGIN
			  WITH
				  chunks (exponent, chunk) AS (
					SELECT
					  exponent,
					  floor((num / (max_bigint ^ exponent) :: NUMERIC(300, 20)) % max_bigint) :: BIGINT
					FROM generate_series(0, 5) exponent
				)
			  SELECT bit_or(chunk :: BIT(64) :: BIT VARYING << (63 * (exponent))) :: BIT VARYING
			  FROM chunks INTO result;
			  RETURN result;
			END;
			$$ LANGUAGE plpgsql;";
		$result = pg_query($db_con, $sql);
		if($result == false)
		{
			echo pg_last_error($db_con)."<br> $sql";
		}
	}
}

printAnyTableContent($AWS_DB_TABLE_ACCESS_TOKENS);
printAnyTableContent($AWS_DB_TABLE_REGISTERED);
printAnyTableContent($AWS_DB_TABLE_TXN_QUEUE);

closeGlobalDBConnection();

?>