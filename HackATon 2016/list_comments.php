<?php
session_start();
include_once( "include/DBConnection.inc.php" );
include_once( "include/Functions.inc.php" );

if( !isset( $paging_from ) || $paging_from < 0 )
	$paging_from = 0;
if( !isset( $paging_to ) )
	$paging_to = 1000;
?>

<table border="1">
	<tr>
		<td>comment</td>
		<td>User</td>
	</tr>
	<?php
		$query = "select Id,UserId,IdeaComment,CreateStamp,EditToken from comments where ideaid=$IdeaId limit $paging_from,$paging_to";
		$result = mysql_query($query,$dbi) or die("201602111148".$query);
		while( list( $Id,$UserId,$IdeaComment,$CreateStamp,$EditToken ) = mysql_fetch_row( $result ) )
		{
			echo "<tr>";
			
			//check if this user can edit this comment
			$query1 = "select count(*) from usertokens where token='".mysql_real_escape_string( $EditToken )."' and sessionid=".$_SESSION['SessionId'];
			$result1 = mysql_query($query1,$dbi) or die("201602111149".$query1);
			list( $EditIsAllowed ) = mysql_fetch_row( $result1 );
			
			//maybe strip HTML tags ? Or we allow that for payed users ? :P
			if( $EditIsAllowed )
				echo "<td><textarea id=\"comm_$Id\" onchange=UpdateCommentContent($Id)>$IdeaComment</textarea></td>\n";
			else
				echo "<td><textarea disabled>$IdeaComment</textarea></td>\n";
			
			$CommentUserName = "";
			if( $UserId )
			{
				$query1 = "select name from users where id=$UserId";
				$result1 = mysql_query($query1,$dbi) or die("201602111146".$query1);
				list( $CommentUserName ) = mysql_fetch_row( $result1 );
			}
			echo "<td>$CommentUserName</td>\n";
			
			echo "</tr>\n";
		}
	?>
</table>
<a href="list_comments.php?IdeaId=<?php echo $IdeaId;?>&paging_from=<?php echo $paging_from-100;?>&paging_to=<?php echo $paging_to-100;?>"> < < Previous page</a>
.......
<a href="list_comments.php?IdeaId=<?php echo $IdeaId;?>&paging_from=<?php echo $paging_from+100;?>&paging_to=<?php echo $paging_to+100;?>"> Next page > > </a>

<iframe name="VariableContent3" id="VariableContent3" src="" width=1 height=1></iframe>

<script>
function UpdateCommentContent( id )
{
	var CommentContent = document.getElementById( "comm_" + id );
	var BackGroundUpdater = document.getElementById( "VariableContent3" );
	BackGroundUpdater.src = "AddComment.php?eid="+id+"&action=e&Comment="+CommentContent.value;
}
</script>

