#include "stdafx.h"
#include "ResourceManager/AsyncTaskManager.h"

LocationRecents::LocationRecents()
{
	memset(m_LocationHistory, 0, sizeof(m_LocationHistory));
	m_dLastSessionLocationId = 0;
}

void LocationRecents::UpdateButtonTexts()
{
	// cause it makes buttons jump around
	if (sWindowManager.isLocationViewWindowVisible() == true)
	{
		return;
	}

	// snapshot values so they do not swap while we view them
	memcpy(m_LocationHistoryShown, m_LocationHistory, MIN(sizeof(m_LocationHistoryShown),sizeof(m_LocationHistory)));

#define BUTTON_MAX_WIDTH 200.0f
	for (uint32_t i = 0; i < MAX_HISTORY_RETENTION_COUNT; i++)
	{
		// big hack to make buttons have the same width
		ImVec2 buttonTextSize = ImGui::CalcTextSize(m_LocationHistoryShown[i].name);
		while (buttonTextSize.x > BUTTON_MAX_WIDTH + 2 * ImGui::GetTextLineHeight())
		{
			m_LocationHistoryShown[i].name[strlen(m_LocationHistoryShown[i].name) - 1] = 0;
			buttonTextSize = ImGui::CalcTextSize(m_LocationHistoryShown[i].name);
		}
		while (buttonTextSize.x < BUTTON_MAX_WIDTH)
		{
			char TempStr[500];
			strcpy_s(TempStr, m_LocationHistoryShown[i].name);
			sprintf_s(m_LocationHistoryShown[i].name, " %s ", TempStr);
			buttonTextSize = ImGui::CalcTextSize(m_LocationHistoryShown[i].name);
		}
	}
}

void LocationRecents::AsyncTask_OnUserLogin(void* params)
{
	params;
	LocationRecents* self = &sLocationRecentManager;
	for (uint32_t i = 0; i < MAX_HISTORY_RETENTION_COUNT; i++)
	{
		char keyName[MAX_DB_STRING_LENGTH];

		sprintf_s(keyName, "%s%d", RECENT_LOCATION_CONFIG_NAME_NAME, i);
		sprintf_s(self->m_LocationHistory[i].name, sConfigManager.GetUserConfig(keyName));

		sprintf_s(keyName, "%s%d", RECENT_LOCATION_CONFIG_ID_NAME, i);
		self->m_LocationHistory[i].id = atoi(sConfigManager.GetUserConfig(keyName));

		sprintf_s(keyName, "%s%d", RECENT_LOCATION_CONFIG_STAMP_NAME, i);
		self->m_LocationHistory[i].m_dLastViewedStamp = atoll(sConfigManager.GetUserConfig(keyName));

		sprintf_s(keyName, "%s%d", RECENT_LOCATION_CONFIG_DESCRIPTION_NAME, i);
		sprintf_s(self->m_LocationHistory[i].description, sConfigManager.GetUserConfig(keyName));
	}

	self->UpdateButtonTexts();
	self->m_dLastSessionLocationId = self->m_LocationHistory[0].id;
}

void LocationRecents::OnUserLogin()
{
	AddAsyncTask(AsyncTask_OnUserLogin, NULL);
}

void LocationRecents::OnLocationView(const char* LocationName, const char* LocationDescription, int id)
{
	m_LocationViewAsyncParams.id = id;
	m_LocationViewAsyncParams.LocationName = LocationName;
	m_LocationViewAsyncParams.LocationDescription = LocationDescription;
	AddAsyncTask(AsyncTask_OnLocationView, NULL);
}

void LocationRecents::SaveData(int SpecificId)
{
	for (uint32_t i = 0; i < MAX_HISTORY_RETENTION_COUNT; i++)
	{
		// only update one specific ID. Happens on location edit
		if (SpecificId != -1 && SpecificId != m_LocationHistory[i].id)
		{
			continue;
		}
		char keyName[MAX_DB_STRING_LENGTH], keyVal[MAX_DB_STRING_LENGTH];

		sprintf_s(keyName, "%s%d", RECENT_LOCATION_CONFIG_NAME_NAME, i);
		sConfigManager.SetUserConfig(keyName, m_LocationHistory[i].name);

		sprintf_s(keyName, "%s%d", RECENT_LOCATION_CONFIG_ID_NAME, i);
		sprintf_s(keyVal, "%d", m_LocationHistory[i].id);
		sConfigManager.SetUserConfig(keyName, keyVal);

		sprintf_s(keyName, "%s%d", RECENT_LOCATION_CONFIG_STAMP_NAME, i);
		sprintf_s(keyVal, "%llu", m_LocationHistory[i].m_dLastViewedStamp);
		sConfigManager.SetUserConfig(keyName, keyVal);

		sprintf_s(keyName, "%s%d", RECENT_LOCATION_CONFIG_DESCRIPTION_NAME, i);
		sConfigManager.SetUserConfig(keyName, m_LocationHistory[i].description);
	}
}

const LocationHystoryData* LocationRecents::GetHistoryData(int index)
{
	if (index < 0 || index >= MAX_HISTORY_RETENTION_COUNT)
	{
		return NULL;
	}
	return &m_LocationHistory[index];
}

void LocationRecents::OnLocationUpdated(int id, const char* new_name)
{
	for (int i = 0; i < _countof(m_LocationHistory); i++)
	{
		if (m_LocationHistory[i].id != id)
		{
			continue;
		}
		sprintf_s(m_LocationHistory[i].name, new_name);
		SaveData(id);
		UpdateButtonTexts();
		break;
	}
}

void LocationRecents::AsyncTask_OnLocationView(void* params)
{
	params;
	LocationRecents* self = &sLocationRecentManager;

	// if we already have this id in recents list
	uint32_t isAlreadyRecent = MAX_HISTORY_RETENTION_COUNT - 1;
	for (uint32_t i = 0; i < MAX_HISTORY_RETENTION_COUNT; i++)
	{
		if (self->m_LocationHistory[i].id == self->m_LocationViewAsyncParams.id)
		{
			isAlreadyRecent = i;
			break;
		}
	}
	// shift existing data to "right"
	for (uint32_t i = isAlreadyRecent; i > 0; i--)
	{
		memcpy(&self->m_LocationHistory[i], &self->m_LocationHistory[i - 1], sizeof(LocationHystoryData));
	}

	//push recent data on location 0
	self->m_LocationHistory[0].id = self->m_LocationViewAsyncParams.id;
	strcpy_s(self->m_LocationHistory[0].name, self->m_LocationViewAsyncParams.LocationName);
	strcpy_s(self->m_LocationHistory[0].description, self->m_LocationViewAsyncParams.LocationDescription);

	// queue saves to user config
	self->SaveData();

	self->UpdateButtonTexts();
}