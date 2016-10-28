<?php
//disable warnings for now 
session_start();
include( "include/DBConnection.inc.php" );
include( "include/Functions.inc.php" );

$CanRegister = 1;
//check if we want to register
if( isset( $UserEmail ) && strlen( $UserEmail ) > 0 && strpos( $UserEmail, "@" ) != 0 )
{
	$tSessionId = CreateUser( $UserName, $UserPassword, $UserEmail );
	PrintStatus();
}
else if( isset( $UserName ) && isset( $UserPassword ) && IsLoggedIn() == 0 )
{
	$tSessionId = Login( $UserName, $UserPassword );
	if( $SessionId != "" )
		echo "User logged in successfully<br/>";
}
?>


<script src="include/fingerprint2.js"></script>
<script src="include/Functions.js"></script>

Login or create a user. You will need to confirm in order to be able to log in
<form name="Login" id="Login" action="<?php echo $_SERVER['PHP_SELF']; ?>">
	Username : <input type="text" name="UserName" value="<?php echo $UserName; ?>"><br/>
	Password : <input type="text" name="UserPassword" value="<?php echo $UserPassword; ?>"><br/>
	Fill these on new account : <br />
	Email : <input type="text" name="UserEmail" value="<?php echo $UserEmail; ?>"><br/>
	<input type="hidden" name="fp" value="" />
	<input type="hidden" name="SessionToken" value="<?php echo $UserToken; ?>" />
	<input type="submit" value="Login"<br/>
</form>
<a href="SendConfirmationMail.php" >Confirm Email</a>
<script>
	SetFingerPrint();
</script>