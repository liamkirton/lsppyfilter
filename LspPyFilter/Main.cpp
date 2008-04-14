// ========================================================================================================================
// LspPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// Main.cpp
//
// Created: 15/05/2007
// ========================================================================================================================

#include <windows.h>

#include <iostream>

#include "../LspPyFilterMarshal/ILspPyFilter_h.h"
#include "../LspPyFilterMarshal/ILspPyFilter_i.c"
#include "ClassFactory.h"
#include "Guid.h"
#include "PyInstance.h"

// ========================================================================================================================

BOOL WINAPI ConsoleCtrlHandlerRoutine(DWORD dwCtrlType);

// ========================================================================================================================

const char *c_LspPyFilterVersion = "0.2.1";

// ========================================================================================================================

ClassFactory g_ClassFactory(CLSID_LspPyFilter);

CRITICAL_SECTION g_ConsoleCriticalSection;
HANDLE g_hExitEvent = NULL;

std::string g_Path;

// ========================================================================================================================

int main(int argc, char *argv[])
{
	std::cout << std::endl
			  << "LspPyFilter " << c_LspPyFilterVersion << std::endl
			  << "Copyright \xB8" << "2007 Liam Kirton <liam@int3.ws>" << std::endl << std::endl
			  << "Built at " << __TIME__ << " on " << __DATE__ << std::endl << std::endl;

	if(argc != 2)
	{
		std::cout << "Usage: LspPyFilter.exe <Filter.py>" << std::endl;
		return -1;
	}
	else
	{
		g_Path = argv[1];
	}

	PyInstance::GetInstance()->Load(g_Path);

	if(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
	{
		std::cout << "Error: CoInitializeEx() Failed." << std::endl;
		return -1;
	}

	InitializeCriticalSection(&g_ConsoleCriticalSection);

	try
	{
		if((g_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		{
			throw std::exception("CreateEvent() Failed.");
		}

		DWORD dwRegister;
		if(FAILED(CoRegisterClassObject(CLSID_LspPyFilter,
										&g_ClassFactory,
										CLSCTX_LOCAL_SERVER,
										REGCLS_MULTIPLEUSE,
										&dwRegister)))
		{
			throw std::exception("CoRegisterClassObject() Failed.");
		}

		EnterCriticalSection(&g_ConsoleCriticalSection);
		std::cout << "Running. Press Ctrl+Break to Reload, Ctrl+C to Quit." << std::endl << std::endl;
		LeaveCriticalSection(&g_ConsoleCriticalSection);

		SetConsoleCtrlHandler(ConsoleCtrlHandlerRoutine, TRUE);
		WaitForSingleObject(g_hExitEvent, INFINITE);
		SetConsoleCtrlHandler(ConsoleCtrlHandlerRoutine, FALSE);

		if(FAILED(CoRevokeClassObject(dwRegister)))
		{
			throw std::exception("CoRevokeClassObject() Failed.");
		}
	}
	catch(const std::exception &e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	if(g_hExitEvent != NULL)
	{
		CloseHandle(g_hExitEvent);
		g_hExitEvent = NULL;
	}

	DeleteCriticalSection(&g_ConsoleCriticalSection);

	CoUninitialize();
}

// ========================================================================================================================

BOOL WINAPI ConsoleCtrlHandlerRoutine(DWORD dwCtrlType)
{
	if(dwCtrlType == CTRL_BREAK_EVENT)
	{
		PyInstance::GetInstance()->Unload();
		PyInstance::GetInstance()->Load(g_Path);
	}
	else
	{
		if(g_hExitEvent != NULL)
		{
			EnterCriticalSection(&g_ConsoleCriticalSection);
			std::cout << "Closing." << std::endl;
			LeaveCriticalSection(&g_ConsoleCriticalSection);

			SetEvent(g_hExitEvent);
		}
	}
	return TRUE;
}

// ========================================================================================================================
