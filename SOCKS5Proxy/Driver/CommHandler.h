#pragma once

/*********************************************
* Listen to external commands and execute them
*********************************************/

enum ExternalCommandTypes
{
	ECT_StartRedirecting,
	ECT_StopRedirecting,
	ECT_DropConnections,
	ECT_LoadConfigFile,
	ECT_ExitDriver,
	ECT_DropStartConnections,
	ECT_GetLicenseStatus,
};

void StartCommandsListener();