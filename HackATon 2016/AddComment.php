<?php
session_start();
include_once( "include/DBConnection.inc.php" );
include_once( "include/Functions.inc.php" );

$EditToken = "";

//if we are trying to edit an idea, we load the Data from the DB
if( isset( $action ) && isset( $eid ) && $eid > 0 )
{
	//do we need to load data or we need to save data
	if( $action == "le" )
	{
		$query = "select Id,UserId,IdeaComment,EditToken from comments where id=$eid";
		$result = mysql_query($query,$dbi) or die("20160211154".$query);
		list( $Id,$eUserId,$Comment,$EditToken ) = mysql_fetch_row( $result );	
	}
	//are we allowed to edit this idea ?
	$query = "select EditToken from ideas where id=$eid";
	$result = mysql_query($query,$dbi) or die("201602151200".$query);
	list( $EditToken ) = mysql_fetch_row( $result );	
	$query1 = "select count(*) from usertokens where token='".mysql_real_escape_string( $EditToken )."' and sessionid=".$_SESSION['SessionId'];
	$result1 = mysql_query($query1,$dbi) or die("201602101501".$query1);
	list( $EditIsAllowed ) = mysql_fetch_row( $result1 );
}

//did we get called to register a new idea ?
if( isset( $Comment ) && strlen( $Comment ) > 0 && ( !isset( $eid ) || $eid == 0 ) )
{
	//check for anti spam protection for this user
	
	//register comment as a new comment
	$EditToken = GenerateUniqueToken();
	
	$query = "insert into comments ( UserId, IdeaId, IdeaComment, fingerprint, createstamp, edittoken ) values ";
	$query .= "( $UserId, $IdeaId, '".mysql_real_escape_string( $Comment )."'";
	$query .= ",'".mysql_real_escape_string( $fp )."', ".time().", '".mysql_real_escape_string( $EditToken )."')";
	$result = mysql_query($query,$dbi) or die("201602101259".$query );
	
	//memorize session tokens, so the user can edit comments even if he is not logged in
	$query = "insert into usertokens ( sessionid, token ) values ( ".$_SESSION['SessionId'].", '".mysql_real_escape_string( $EditToken )."')";
	$result = mysql_query($query,$dbi) or die("201602101258".$query);
}

//in case this is an edit action, we will receive the ID we want to load up
if( isset( $action ) && isset( $eid ) && $eid > 0 )
{
	if( $action == "e" && $EditIsAllowed <= 0 )
		jumpto( "list_ideas.php" );

	if( $action == "e" )
	{
		if( strlen( $Comment ) == 0 )
		{
			//make sure we have 0 comments attached to this idea
			DeleteComment( $eid );
		}
		else
		{	
			$query = "update comments set IdeaComment='".mysql_real_escape_string( $Comment )."'";
			$query .= "where id=$eid";
			$result = mysql_query($query,$dbi) or die("201602111249".$query );
		}
	}
	if( $action == "le" )
		$action = "e";	// on next reload, save changes
}
?>

<script src="include/fingerprint2.js"></script>
<script src="include/Functions.js"></script>
<form name="addCommentform" id="addCommentform"  action="AddComment.php">
	Comment : <textarea name="Comment" id="Comment" rows="8" cols="70"><?php echo $Comment;?></textarea><br/>
	<input type="hidden" id="IdeaId" name="IdeaId" value="<?php echo $IdeaId;?>" />
	<input type="hidden" id="fp" name="fp" value="" />
	<input type="hidden" id="action" name="action" value="<?php echo $action;?>" />
	<input type="hidden" id="eid" name="eid" value="<?php echo $eid;?>" />
	<input type="hidden" id="eUserId" name="eUserId" value="<?php echo $eUserId;?>" />
	<input type="submit" value="Add comment"><br/>
</form>

<script>
//assign a fingerprint to this page. It might not be unique, but it might get used as a hint later
SetFingerPrint();
</script>
