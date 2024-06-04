delete from UserSessions where SessionCreatedAt < ( CURRENT_TIMESTAMP + 60*60*24 ); -- delete sessions that are older than 1 day. Allow statistics to be generated ( if anyone wants to )
delete from PsswRstQueue where ReqCreatedTimestamp < ( CURRENT_TIMESTAMP + 60*60*24 ); -- able to detect hack attempts
delete from UserAPIActivity where LastUsedAPIStamp < ( CURRENT_TIMESTAMP + 60*60*24 ); -- able to detect hack attempts
