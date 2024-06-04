#pragma once

#if defined(WINDOWS) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
#endif

// APIENTRY is redefined in GLFW. Might need to sort that out later
#ifdef WINDOWS
	#include <windows.h>
#endif

#include <stdio.h>          
#include <stdlib.h>         
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <thread>
#include <list>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <algorithm>
#include <mutex>
#include <queue>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <cassert>
#include <cstdio>
#include <cstring>

#include "../dependencies/json/json.hpp"

#include "Session/ApplicationSession.h"
#include "UserSession.h"

#include "./Util/stdafx.h"
#include "./Licensing/stdafx.h"
#include "./CrashHandle/stdafx.h"
#include "./Web/stdafx.h"
#include "./UI/stdafx.h"
#include "./ResourceManagers/stdafx.h"
#include "ResourceManager/AsyncTaskManager.h"
