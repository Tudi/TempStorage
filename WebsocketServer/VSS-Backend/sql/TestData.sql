
-- used for testing and debugging purpuses
insert into Organizations (Name,Email,DirectorName) values ("Root authority", "nobody@nowhere", "Eric");

INSERT INTO `users` (`UserID`, `SerialNumber`, `RoleID`, `Username`, `FirstName`, `LastName`, `Password`, `Photo`, `DateOfBirth`, `JobRole`, `OrganizationID`, `Email`, `RecoveryEmail`, `OfficePhoneNumber`, `MobilePhoneNumber`, `UserCreatedTimestamp`, `TimezoneSeconds`, `UserIsActive`, `PreferedRadarVisualizationMode`, `RadarVisualizationInterval`) VALUES (9, NULL, 1, 'a', 'firstN', 'LastN', '$2y$10$xfq45IbVGGw01QdazESmFO5Gx2eRaG2lfJBa.OC9gCZW3Im.Mq2ay', 'photo', '2023-09-27 01:01:00', 'dirtdigger', 1, 'jozsab1@gmail.com', 'jozsab1@gmail.com', '123412', '1345123', '2023-09-28 15:31:29', 120, 1, NULL, NULL);

insert ignore into RoleRights values (1,1),(1,5),(1,10),(1,20),(1,30),(1,40),(1,50),(1,60),(1,70),(1,80),(1,90),(1,100),(1,110),(1,120),(1,130),(1,140),(1,150),(1,160),(1,170),(1,180),(1,190),(1,200),(1,210),(1,220),(1,230),(1,240),(1,250),(1,260),(1,270),(1,280),(1,290),(1,300),(1,310),(1,320),(1,330),(1,340),(1,350),(1,360),(1,370),(1,380),(1,400),(1,401),(1,450),(1,451);

insert ignore into RoleEndpointRights values (1,1),(1,2),(1,3),(1,4),(1,45),(1,46),(1,47),(1,66),(1,67),(1,68),(1,87),(1,112),(1,151),(1,226),(1,259),(1,323),(1,408),(1,459),(1,513),(1,551),(1,591),(1,654),(1,948),(1,1102),(1,1103),(1,1104),(1,1174),(1,1246),(1,1297),(1,1350),(1,1351);

insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("Walmart Store #54","Omaha","NE");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName1","LocationCity1","LocationState1");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName2","LocationCity2","LocationState2");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName3","LocationCity3","LocationState3");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName4","LocationCity4","LocationState4");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName5","LocationCity5","LocationState5");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName6","LocationCity6","LocationState6");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName7","LocationCity7","LocationState7");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName8","LocationCity8","LocationState8");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName9","LocationCity9","LocationState9");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName10","LocationCity10","LocationState10");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName11","LocationCity11","LocationState11");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName12","LocationCity12","LocationState12");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName13","LocationCity13","LocationState13");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName14","LocationCity14","LocationState14");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName15","LocationCity15","LocationState15");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName16","LocationCity16","LocationState16");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName17","LocationCity17","LocationState17");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName18","LocationCity18","LocationState18");
insert into GeographicLocations (LocationName, LocationCity, LocationState) values ("LocationName19","LocationCity19","LocationState19");

insert into Radar (RadarName, RadarDescription, LocationID) values ("Walm1", "North facing", 1);
insert into Radar (RadarName, RadarDescription, LocationID) values ("Walm2", "Floor 2", 1);
insert into Radar (RadarName, RadarDescription, LocationID) values ("Walm3", "Elevator", 1);

insert into AlertDefintion (AlertDefinitionId, AlertDefinitionName, AlertTypeId, AlertActionTypeId, AlertEmail, AlertMobilePhoneNumber, OwnerUserId, OwnerOrganizationId) values 
(1, "Alert person + email", 1, 1, "jozsab1@gmail.com", "+40742237492", 9, 1);
INSERT INTO `alertdefintion` (`AlertDefinitionId`, `AlertDefinitionName`, `AlertDefinitionDescription`, `AlertTypeId`, `AlertTypeTresholdMin`, `AlertTypeTresholdMax`, `CooldownSeconds`, `AlertActionTypeId`, `AlertEmail`, `AlertMobilePhoneNumber`, `ScriptId`, `OwnerUserId`, `OwnerOrganizationId`, `Disabled`) VALUES (2, 'Target approaching', NULL, 4, NULL, NULL, 600, 4, 'jozsab1@gmail.com', '+40742237492', NULL, 9, 1, 0);
INSERT INTO `alertdefintion` (`AlertDefinitionId`, `AlertDefinitionName`, `AlertDefinitionDescription`, `AlertTypeId`, `AlertTypeTresholdMin`, `AlertTypeTresholdMax`, `CooldownSeconds`, `AlertActionTypeId`, `AlertEmail`, `AlertMobilePhoneNumber`, `ScriptId`, `OwnerUserId`, `OwnerOrganizationId`, `Disabled`) VALUES (3, 'Target close', NULL, 5, NULL, NULL, 600, 4, 'jozsab1@gmail.com', '+40742237492', NULL, 9, 1, 0);
INSERT INTO `alertdefintion` (`AlertDefinitionId`, `AlertDefinitionName`, `AlertDefinitionDescription`, `AlertTypeId`, `AlertTypeTresholdMin`, `AlertTypeTresholdMax`, `CooldownSeconds`, `AlertActionTypeId`, `AlertEmail`, `AlertMobilePhoneNumber`, `ScriptId`, `OwnerUserId`, `OwnerOrganizationId`, `Disabled`) VALUES (4, 'Take action on approach', NULL, 6, NULL, NULL, 600, 3, 'jozsab1@gmail.com', '+40742237492', NULL, 9, 1, 0);

insert into Alerts (AlertDefinitionId, AlertStatusTypeId, LocationId, RadarId, OwnerUserId, OwnerOrganizationId) values (1, 1, 1, 1, 9, 1);

insert into ModuleTypes (ModuleTypeID,ModuleTypeName) values (1, "Radar");

insert into ModuleDefines (ModuleName, ModuleTag, ModuleLocationID) values ( "Vicinity Security Radar", "Radar 1", 1);
insert into ModuleDefines (ModuleName, ModuleTag, ModuleLocationID, ModuleStatusID) values ( "Module name 2", "DeviceName 2", 1, 2);
insert into ModuleDefines (ModuleName, ModuleTag, ModuleLocationID, ModuleStatusID) values ( "Module name 3", "DeviceName 3", 1, 3);

insert into OrganizationModules (OrganizationID, ModuleDefineID) values (1,1);
insert into OrganizationModules (OrganizationID, ModuleDefineID) values (1,2);
insert into OrganizationModules (OrganizationID, ModuleDefineID) values (1,3);



