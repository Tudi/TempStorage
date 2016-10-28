<?php
session_start();
include_once( "include/DBConnection.inc.php" );
include_once( "include/Functions.inc.php" );

if( isset( $GroupId ) )
	$_SESSION["IdeaGroup"] = $GroupId;
else 
	$GroupId = $_SESSION["IdeaGroup"];
?>

<table border="1">
	<tr>
		<td>Idea</td>
		<td>Vote</td>
	</tr>
	<?php
		$query = "select Id,Title,Idea,AllowVote,RegisteredVotes,AllowComments,CreateStamp,EditToken,UpVoteCount,DownVoteCount,Status,HandledByUser from ideas where GroupId=$GroupId";
		if( isset( $SearchString ) && strlen( $SearchString ) > 0 )
			$query .= " and title like '%".mysql_real_escape_string( $SearchString )."%'";
		if( isset( $from ) && isset( $to ) )
			$query .= " limit $from,$to";
		$result = mysql_query($query,$dbi) or die("201602101451".$query);
		while( list( $Id,$Title,$Idea,$AllowVote,$RegisteredVotes,$AllowComments,$CreateStamp,$EditToken,$UpVoteCount,$DownVoteCount,$Status,$HandledByUser ) = mysql_fetch_row( $result ) )
		{
			echo "<tr>";
			
			//maybe strip HTML tags ? Or we allow that for payed users ? :P
			echo "<td><a href=\"AddIdea.php?eid=$Id&action=le\"> $Title</a></td>\n";
			
			if( $UserId > 0 )
			{
				//check if this user can vote this idea
				$query1 = "select VoteResult from uservotes where sessionid=".$_SESSION['SessionId']." and ideaid=$Id";
				$result1 = mysql_query($query1,$dbi) or die("201602101514".$query1);
				list( $VoteResult ) = mysql_fetch_row( $result1 );
				if( $VoteResult != 0 )
					$AllowVote = 0;
				
				if( $AllowVote > 0 && ( $RegisteredVotes <= 0 || ( $_SESSION['UserId'] > 0 && $RegisteredVotes > 0 ) ) )
					echo "<td><input type=\"button\" id=\"{$Id}_1\" value=\"$UpVoteCount\" onclick=ChangeVote($Id,1) /></td>\n";
				else
					echo "<td><input type=\"button\" id=\"{$Id}_1\" value=\"$UpVoteCount\" disabled /></td>\n";
			}		
			echo "</tr>\n";
		}
	?>
	<tr>
		<td></td>
		<td>
			<a href="AddIdea.php">Add idea</a>
		</td>
	</tr>
</table>

<iframe name="VariableContent2" id="VariableContent2" src="" width=1 height=1></iframe>

<script>
function ChangeVote( id, amt )
{
	//update UI
	var Name1 = id + "_1";
	var Object1 = document.getElementById( Name1 );
	if( Object1.disabled == false )
	{
		Object1.value = parseInt( Object1.value ) + Math.abs( amt );
		Object1.disabled = true;
		
		//update DB also
		document.getElementById('VariableContent2').src = "list_ideas_actions.php?v=" + amt + "&id=" + id;
	}
}
</script>
