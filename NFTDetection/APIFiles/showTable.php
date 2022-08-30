<?php
ini_set('display_errors', 1); ini_set('display_startup_errors', 1); error_reporting(E_ALL);
#phpinfo();
$db_connection = pg_connect("host=rev3alimageinfo.cntmkgzhegqt.us-east-1.rds.amazonaws.com port=5432 dbname=imageinfo user=rev3al2 password=rev3al.Rev3al2");
if($db_connection == false)
	die("failed to connect to the DB");
//$result = pg_query($db_connection, "SELECT * FROM pg_catalog.pg_tables where tablename not like 'pg_%'");
//$result = pg_query($db_connection, "select * from rds_tools.role_password_encryption_type()");
//$result = pg_query($db_connection, "create role rev3al2 with password 'rev3al.Rev3al2' login");
//$result = pg_query($db_connection, "grant rds_superuser to rev3al2");
//$result = pg_query($db_connection, "SHOW password_encryption");
//$result = pg_query($db_connection, "SET password_encryption  = 'md5'");
//$result = pg_query($db_connection, "ALTER USER \"rev3al2\" with password 'rev3al.Rev3al2'");
//$result = pg_query($db_connection, "SELECT datname FROM pg_database WHERE datistemplate = false");
$result = pg_query($db_connection, "select * from imginfo");
if($result)
{
//	var_dump(pg_fetch_all($result));
	printTableImgInfo(pg_fetch_all($result));
}
else
	echo "failed to execute query";
pg_close($db_connection);

function printTableImgInfo($result)
{
	?>
	<table border=1>
		<tr>
			<td>row id</td>
			<td>s3_name</td>
			<td>a_hash</td>
			<td>p_hash</td>
		</tr>
		<?php
		foreach($result as $key => $val)
		{
			$val2 = array_values($val);
			echo "<tr><td>{$val2[0]}</td><td>{$val2[1]}</td><td>{$val2[2]}</td><td>{$val2[3]}</td></tr>";
		}
		?>
		<?
	</table>
	<?php
}
?>