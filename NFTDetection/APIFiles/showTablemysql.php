<?php
ini_set('display_errors', 1); ini_set('display_startup_errors', 1); error_reporting(E_ALL);
require 'settings.cfg';
$con=mysqli_connect($AWS_DB_URL, $AWS_DB_USER, $AWS_DB_PASSW, $AWS_DB_DB);
// Check connection
if (mysqli_connect_errno())
{
	echo "Failed to connect to MySQL: " . mysqli_connect_error();
	die();
}

$try_create=0;
if($try_create)
{
	$result = mysqli_query($con, "drop table ".$AWS_DB_TABLE_REGISTERED."");
	$create_sql = "CREATE TABLE `".$AWS_DB_TABLE_REGISTERED."` (
		`Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
		`FileName` VARCHAR(2000) NOT NULL DEFAULT '0',
		`Artist` VARCHAR(2000) NOT NULL DEFAULT '0',
		`ArtTitle` VARCHAR(2000) NOT NULL DEFAULT '0',
		`UserId` INT UNSIGNED NOT NULL DEFAULT 0,
		`OriginalURL` VARCHAR(2000) NOT NULL DEFAULT '0',
		`BlockChainHash` VARCHAR(2000) NOT NULL DEFAULT '0',
		`AHash` BIGINT UNSIGNED NULL DEFAULT 0,
		`PHash` BIGINT UNSIGNED NULL DEFAULT 0,
		`MHash` BIGINT UNSIGNED NULL DEFAULT 0,
		`CreatedAt` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
		`PeerIp` VARCHAR(50) NULL DEFAULT NULL,
		`FinishedAt` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
		`CallBackURL` VARCHAR(2000) NOT NULL DEFAULT '0',
		`RowAccesToken` VARCHAR(2000),
		INDEX `row lookup` (`Id`),
		INDEX `searching` (`PHash`, `AHash`),
		PRIMARY KEY (`Id`),
		UNIQUE INDEX `Id` (`Id`)
	)COLLATE='utf8mb4_0900_ai_ci'";
	$result = mysqli_query($con, $create_sql);
	if($result == false)
	{
		echo mysqli_error($con);
	}
}

$result = mysqli_query($con,"select * from ".$AWS_DB_TABLE_REGISTERED."");
if($result != false)
{
	//while($row = mysqli_fetch_array($result))
	//	var_dump($row);
	printTableImgInfo($result);
}
else
{
	echo "There was an sql error :".mysqli_error($con);
}

mysqli_close($con);

function printTableImgInfo($result)
{
	echo "Content of the registerd images table<br>";
	echo "<table border=1>";
	$rows_printed = 0;
	while($row = mysqli_fetch_array($result))
	{
		if($rows_printed==0)
		{
			echo "<tr>";
			$i_printed = 0;
			foreach($row as $key => $val)
			{
				if(($i_printed & 1) == 1 )
					echo "<td>".$key."</td>";
				$i_printed++;
			}
			echo "</tr>";
		}
		$values = array_values($row);
		echo "<tr>";
		for($i=0;$i<count($values);$i+=2)
		{
			echo "<td>".$values[$i]."</td>";
		}
		echo "</tr>";
		$rows_printed++;
	}
	echo "</table>";
}
?>
