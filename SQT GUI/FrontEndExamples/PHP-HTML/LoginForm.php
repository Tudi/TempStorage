<?php
//logout support
if( $_REQUEST['Logout'] )
	$_SESSION['login'] = 0;
	
//check login
if( !isset( $_SESSION['login'] ) || $_SESSION['login'] != 1 )
{
	$HasLogin = 0;
	if( isset( $_REQUEST['UserName'] ) )
	{
		if( strcmp( $_REQUEST['UserName'], 'niamh08' ) == 0 )
		{
			$HasLogin = 1;
			$_SESSION['login'] = 1;
		}
	}
	//show login frame
	if( $HasLogin == 0 )
	{
		?>
		<form name="Login" id="Login" action="<?php echo $_SERVER['PHP_SELF']; ?>">
			Username : <input type="text" name="UserName" value="<?php echo $UserName; ?>"><br/>
			<input type="submit" value="Login"<br/>
		</form>
		<?php
		//kill execution at this point
		exit();
	}
}
// show logout
?>
<a href="<?php echo $_SERVER['PHP_SELF']; ?>?Logout=1">Logout</a>		<br>
