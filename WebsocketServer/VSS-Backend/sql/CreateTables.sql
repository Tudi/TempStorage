-- Organization info. Should be able to contact a specific company by a sales agent in case new offers arise. 
CREATE TABLE Organizations (
    OrganizationID INT PRIMARY KEY AUTO_INCREMENT,
    Name VARCHAR(255) COLLATE utf8mb4_general_ci,
    Email VARCHAR(255) COLLATE utf8mb4_general_ci,
    Address VARCHAR(512) COLLATE utf8mb4_general_ci,
    Phone1 VARCHAR(20),
    Phone1Name VARCHAR(255) COLLATE utf8mb4_general_ci,
    Phone2 VARCHAR(20),
    Phone2Name VARCHAR(255) COLLATE utf8mb4_general_ci,
    DirectorName VARCHAR(255) COLLATE utf8mb4_general_ci,
    CreatedTimestamp timestamp DEFAULT CURRENT_TIMESTAMP
);

-- Used by cleanup scripts. Needed for logs to be able to access Organization details even if they got deleted
CREATE TABLE Organizations_Deleted AS
SELECT * FROM Organizations
WHERE 1 = 0; 

-- Used by UI to list available ID-string pairs
CREATE TABLE IF NOT EXISTS UserRoleDefines (
    RoleID INT PRIMARY KEY AUTO_INCREMENT,
    RoleName VARCHAR(50) UNIQUE COLLATE utf8mb4_general_ci,
    RoleDescription VARCHAR(512)
);

-- Used by UI to list available ID-string pairs
CREATE TABLE IF NOT EXISTS UserActiveStates (
    UserActiveID INT PRIMARY KEY AUTO_INCREMENT,
    UserActiveName VARCHAR(50) UNIQUE COLLATE utf8mb4_general_ci
);

-- MFA types
CREATE TABLE IF NOT EXISTS MFATypes (
    MFATypeID INT PRIMARY KEY AUTO_INCREMENT,
    MFATypeName VARCHAR(50) UNIQUE COLLATE utf8mb4_general_ci
);

-- Store details about users. Should be able to contact them individually, without going through organization
CREATE TABLE IF NOT EXISTS Users (
    UserID INT PRIMARY KEY AUTO_INCREMENT,
    SerialNumber INT UNIQUE,
    RoleID INT,
    Username VARCHAR(255) UNIQUE COLLATE utf8mb4_general_ci,
    FirstName VARCHAR(255) COLLATE utf8mb4_general_ci,
    LastName VARCHAR(255) COLLATE utf8mb4_general_ci,
    Password VARCHAR(255),
    Photo VARCHAR(255) COLLATE utf8mb4_general_ci,
    DateOfBirth timestamp,
    JobRole VARCHAR(255) COLLATE utf8mb4_general_ci,
    OrganizationID INT,
    Email VARCHAR(255) COLLATE utf8mb4_general_ci,
    RecoveryEmail VARCHAR(255) UNIQUE COLLATE utf8mb4_general_ci,
    OfficePhoneNumber VARCHAR(20),
    MobilePhoneNumber VARCHAR(20),
    UserCreatedTimestamp timestamp DEFAULT CURRENT_TIMESTAMP,
	TimezoneSeconds INT,
    UserActiveID INT DEFAULT 1,
    PreferedRadarVisualizationMode INT DEFAULT 0,
    RadarVisualizationInterval INT DEFAULT 0,
	ValidUntilTimestamp timestamp DEFAULT 0, -- limited time user. Probably just field engineers
	MFATypeID INT DEFAULT 1,
    
    FOREIGN KEY (RoleID) REFERENCES UserRoleDefines(RoleID), 
    FOREIGN KEY (UserActiveID) REFERENCES UserActiveStates(UserActiveID), 
    FOREIGN KEY (OrganizationID) REFERENCES Organizations(OrganizationID),
    FOREIGN KEY (MFATypeID) REFERENCES MFATypes(MFATypeID)
);

-- Used by cleanup scripts. Needed for logs to be able to access User details even if they got deleted
CREATE TABLE Users_Deleted AS
SELECT * FROM Users
WHERE 1 = 0; 

-- Password reset requests will be stored here. Once a user actually confirms the reset request, the queue entry gets consumed
CREATE TABLE IF NOT EXISTS PsswRstQueue (
    ResetID INT PRIMARY KEY AUTO_INCREMENT,
	Email VARCHAR(255),
	UserID INT,
	CallbackId varchar(20), -- randomizer
	NewPassw varchar(20),
	IsConsumed TINYINT DEFAULT 0,
    ReqCreatedTimestamp timestamp DEFAULT CURRENT_TIMESTAMP, -- requests expire after a while
    FOREIGN KEY (UserID) REFERENCES Users(UserID)
);

-- Used by UI to list available ID-string pairs
-- A specific User Role will have rights assigned to it. Ex : view other users from same organization
CREATE TABLE IF NOT EXISTS RoleRightDefines (
    RightID INT PRIMARY KEY AUTO_INCREMENT,
    RightName VARCHAR(50) UNIQUE COLLATE utf8mb4_general_ci,
    RightDescription VARCHAR(512)
);

-- match the role with the right
CREATE TABLE IF NOT EXISTS RoleRights (
    RoleID INT,
    RightID INT,
    PRIMARY KEY (RoleID, RightID),
    FOREIGN KEY (RoleID) REFERENCES UserRoleDefines(RoleID),
    FOREIGN KEY (RightID) REFERENCES RoleRightDefines(RightID)
);

-- Used by UI to list available ID-string pairs
CREATE TABLE IF NOT EXISTS ApiEndpointDefines (
    EndpointDefineID INT PRIMARY KEY AUTO_INCREMENT,
    EndpointDefineName VARCHAR(50) UNIQUE COLLATE utf8mb4_general_ci,
    EndpointDefineDescription VARCHAR(512)
);

-- Specific role will be able access only these specific Endpoints. Ex : log in, change passw ...
CREATE TABLE IF NOT EXISTS RoleEndpointRights (
    RoleID INT,
    EndpointDefineID INT,
    PRIMARY KEY (RoleID, EndpointDefineID),
    FOREIGN KEY (RoleID) REFERENCES UserRoleDefines(RoleID),
    FOREIGN KEY (EndpointDefineID) REFERENCES ApiEndpointDefines(EndpointDefineID)
);

-- It's a dedicated table to be able to share session on multiple devices
-- Based on licensing, some user accounts might have limited session settings
CREATE TABLE IF NOT EXISTS UserSessions (
    SessionID BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT, -- not going to use autoincrement
    UserID INT UNIQUE,
	SessionSalt INT DEFAULT 1, -- used to evolve session ID over time to not be able to steal it
    SessionIpAddress VARCHAR(255),
    SessionCreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    MFAType INT DEFAULT 0,	-- notUsed(0), issued(1), confirmed(3)
    MFAToken VARCHAR(255),
    MFAExpires TIMESTAMP,
    FOREIGN KEY (UserID) REFERENCES Users(UserID)
);

-- Even if user is not logged in, there is a chance to obtain an array of users for a specific endpoint
-- It is highly expected for 1 endpoint to have only 1 UserId
-- Can generate statistics Which users is active on how many computers ( account sharing )
-- Can generate statistics if some computer is using multiple accounts ( strange )
CREATE TABLE IF NOT EXISTS UserClientEndpoints (
    ClientEndpoint VARCHAR(512),
    UserId INT,
    ClientIP VARCHAR(64),
	CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE INDEX ClientEndpoints (ClientEndpoint, UserId, ClientIP)
);

-- When a user accesses a specific API endpoint, a new entry will be created here
-- Only valid for logged in users
CREATE TABLE IF NOT EXISTS UserAPIActivity (
    ActivityID INT PRIMARY KEY AUTO_INCREMENT,
    UserID INT,
    LastUsedAPIStamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ApiUrl VARCHAR(512),
    ApiParams TEXT,
	IsGuessedUserId TINYINT DEFAULT 0,
    FOREIGN KEY (UserID) REFERENCES Users(UserID) 
);

CREATE TABLE IF NOT EXISTS Logs (
    LogID INT PRIMARY KEY AUTO_INCREMENT,
	UserID INT,	-- can be 0 if the Log was generated by a non authenticated user
	LogSourceGroup INT, -- UI / Backend / Radar ....
	LogSource INT, -- within the group, might have a sub group specification
	LogSeverityID INT, -- KPI/Debug/Normal/Unexpected/Sever/Critical
	LogClientStamp TIMESTAMP, -- generated by the client
	LogServerStamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, -- in case the API call executed unexpected large amount. Should be couple hundred ms
	LogDetails TEXT,
	LogFilterData1 INT,
	LogFilterData2 FLOAT,	
	LogFilterData3 VARCHAR(50) -- example radar with tag XXX
);

CREATE TABLE IF NOT EXISTS LogSourceGroupDefines (
    LogSourceGroupID INT PRIMARY KEY AUTO_INCREMENT,
    LogSourceGroupName VARCHAR(50) UNIQUE,
    LogSourceGroupDescription VARCHAR(512)
);

CREATE TABLE IF NOT EXISTS LogSeverityDefines (
    LogSeverityID INT PRIMARY KEY AUTO_INCREMENT,
    LogSeverityName VARCHAR(50) UNIQUE,
    LogSeverityDescription VARCHAR(512)
);

CREATE TABLE IF NOT EXISTS ConfigValues (
    ConfigValueID INT PRIMARY KEY AUTO_INCREMENT,
    ConfigValueName VARCHAR(50) UNIQUE,
    ConfigValueDescription VARCHAR(512),
    ConfigValue VARCHAR(512) UNIQUE,
	PushToClients TINYINT DEFAULT 0
);

-- Places that we could show on google map. Might contain more than 1 radars
CREATE TABLE IF NOT EXISTS GeographicLocations (
    LocationID INT PRIMARY KEY AUTO_INCREMENT,
    LocationName VARCHAR(50) UNIQUE,
    LocationDescription VARCHAR(512),
    LocationAddressLine1 VARCHAR(512),
    LocationAddressLine2 VARCHAR(512),
    LocationCity VARCHAR(512),
    LocationState VARCHAR(512),
    LocationCountyOrRegion VARCHAR(512),
    LocationCountry VARCHAR(512),
    LocationX DECIMAL(10, 6), -- probably longitude
    LocationY DECIMAL(10, 6), -- probably latitude
    LocationZ DECIMAL(10, 6), -- probably elevation
    LocationSize FLOAT, -- If we intend to cover a larger zone
	IsDeleted TINYINT DEFAULT 0,
	FOREIGN KEY (OrganizationID) REFERENCES Organizations(OrganizationID)
);

-- Trigger on movement/object/people count/proximity ...
-- Influences what script is run to trigger an alarm instance
-- when a script wants to trigger an alarm instance, it will go to AlertDefintion table to see who is subscribed to this alert type
CREATE TABLE IF NOT EXISTS AlertType (
    AlertTypeId INT PRIMARY KEY AUTO_INCREMENT,
    AlertTypeName VARCHAR(50) UNIQUE,
    AlertTypeDescription VARCHAR(512)
);

-- send mail, SMS, shut down atm ...
CREATE TABLE IF NOT EXISTS AlertActionType (
    AlertActionTypeId INT PRIMARY KEY AUTO_INCREMENT,
    AlertActionTypeName VARCHAR(50) UNIQUE,
    AlertActionTypeDescription VARCHAR(512)
);

-- based on definitions, alert instances will be created
CREATE TABLE IF NOT EXISTS AlertDefintion (
    AlertDefinitionId INT PRIMARY KEY AUTO_INCREMENT,
    AlertDefinitionName VARCHAR(50) UNIQUE,
    AlertDefinitionDescription VARCHAR(512),
    AlertTypeId INT,
    AlertTypeTresholdMin FLOAT,
    AlertTypeTresholdMax FLOAT,
    CooldownSeconds INT default 600,
    AlertActionTypeId INT,
    AlertEmail VARCHAR(50) UNIQUE,
    AlertMobilePhoneNumber VARCHAR(50) UNIQUE,
    ScriptId INT,
    OwnerUserId INT,
    OwnerOrganizationId INT,
	Disabled TINYINT default 0,
	FOREIGN KEY (AlertTypeId) REFERENCES AlertType(AlertTypeId),
	FOREIGN KEY (AlertActionTypeId) REFERENCES AlertActionType(AlertActionTypeId),
	FOREIGN KEY (OwnerOrganizationId) REFERENCES Organizations(OrganizationId),
	FOREIGN KEY (OwnerUserId) REFERENCES Users(UserId)
);

-- alert triggered/sent email/sent sms/confirmed by operator
CREATE TABLE IF NOT EXISTS AlertStatusType (
    AlertStatusTypeId INT PRIMARY KEY AUTO_INCREMENT,
    AlertStatusTypeName VARCHAR(150) UNIQUE,
    AlertStatusTypeDescription VARCHAR(512)
);

-- alert instances. Used for tracking and assesing an alert
CREATE TABLE IF NOT EXISTS Alerts (
    AlertId INT PRIMARY KEY AUTO_INCREMENT,
    AlertDefinitionId INT,
    AlertStatusTypeId INT,
    CreateTreshold FLOAT,
    CreatedTimestamp timestamp DEFAULT CURRENT_TIMESTAMP,
    LocationId INT,
    ModuleId INT,
    OwnerOrganizationId INT,
    OwnerUserId INT,
	FOREIGN KEY (AlertDefinitionId) REFERENCES AlertDefintion(AlertDefinitionId),
	FOREIGN KEY (ModuleId) REFERENCES ModuleInstances(ModuleInstanceID),
	FOREIGN KEY (AlertStatusTypeId) REFERENCES AlertStatusType(AlertStatusTypeId),
	FOREIGN KEY (OwnerOrganizationId) REFERENCES Organizations(OrganizationId),
	FOREIGN KEY (OwnerUserId) REFERENCES Users(UserId)
);

CREATE TABLE Alerts_Deleted AS
SELECT * FROM Alerts
WHERE 1 = 0; 

CREATE TABLE IF NOT EXISTS ModuleStatusTypes (
    ModuleStatusID INT PRIMARY KEY AUTO_INCREMENT,
    ModuleStatusName VARCHAR(50) UNIQUE COLLATE utf8mb4_general_ci,
    ModuleStatusDescription VARCHAR(512)
);

CREATE TABLE IF NOT EXISTS ModuleTypes (
    ModuleTypeID INT PRIMARY KEY AUTO_INCREMENT,
    ModuleTypeName VARCHAR(50) UNIQUE COLLATE utf8mb4_general_ci,
    ModuleTypeDescription VARCHAR(512)
);

-- Used by UI to list available ID-string pairs
-- These might contain a set of radars ?
CREATE TABLE IF NOT EXISTS ModuleDefines (
    ModuleDefineID INT PRIMARY KEY AUTO_INCREMENT,
    ModuleName VARCHAR(50) UNIQUE,
	ModuleTypeID INT,
    ModuleDeveloper VARCHAR(512),
    ModuleDescription VARCHAR(512),
    ModuleSerialNumber VARCHAR(512),
    ModuleVersionNumber VARCHAR(512),
    ModuleTag VARCHAR(512),
    ModuleSubscriptionPrice FLOAT, -- price in USD
	ModuleExposedScriptFields VARCHAR(2000), -- JSON will be sent to subscribers when a module sends new data
	ModuleUpdateFrequency FLOAT, -- number of updates Max to be received by subscribers
    FOREIGN KEY (ModuleTypeID) REFERENCES ModuleTypes(ModuleTypeID),
    FOREIGN KEY (ModuleStatusID) REFERENCES ModuleStatusTypes(ModuleStatusID),
);

CREATE TABLE IF NOT EXISTS ModuleInstances (
    ModuleInstanceID INT PRIMARY KEY AUTO_INCREMENT,
	ModuleDefineID INT,
	ModuleStatusID INT DEFAULT 1, -- online
	ModuleIP VARCHAR(256), -- how we can connect to this instance ?
	ModuleLocationID INT,
    EstimatedOnlineStamp timestamp DEFAULT CURRENT_TIMESTAMP, -- if for some reason it's offline. Maybe it's known when it will come online
	IsDeleted TINYINT DEFAULT 0,
    FOREIGN KEY (ModuleStatusID) REFERENCES ModuleStatusTypes(ModuleStatusID),
    FOREIGN KEY (ModuleDefineID) REFERENCES ModuleDefines(ModuleDefineID),
    FOREIGN KEY (ModuleLocationID) REFERENCES GeographicLocations(LocationId)
);

-- which organization is able to access which modules
-- it's in a separate table so the same module could be sold to more than 1 organization
CREATE TABLE IF NOT EXISTS OrganizationModules (
    OrganizationID INT,
    ModuleInstanceID INT,
    PRIMARY KEY (OrganizationID, ModuleInstanceID),
    FOREIGN KEY (OrganizationID) REFERENCES Organizations(OrganizationID),
    FOREIGN KEY (ModuleInstanceID) REFERENCES ModuleInstances(ModuleInstanceID)
);

CREATE TABLE IF NOT EXISTS KPIData (
	ClientEndpoint VARCHAR(256) PRIMARY KEY, 
	FPS_MIN FLOAT,
	FPS_AVG FLOAT,
	FPS_MAX FLOAT,
	Lag_AVG FLOAT,
	APIDUR_AVG FLOAT,
	OnlineTime BIGINT
);

CREATE TABLE IF NOT EXISTS CrashDumps (
	CrashDumpId int PRIMARY KEY AUTO_INCREMENT,
	ClientEndpoint VARCHAR(256), 
	BuildHash VARCHAR(512), 
	CallStack VARCHAR(15000)
);

CREATE TABLE IF NOT EXISTS DPS_Instances (
	DPSID int PRIMARY KEY,-- when a DPS comes online he will create row for itself. Specified to the DPS when it got started up
	ConnectionURL VARCHAR(256), -- sent to UI clients to connect to it. Session management will be shared with UI
	StartupStamp timestamp DEFAULT CURRENT_TIMESTAMP, 
	LastHeartbeat timestamp DEFAULT CURRENT_TIMESTAMP, -- tick updated every X seconds to know if this shard is online 
	CPUAvg FLOAT, -- resets when the instance restarts
	CPUPeak FLOAT, -- resets when the instance restarts
	BytesPerMinIn int, -- resets when the instance restarts
	BytesPerMinOut int, -- resets when the instance restarts
	PacketsPerMinIn int, -- resets when the instance restarts
	PacketsPerMinOut int, -- resets when the instance restarts
	BytesIn int,
	BytesOut int,
	PacketsIn int,
	PacketsOut int,
	WSClients int,
	PID int,
	MemUsage int,
	Threads int -- resets when the instance restarts
);

-- load balancer should manage this table. Assign Modules to data processing servers
-- right now there is no redundancy mechanism implemented. Secundary or backup DPS servers for modules
-- In theory client could connect to redundant DPS servers and filter redundant data just by event IDs
CREATE TABLE IF NOT EXISTS DPS_Modules (
    DPSID INT,
    ModuleInstanceID INT,
    PRIMARY KEY (DPSID, ModuleInstanceID)
);

SHOW WARNINGS;
