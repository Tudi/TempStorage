#pragma once

#include "Util/MonochromeImage.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
	#define IMGUI_VULKAN_DEBUG_REPORT
#endif

//ImVec4 clear_color; // background color for windows

enum ImGUIBackendErrorCodes 
{
	BE_NO_ERROR = 0,
	BE_GLFW_INIT_FAILED,
	BE_VULKAN_NOT_SUPPORTED,
	BE_MEM_ALLOCATION_ERROR,
	BE_FAILED_TO_CREATE_GL_WINDOW,
};

// A struct to manage data related to one image in vulkan
typedef struct VSSImageStore
{
    VkDescriptorSet DS;         // Descriptor set: this is what you'll pass to Image()
    int             Width;
    int             Height;
    int             Channels;

    // Need to keep track of these to properly cleanup
    VkImageView     ImageView;
    VkImage         Image;
    VkDeviceMemory  ImageMemory;
    VkSampler       Sampler;
    VkBuffer        UploadBuffer;
    VkDeviceMemory  UploadBufferMemory;

    MonochromeImage ImageHitmap; // if requested a monochrome image is generated from the alpha map
                                   // this will be used to check if button should register "OnClick"
    VSSImageStore() { memset(this, 0, sizeof(*this)); }
}VSSImageStore;

// creates a new thread to handle the repaint cycle until the application will be shut down
ImGUIBackendErrorCodes ImGUI_Backend_Init();
ImGUIBackendErrorCodes ImGUI_Backend_RunInfiniteFrameLoop();
ImGUIBackendErrorCodes ImGUI_Backend_Unload();

constexpr void GetDrawAreaSize(int& width, int& height, bool bDPIAdjusted);
float ImGui_CenterCursorForText(const char* txt);
VSSImageStore* ImGui_LoadTextureFromFile(const char* filename, bool bGenerateMonochromeImage = false);
void ImGui_RemoveTexture(VSSImageStore** tex_data);

// compile time function to turn a human readable string to a unique hash number
#define GetStrOfVal(y) #y
#define GetImGuiID2(x) GetImGuiID(x),x
// x is a string, add some index to it
#define GetImGuiIDStrIndex(x,y) (GetImGuiID(x)+(y))
// x is a string. Owner class is shared ( can't be used )
#define GetImGuiIDStrCounter(x) (GetImGuiID(x)+(__COUNTER__))
// x is a string and we are inside a class
#define GetImGuiIDNameCounterPointer(x) ((ImGuiID)(GetImGuiID(x)+(__COUNTER__)+(unsigned __int64)(this)))
#define GetImGuiIDNamePointer(x) ((ImGuiID)(GetImGuiID(GetStrOfVal(x))+(unsigned __int64)(x)))
#define GetImGuiIDPointer(x) ((ImGuiID)((unsigned __int64)(x)))

constexpr ImGuiID GetImGuiID(const char* str)
{
    ImGuiID hash = 0;
	if (str != NULL)
	{
	    while (*str) 
	    {
	        hash = (hash << 7) + (hash << 1) + *str++;
	    }
	}
    return hash;
}