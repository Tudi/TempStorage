#pragma once

/*
* Purpose of this module is to gather data from Modules
* Incomming data needs to be "cached" so it can be broadcasted to multiple clients ( UI , DP )
* Some clients might not support the same data rate as the source. Interpolation is needed in this case
* There will be modules that might also want to process the feed. Custom alerts. AI data processing
* VSS-DPS should handle alert lifecycle management
*/

#include <string>
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include "Network/VSSPacketDefines.h"
#include "Util/ObjDescriptor.h"
#include "Util/VectRW.h"
#include "Network/VSSPacketDefines.h"

// Modules should send data to connected applications
// Right now it's not clear what the format or the data will be
// It is expected to send a list of object locations periodically
// Some Modules might also send alerts
typedef struct DSModuleData
{
	//UDP packet size is limited. Not expecting lots of objects to be detected
#define MAX_STACK_ALLOC_MODULE_OBJECTS 20
	DSModuleData() 
	{ 
		ObjCount = 0;
		InitTypeInfo(); 
	}
	REFLECT_TYPE(DSModuleData);
	typedef struct ObjectDetails {
		unsigned short ObjectId;
		float x, y;
//		std::string tags;
	}ObjectDetails;
	//	std::vector<ObjectDetails> Objects; // todo : maybe use direct stack alloc for single object ?
	inline ObjectDetails &emplace_back()
	{
//		return Objects.emplace_back();
		return Objects[ObjCount++];
	}
	inline void reserve(size_t count)
	{
//		Objects.reserve(count);
		assert(count < MAX_STACK_ALLOC_MODULE_OBJECTS);
	}
	inline size_t size()
	{
//		return Objects.size();
		return ObjCount;
	}
	// use noinit stack alloc instead heap alloc
	__int64 ModuleID;
	unsigned __int64 Timestamp; // when the data was captured by the module
	__int64 ObjCount;
	ObjectDetails Objects[MAX_STACK_ALLOC_MODULE_OBJECTS];
	// todo : rest of data to be added
}DSModuleData;

typedef struct DSModuleAlert
{
	DSModuleAlert() { InitTypeInfo(); }

	REFLECT_TYPE(DSModuleAlert);
	__int64 AlertDefinitionId;
	__int64 AlertId; // will be obtained by creating a DB row
	VSS_N_ModuleAlertState::AlertTypes AlertType;
	unsigned __int64 TriggerStamp;
	VSS_N_ModuleAlertState::AlertStateFlags StateFlags; // by default this is "created" status
	__int64 LocationId; // will be fetched from DB ( or cache ) once an alert arrives from a Module
	__int64 ModuleID;
	__int64 OrganizationId; // should come from scripts that trigger alert detection
	__int64 UserId; // should come from scripts that trigger alert detection
}DSModuleAlert;

/// <summary>
/// When a module sends data to DS, DS will notify all subscribers
/// </summary>
typedef void (*DS_CB_OnDataArrived)(void* CBData, void* UserData1, void* UserData2);

class VSSDSManager
{
public:
	inline static VSSDSManager& getInstance()
	{
		static VSSDSManager instance;
		return instance;
	}
	~VSSDSManager();
	void DestructorCheckMemLeaks();
	// start worker threads. Get the list of modules to manage
	void Init(__int64 DPS_Id);
	// Happens on load rebalancing. Should flush old module connections and create new ones
	void ReInit();
	// Connect to this module (also) and listen to it's feed
	void AddModuleConnectionInfo(const __int64 ModuleId, const std::string &ConnectionURL, const int AdminStatus);
	// Network sessions will subscribe to feed
	void SubscribeToModule(VSS_N_SubscribeModules::SubscriptionType SubType, const __int64 ModuleId, const DS_CB_OnDataArrived CB_func, void* UserData1, void* UserData2);
	// if is a MUST to unsubscribe before a network session gets destroyed
	void UnSubscribeFromModule(VSS_N_SubscribeModules::SubscriptionType SubType, const __int64 ModuleId, const DS_CB_OnDataArrived CB_func, void* UserData1, void* UserData2);
	// unsubscribe everything from this network session
	void UnSunscribeAll(void* UserData1);
	// Data packet arrived from a module. Relay it to all subscribers
	void OnModuleFeedArrived(const __int64 InstanceId, DSModuleData *md);
	// Alert packet arrived from module. Relay it to all subscribers
	void OnModuleAlertArrived(const __int64 InstanceId, DSModuleAlert *ma);
	// probably Email or SMS got sent
	void OnModuleAlertUpdate(DSModuleAlert* ma);
private:
	VSSDSManager();
	VSSDSManager(const VSSDSManager&) = delete;
	VSSDSManager& operator=(const VSSDSManager&) = delete;

	struct ModuleConnectionData
	{
		__int64 InstanceId;
		std::string ConnectionURL;
		int Status; // is it even worth fetching offline modules ?
	};
	// DS will try to connect to these
	std::vector<ModuleConnectionData> m_ModuleConnectionData;

	typedef struct ModuleSubscriptionData
	{
		ModuleSubscriptionData() {}; // no need to init, we should override data all the time
		DS_CB_OnDataArrived CB_func;
		void* UserData1;
		void* UserData2;
	}ModuleSubscriptionData;
	// when DS receives data from a module, will proxy the data to all subscribers
	RWLockedContainer<char>		m_MutexListLock; // avoid some thread deleting while others read it
	std::unordered_map<__int64, std::vector<ModuleSubscriptionData>> m_ModuleSubscriptionsAlert;
	std::unordered_map<__int64, std::vector<ModuleSubscriptionData>> m_ModuleSubscriptionsFeed;
};

#define sDSManager VSSDSManager::getInstance()