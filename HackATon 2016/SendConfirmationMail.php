<?php
session_start();
include( "DBConnection.inc.php" );
include( "Functions.inc.php" );

if( isset( $UserEmail ) )
{
	$ActionToken = GenerateUniqueToken();
	//update DB that we are sending this token to email
	$query = "update users set ConfirmationToken='".mysql_real_escape_string( $ActionToken )."' where email='".mysql_real_escape_string( $UserEmail )."'";
	$result=mysql_query($query,$dbi) or die("201602091243".$query);
	$ret = mail( $UserEmail, "Confirm Account", "http://pc371/Hack_A_Ton_2016_02/SendConfirmationMail.php?Token=$ActionToken", "From: IdeaBoard@waters.com");
	echo "Email Sent";
}

if( isset( $Token ) )
{
	//update DB that we are sending this token to email
	$query = "update users set Confirmed=1 where ConfirmationToken='".mysql_real_escape_string( $ActionToken )."'";
	$result=mysql_query($query,$dbi) or die("201602091245".$query);
}
?>

Login or create a user. You will need to confirm in order to be able to log in
<form name="Login" id="Login"  action="<?php echo $_SERVER['PHP_SELF']; ?>">
	Email : <input type="text" name="UserEmail" value="<?php echo $UserEmail; ?>"><br/>
	<input type="submit" value="Resend Validation Email"<br/>
</form>