<?php
if(isset($dbi))
	mysql_close($dbi);

if(isset($CurCacheFileName) && $CurCacheFileName != "" )
	AutoCacheEnd();
?>