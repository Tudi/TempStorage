-- role right defines. These must have fixed values as they are shared across multiple applications
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (10, 'Create User');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`,`RightDescription`) values (20, 'View Users','Same organization');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`,`RightDescription`) values (30, 'View Any User','Other organizations');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`,`RightDescription`) values (40, 'Edit Users', 'Same organization');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`,`RightDescription`) values (50, 'Edit Any User', 'Other organizations');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (60, 'Set higher user role');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (70, 'Delete Users');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (80, 'View User Logs');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (90, 'View User Activity');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (100, 'View Organization Logs');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (110, 'View Organization Activity');

REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (120, 'Create Alert');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (130, 'View Alert');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (140, 'Edit Alert');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (150, 'Delete Alert');

REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (160, 'Create Report');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (170, 'View Report');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (180, 'Edit Report');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (190, 'Delete Report');

REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (200, 'Create Radar');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (210, 'View Radar');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (220, 'Edit Radar');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (230, 'Delete Radar');

REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (240, 'Create Organization');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (250, 'View Organization');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (260, 'Edit Organization');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (270, 'Delete Organization');

REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (280, 'Create Location');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (290, 'View Location');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (300, 'Edit Location');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (310, 'Delete Location');

REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (320, 'Create Module');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (330, 'View Module');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (340, 'Edit Module');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (350, 'Delete Module');

REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (360, 'Save KPI data');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (370, 'Save Crash data');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (380, 'Fetch Organization DPS servers');

REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (400, 'View other Organization');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (401, 'Update other Organization');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (450, 'View other User');
REPLACE INTO RoleRightDefines (`RightID`, `RightName`) values (451, 'Update other User');

-- UserRoleDefines
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('Owner');
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('Manager');
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('Associate');
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('Field Engineer');
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('OwnerInternal');
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('ManagerInternal');
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('AssociateInternal');
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('FieldEngineer');
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('AdminExternal');
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('ManagerExternal');
INSERT IGNORE INTO UserRoleDefines (`RoleName`) values ('AssociateExternal');

-- UserActiveStates
INSERT IGNORE INTO UserActiveStates (`UserActiveID`, `UserActiveName`) values (1, 'Active');
INSERT IGNORE INTO UserActiveStates (`UserActiveID`, `UserActiveName`) values (2, 'Disabled'); -- shows up in user list
INSERT IGNORE INTO UserActiveStates (`UserActiveID`, `UserActiveName`) values (3, 'Deleted'); -- can undelete by higher authority

-- API Endpoint defines : A list of endpoints that you can set access rights based on UserRole. Only for authenticated users !
INSERT INTO `ApiEndpointDefines` (`EndpointDefineID`, `EndpointDefineName`, `EndpointDefineDescription`) VALUES (1, 'GetAPIRedirection.php', 'Tell the App that it should use a sepcific backend server instead of the default');
INSERT INTO `ApiEndpointDefines` (`EndpointDefineID`, `EndpointDefineName`, `EndpointDefineDescription`) VALUES (2, 'GetLatencyAPI.php', 'Measure the round trip latency of App->Server->DB->Server->App');
INSERT INTO `ApiEndpointDefines` (`EndpointDefineID`, `EndpointDefineName`, `EndpointDefineDescription`) VALUES (3, 'GetLatencyRaw.php', 'Measure the round trip from App->server->App.');
INSERT INTO `ApiEndpointDefines` (`EndpointDefineID`, `EndpointDefineName`, `EndpointDefineDescription`) VALUES (4, 'Login.php', 'Obtain a session key for a user.');
INSERT INTO `ApiEndpointDefines` (`EndpointDefineID`, `EndpointDefineName`, `EndpointDefineDescription`) VALUES (5, 'PasswChange.php', 'Change a User password. Must be logged in.');
INSERT INTO `ApiEndpointDefines` (`EndpointDefineID`, `EndpointDefineName`, `EndpointDefineDescription`) VALUES (6, 'RstPasswReq.php', 'Reuqest a new passw to be sent to recovery email');
INSERT INTO `ApiEndpointDefines` (`EndpointDefineID`, `EndpointDefineName`, `EndpointDefineDescription`) VALUES (7, 'RstPasswConf.php', 'Confirm a passw reset request receied in email');
INSERT INTO `ApiEndpointDefines` (`EndpointDefineID`, `EndpointDefineName`, `EndpointDefineDescription`) VALUES (8, 'AddLog.php', 'Save to the DB a log string');
INSERT INTO `ApiEndpointDefines` (`EndpointDefineID`, `EndpointDefineName`, `EndpointDefineDescription`) VALUES (9, 'GetUserInfo.php', 'Get user info based on ID');
INSERT INTO `ApiEndpointDefines` (`EndpointDefineID`, `EndpointDefineName`, `EndpointDefineDescription`) VALUES (10, 'SetUserInfo.php', 'Update user info based on ID');

-- Log severity values. These values are also used by the UI project
INSERT IGNORE INTO LogSeverityDefines (`LogSeverityID`, `LogSeverityName`, `LogSeverityDescription`) values (1, 'Debug', NULL); 
INSERT IGNORE INTO LogSeverityDefines (`LogSeverityID`, `LogSeverityName`, `LogSeverityDescription`) values (2, 'KPI', NULL); 
INSERT IGNORE INTO LogSeverityDefines (`LogSeverityID`, `LogSeverityName`, `LogSeverityDescription`) values (4, 'Normal', NULL); 
INSERT IGNORE INTO LogSeverityDefines (`LogSeverityID`, `LogSeverityName`, `LogSeverityDescription`) values (8, 'Unexpected', NULL); 
INSERT IGNORE INTO LogSeverityDefines (`LogSeverityID`, `LogSeverityName`, `LogSeverityDescription`) values (16, 'Sever', NULL); 
INSERT IGNORE INTO LogSeverityDefines (`LogSeverityID`, `LogSeverityName`, `LogSeverityDescription`) values (32, 'Critical', NULL); 

-- log source group values. These are used across multiple projects, so try to make sure you have them syncronized
INSERT IGNORE INTO LogSourceGroupDefines (`LogSourceGroupID`, `LogSourceGroupName`, `LogSourceGroupDescription`) values (1, 'UI', 'Windows application with UI'); 
INSERT IGNORE INTO LogSourceGroupDefines (`LogSourceGroupID`, `LogSourceGroupName`, `LogSourceGroupDescription`) values (2, 'Backend', 'web server'); 
INSERT IGNORE INTO LogSourceGroupDefines (`LogSourceGroupID`, `LogSourceGroupName`, `LogSourceGroupDescription`) values (3, 'Radar', 'mounted on top of an ATM'); 
INSERT IGNORE INTO LogSourceGroupDefines (`LogSourceGroupID`, `LogSourceGroupName`, `LogSourceGroupDescription`) values (4, 'InputAI', 'Process radar related data'); 
INSERT IGNORE INTO LogSourceGroupDefines (`LogSourceGroupID`, `LogSourceGroupName`, `LogSourceGroupDescription`) values (5, 'MessageBroaker', 'Handles data queues to not kill backend'); 

-- Alert types. Influences what script is run to trigger an alarm instance
INSERT IGNORE INTO AlertType (`AlertTypeId`, `AlertTypeName`, `AlertTypeDescription`) values (1, 'Multi Person Presence', 'If there is more than 1 person at the ATM'); 
INSERT IGNORE INTO AlertType (`AlertTypeId`, `AlertTypeName`, `AlertTypeDescription`) values (2, 'Person run towards ATM', 'Faster than generic movement'); 
INSERT IGNORE INTO AlertType (`AlertTypeId`, `AlertTypeName`, `AlertTypeDescription`) values (3, 'Dangerous object', 'Person in proximity holds dangerous object'); 

-- what to do when an alert is created. Calls an internal script based on ID
INSERT IGNORE INTO AlertActionType (`AlertActionTypeId`, `AlertActionTypeName`, `AlertActionTypeDescription`) values (1, 'Send email', ''); 
INSERT IGNORE INTO AlertActionType (`AlertActionTypeId`, `AlertActionTypeName`, `AlertActionTypeDescription`) values (2, 'Send SMS', ''); 
INSERT IGNORE INTO AlertActionType (`AlertActionTypeId`, `AlertActionTypeName`, `AlertActionTypeDescription`) values (3, 'Send email + SMS', ''); 
INSERT IGNORE INTO AlertActionType (`AlertActionTypeId`, `AlertActionTypeName`, `AlertActionTypeDescription`) VALUES (4, 'Trigger', 'No email or SMS');

-- values shown on the UI based on alert status
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1, 'Triggered');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2, 'Email Sent');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (4, 'SMS Sent');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (8, 'Email pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (16, 'SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (32, 'User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2, ' Email Sent');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+4, ' SMS Sent');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+8, ' Email pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+16, ' SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+32, ' User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+4, 'Email Sent+SMS Sent');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+8, 'Email Sent+Email pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+16, 'Email Sent+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+32, 'Email Sent+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (4+8, 'SMS Sent+Email pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (4+16, 'SMS Sent+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (4+32, 'SMS Sent+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (8+16, 'Email pending+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (8+32, 'Email pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (16+32, 'SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+4, ' Email Sent+SMS Sent');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+8, ' Email Sent+Email pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+16, ' Email Sent+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+32, ' Email Sent+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+4+8, ' SMS Sent+Email pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+4+16, ' SMS Sent+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+4+32, ' SMS Sent+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+8+16, ' Email pending+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+8+32, ' Email pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+16+32, ' SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+4+8, 'Email Sent+SMS Sent+Email pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+4+16, 'Email Sent+SMS Sent+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+4+32, 'Email Sent+SMS Sent+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+8+16, 'Email Sent+Email pending+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+8+32, 'Email Sent+Email pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+16+32, 'Email Sent+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (4+8+16, 'SMS Sent+Email pending+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (4+8+32, 'SMS Sent+Email pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (4+16+32, 'SMS Sent+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (8+16+32, 'Email pending+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+4+8, ' Email Sent+SMS Sent+Email pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+4+16, ' Email Sent+SMS Sent+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+4+32, ' Email Sent+SMS Sent+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+8+16, ' Email Sent+Email pending+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+8+32, ' Email Sent+Email pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+16+32, ' Email Sent+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+4+8+16, ' SMS Sent+Email pending+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+4+8+32, ' SMS Sent+Email pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+4+16+32, ' SMS Sent+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+8+16+32, ' Email pending+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+4+8+16, 'Email Sent+SMS Sent+Email pending+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+4+8+32, 'Email Sent+SMS Sent+Email pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+4+16+32, 'Email Sent+SMS Sent+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+8+16+32, 'Email Sent+Email pending+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (4+8+16+32, 'SMS Sent+Email pending+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+4+8+16, ' Email Sent+SMS Sent+Email pending+SMS Pending');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+4+8+32, ' Email Sent+SMS Sent+Email pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+4+16+32, ' Email Sent+SMS Sent+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+8+16+32, ' Email Sent+Email pending+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+4+8+16+32, ' SMS Sent+Email pending+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (2+4+8+16+32, 'Email Sent+SMS Sent+Email pending+SMS Pending+User Confirmed');
insert ignore INTO AlertStatusType (`AlertStatusTypeId`, `AlertStatusTypeName`) values (1+2+4+8+16+32, ' Email Sent+SMS Sent+Email pending+SMS Pending+User Confirmed');

-- module status types
INSERT IGNORE INTO ModuleStatusTypes (`ModuleStatusID`, `ModuleStatusName`) values (1, 'Online'); 
INSERT IGNORE INTO ModuleStatusTypes (`ModuleStatusID`, `ModuleStatusName`) values (2, 'Offline'); 
INSERT IGNORE INTO ModuleStatusTypes (`ModuleStatusID`, `ModuleStatusName`) values (3, 'Disabled');
INSERT IGNORE INTO ModuleStatusTypes (`ModuleStatusID`, `ModuleStatusName`) values (4, 'Defect');
INSERT IGNORE INTO ModuleStatusTypes (`ModuleStatusID`, `ModuleStatusName`) values (5, 'Uninstalled');
INSERT IGNORE INTO ModuleStatusTypes (`ModuleStatusID`, `ModuleStatusName`) values (6, 'In Repairs');

-- there should be a geographic location that marks invalid value
INSERT IGNORE INTO GeographicLocations (`LocationID`, `LocationName`) values (1, ' ');

-- MFA types
INSERT IGNORE INTO MFATypes values (1, 'No MFA used'); -- passw authentication
INSERT IGNORE INTO MFATypes values (2, 'Email MFA');
INSERT IGNORE INTO MFATypes values (3, 'SMS MFA');



