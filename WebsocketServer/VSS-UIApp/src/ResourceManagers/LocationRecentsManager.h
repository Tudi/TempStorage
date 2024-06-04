#pragma once

// keeps track of recently viewed locations
// able to save / load to local storage the recents lists
// if shown as visible, it will show buttons on top of the screen

#define MAX_HISTORY_RETENTION_COUNT 5
#define RECENT_LOCATION_CONFIG_NAME_NAME "RecentLocation_Name_"
#define RECENT_LOCATION_CONFIG_ID_NAME "RecentLocation_Id_"
#define RECENT_LOCATION_CONFIG_STAMP_NAME "RecentLocation_Stamp_"
#define RECENT_LOCATION_CONFIG_DESCRIPTION_NAME "RecentLocation_Desc_"

typedef struct LocationHystoryData
{
	char name[MAX_DB_STRING_LENGTH];
	char description[MAX_DB_STRING_LENGTH];
	int id;
	uint64_t m_dLastViewedStamp;
}LocationHystoryData;

#include "Util/ObjDescriptor.h"

class LocationRecents
{
public:
	inline static LocationRecents& getInstance() {
#ifdef _DEBUG
		static LocationRecents* instance = new LocationRecents;
		return *instance;
#else
		static LocationRecents instance;
		return instance;
#endif
	}

	void OnLocationView(const char* LocationName, const char* LocationDescription, int id);
	const LocationHystoryData *GetHistoryData(int index);
	void OnUserLogin(); // load the recents data
	int GetLastSessionLocationId() { return m_dLastSessionLocationId; }
	void OnLocationUpdated(int id, const char *new_name);
private:
	LocationRecents();
	LocationRecents(const LocationRecents&) = delete;
	LocationRecents& operator=(const LocationRecents&) = delete;

	static void AsyncTask_OnUserLogin(void* params);

	struct LocationViewAsyncParams
	{
		const char* LocationName;
		const char* LocationDescription;
		int id;
	};
	LocationViewAsyncParams m_LocationViewAsyncParams;
	static void AsyncTask_OnLocationView(void* params);

	void SaveData(int SpecificId = -1);
	void UpdateButtonTexts();
	LocationHystoryData m_LocationHistory[MAX_HISTORY_RETENTION_COUNT];
	// to avoid buttons flickering, once they are show, they will not swap location
	LocationHystoryData m_LocationHistoryShown[MAX_HISTORY_RETENTION_COUNT];
	int m_dLastSessionLocationId;
};

#define sLocationRecentManager LocationRecents::getInstance()