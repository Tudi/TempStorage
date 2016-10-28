Sections :
	- Description
	- Usage
	
===============================================================================	
Description	:
Upgrading from Visual Studio 2010 to VS2015 introduces a problem, when unloading DLLs at some point a memory corruption ocures. 
Becase DLL loading and unloading you are very limited in calling DLL functions ( Some DLLs might got unloaded already. Some DLLs are thread locked.. )
you are very limted in debugging. You might nbeed to implement some functions on static buffers.
The most important step in fixing this issue is understanding the issue.

My guess is that when upgrading VS, the CRT libs also get upgraded for some DLLs. NOT ALL !. Having multiple versions of CRT means you have multiple versions of memory managers.
Different versions of memory managers handle memory allocation and disposal differently. You can no longer safely presume the generic message passing architecture functions.

For example ( just example ) CRT ver 1 and CRT ver 2 uses a memory pool. If you pass a memory block from DLL1 to DLL2, if you deallocate it in DLL2 it might get inserted in CRT 2, 
when you return execution to DLL1, it might also get inserted into memory pool. At next allocation, this same location might get allocated 2 times. 
! This is not a real example, DLLs have their own context and allocation should happen within. The idea that 2 allocators work in parallel can give birth to many unexpected situation.

What i tried to do is create a DLL that hooks allocations. When this DLL gets loaded, it will iterate through all loaded DLLs and trampoline allocation. It will also create a hook
for all newly loaded DLLs to trampoline them also.
Once an allocation happens, i try to make sure it's a unique region and it has not been deallocated multiple times.
!! This did not help me catch the issue. 

The situation should be described differently, should be investigated "differently". At deallocation we realize a region of memeory got "corrupted". Why ? Multiple allocation on same region ?
Overlaping memory regions ? Multiple descructor calls on same object ? Did some destructor write out of bounds ? Did some function overwrite the memory block without any allocator issue ?
Is it an alligment issue ? Is it a bad calling convention difference issue ? Is it a header change we forcely included in Empower that should have been included from VS ? 
Is it some external DLL that is causing the issue ? Could we try to unload DLLs one by 1 and check if we can avoid the issue ? Maybe some external DLL is using some outdated calling convention
or parameters ?............


===============================================================================	
Usage	:
Include the code into one of the Empower projects. 
At some point call "HookAllDllEntryPoints" function to list all loaded DLLs. While using Empower you might encounter delayed DLL loads !
At some point call "setupHeapProfiling" function to trampoline allocations and deallocations. Might need to work on the code that checks that no region overlapping happens. Fencing is correct....
