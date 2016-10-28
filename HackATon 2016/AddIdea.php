<?php
session_start();
include_once( "include/DBConnection.inc.php" );
include_once( "include/Functions.inc.php" );

$EditToken = "";

if( isset( $GroupId ) )
	$_SESSION["IdeaGroup"] = $GroupId;
else 
	$GroupId = $_SESSION["IdeaGroup"];

//if we are trying to edit an idea, we load the Data from the DB
if( isset( $action ) && isset( $eid ) && $eid > 0 )
{
	//do we need to load data or we need to save data
	if( $action == "le" )
	{
		$query = "select Id,UserId,Title,Idea,AllowVote,RegisteredVotes,AllowComments,CreateStamp,EditToken,UpVoteCount,DownVoteCount,Status,HandledByUser from ideas where id=$eid";
		$result = mysql_query($query,$dbi) or die("20160211154".$query);
		list( $Id,$eUserId,$Title,$Idea,$AllowVote,$RegisteredVotes,$AllowComments,$CreateStamp,$EditToken,$UpVoteCount,$DownVoteCount,$Status,$HandledByUser ) = mysql_fetch_row( $result );	
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
if( isset( $Idea ) && isset( $Title )&& strlen( $Title ) > 0 && ( !isset( $eid ) || $eid == 0 ) )
{
	//check for anti spam protection for this user
	
	$EditToken = GenerateUniqueToken();
	if( isset($CanVote) ) $CanVote = (int)1;
	if( isset($CanComment) ) $CanComment = (int)1;
	if( isset($OnlyRegistered) ) $OnlyRegistered = (int)1;
	
	$query = "insert into ideas ( UserId, GroupId, Title, idea, allowvote, allowcomments, Registeredvotes, fingerprint, createstamp, edittoken ) values ";
	$query .= "( '$UserId', '$GroupId', '".mysql_real_escape_string( $Title )."', '".mysql_real_escape_string( $Idea )."','$CanVote','$CanComment','$OnlyRegistered'";
	$query .= ",'".mysql_real_escape_string( $fp )."', ".time().", '".mysql_real_escape_string( $EditToken )."')";
	$result = mysql_query($query,$dbi) or die("201602101259".$query );
	
	//memorize session tokens, so the user can edit comments even if he is not logged in
	$query = "insert into usertokens ( sessionid, token ) values ( ".$_SESSION['SessionId'].", '".mysql_real_escape_string( $EditToken )."')";
	$result = mysql_query($query,$dbi) or die("201602101258".$query);
	
	jumpto( "list_ideas.php" );	
}

//in case this is an edit action, we will receive the ID we want to load up
if( isset( $action ) && isset( $eid ) && $eid > 0 )
{
	if( $action == "e" && $EditIsAllowed <= 0 )
		jumpto( "list_ideas.php" );
	
	if( $action == "e" )
	{
		if( strlen( $Idea ) == 0 || strlen( $Title ) == 0 )
		{
			//make sure we have 0 comments attached to this idea
			DeleteIdea( $eid );
		}
		else
		{
			if( isset($CanVote) ) $CanVote = (int)1;
				else $CanVote = (int)0;
			if( isset($CanComment) ) $CanComment = (int)1;
				else $CanComment = (int)0;
			if( isset($OnlyRegistered) ) $OnlyRegistered = (int)1;
				else $OnlyRegistered = (int)0;
			
			$query = "update ideas set ";
			$query .= "Title='".mysql_real_escape_string( $Title )."'";
			$query .= ",Idea='".mysql_real_escape_string( $Idea )."'";
			$query .= ", allowvote=$CanVote, allowcomments=$CanComment, Registeredvotes=$OnlyRegistered ";
			$query .= "where id=$eid";
			$result = mysql_query($query,$dbi) or die("201602111249".$query );
		}
		jumpto( "list_ideas.php" );	
	}
	$action = "e";	// on next reload, save changes
}

$CanVoteChecked = "";
if( ( !isset($CanVote) || $CanVote == 1 ) && ( $AllowVote == 1 || !isset( $AllowVote) ) )
	$CanVoteChecked = "Checked";
$CanCommentChecked = "";
if( ( !isset($CanComment) || $CanComment == 1 ) && ( $AllowComments == 1 || !isset( $AllowComments) ) )
	$CanCommentChecked = "Checked";
$OnlyRegisteredChecked = "";
if( ( !isset($OnlyRegistered) || $OnlyRegistered == 1 ) && ( $RegisteredVotes == 1 || !isset( $RegisteredVotes ) ) )
	$OnlyRegisteredChecked = "Checked";
$AnnonymousChecked = "";
if( ( !isset($eUserId) || $eUserId == 0 ) && !isset( $visibility ) )
	$AnnonymousChecked = "checked";
$IsDisabled = "";
if( $EditIsAllowed <= 0 && $eid > 0 )
	$IsDisabled = "disabled";
$ButtonText = "Add Idea";
if( isset( $eid ) )
	$ButtonText = "Edit Idea";
?>

<script src="include/fingerprint2.js"></script>
<script src="include/Functions.js"></script>
<form name="addIdeaform" id="addIdeaform"  action="<?php echo $_SERVER['PHP_SELF']; ?>">
	Can be voted on : <input type="checkbox" id="CanVote" name="CanVote" <?php echo "$CanVoteChecked $IsDisabled"; ?>><br/>	
	Can be commented on : <input type="checkbox" id="CanComment" name="CanComment" <?php echo "$CanCommentChecked $IsDisabled"; ?>><br/>	
	No Annonymous interaction : <input type="checkbox" id="OnlyRegistered" name="OnlyRegistered" <?php echo "$OnlyRegisteredChecked $IsDisabled"; ?>><br/>
	Title : <input type="text" name="Title" id="Title" value="<?php echo $Title;?>" <?php echo $IsDisabled;?>><br/>
	Idea : <textarea name="Idea" id="Idea" rows="8" cols="70" <?php echo $IsDisabled;?>><?php echo $Idea;?></textarea><br/>
	<input type="hidden" id="fp" name="fp" value="" />
	<input type="hidden" id="action" name="action" value="<?php echo $action;?>" />
	<input type="hidden" id="eid" name="eid" value="<?php echo $eid;?>" />
	<input type="hidden" id="eUserId" name="eUserId" value="<?php echo $eUserId;?>" />
	<input type="submit" value="<?php echo $ButtonText;?>"><br/>
</form>

<?php 
if( $eid > 0 ) 
{ 
	$IdeaId = $eid;
	//check if we already voted
	$query1 = "select VoteResult from uservotes where sessionid=".$_SESSION['SessionId']." and ideaid=$IdeaId";
	$result1 = mysql_query($query1,$dbi) or die("201602101600".$query1);
	list( $VoteResult ) = mysql_fetch_row( $result1 );
	if( $VoteResult == "" )
	{
?>
		<a href="list_ideas_actions.php?v=-1&id=<?php echo $eid; ?>" >Ignore Idea</a>
		<a href="list_ideas_actions.php?v=1&id=<?php echo $eid; ?>" >Interested</a>
<?php
	} 
	$eid=0; //do not allow below pages to inherit this value
	include( "AddComment.php" );
	include( "list_comments.php" );
} 
?>
<script>
//assign a fingerprint to this page. It might not be unique, but it might get used as a hint later
SetFingerPrint();
</script>
