


#include <cstdlib>
#include <cstring>
#include <iostream>
#include <tbb/scalable_allocator.h>


#include "ext.h"

namespace {
	Ext extension;
};

#ifdef __GNUC__
	// Code for GNU C compiler
	static void __attribute__((constructor))
	extension_init(void)
	{
		std::cout << "extDB shared library init" << std::endl;
	}

	static void __attribute__((destructor))
	extension_destroy(void)
	{
		std::cout << "extDB shared library destroy" << std::endl;
	}

	extern "C"
	{
	  void RVExtension(char *output, int outputSize, const char *function); 
	};

	void RVExtension(char *output, int outputSize, const char *function)
	{
	  extension.callExtenion(output, outputSize, function);
	};

#elif _MSC_VER
	// Code for MSVC compiler
	#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
	#include <windows.h>

	BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
	{
		switch (ul_reason_for_call)
		{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
		}
		return TRUE;
	}

	extern "C"
	{
	  __declspec(dllexport) void __stdcall RVExtension(char *output, int outputSize, const char *function); 
	};

	void __stdcall RVExtension(char *output, int outputSize, const char *function)
	{
	  extension.callExtenion(output,outputSize,function);
	};

#elif __MINGW32__
// Code for MINGW32 compiler
// Someone figure this out thanks...
#endif
