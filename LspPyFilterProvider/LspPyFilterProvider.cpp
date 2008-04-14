// ========================================================================================================================
// LspPyFilterProvider
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// LspPyFilterProvider.cpp
//
// Created: 03/05/2007
// ========================================================================================================================

#include <winsock2.h>
#include <windows.h>

#include <mstcpip.h>
#include <mswsock.h>
#include <ws2spi.h>
#include <ws2tcpip.h>

#include <queue>

#include "List.h"

#include "LspPyFilterProvider.h"

#include "../LspPyFilter/Guid.h"
#include "../LspPyFilterMarshal/ILspPyFilter_i.c"
#include "../LspPyFilterMarshal/ILspPyFilter_h.h"

// ========================================================================================================================

int WSPAPI WSPStartup(WORD wVersion,
					  LPWSPDATA lpWSPData,
					  LPWSAPROTOCOL_INFOW lpProtocolInfo,
					  WSPUPCALLTABLE UpCallTable,
					  LPWSPPROC_TABLE lpProcTable);

// ------------------------------------------------------------------------------------------------------------------------

int WSPAPI WSPCleanup(LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

SOCKET WSPAPI WSPAccept(SOCKET s,
						sockaddr* addr,
						LPINT addrlen,
						LPCONDITIONPROC lpfnCondition,
						DWORD dwCallbackData,
						LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

int WSPAPI WSPCloseSocket(SOCKET s, LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

int WSPAPI WSPConnect(SOCKET s,
					  const sockaddr* name,
					  int namelen,
					  LPWSABUF lpCallerData,
					  LPWSABUF lpCalleeData,
					  LPQOS lpSQOS,
					  LPQOS lpGQOS,
					  LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

BOOL PASCAL ExtConnectEx(SOCKET s,
						 const sockaddr *name,
						 int namelen,
						 PVOID lpSendBuffer,
						 DWORD dwSendDataLength,
						 LPDWORD lpdwBytesSent,
						 LPOVERLAPPED lpOverlapped);

// ------------------------------------------------------------------------------------------------------------------------

SOCKET WSPAPI WSPSocket(int af,
						int type,
						int protocol,
						LPWSAPROTOCOL_INFO lpProtocolInfo,
						GROUP g,
						DWORD dwFlags,
						LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

int WSPAPI WSPGetSockOpt(SOCKET s,
						 int level,
						 int optname,
						 char* optval,
						 LPINT optlen,
						 LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

int WSPAPI WSPIoctl(SOCKET s,
					DWORD dwIoControlCode,
					LPVOID lpvInBuffer,
					DWORD cbInBuffer,
					LPVOID lpvOutBuffer,
					DWORD cbOutBuffer,
					LPDWORD lpcbBytesReturned,
					LPWSAOVERLAPPED lpOverlapped,
					LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
					LPWSATHREADID lpThreadId,
					LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

int WSPAPI WSPRecv(SOCKET s,
				   LPWSABUF lpBuffers,
				   DWORD dwBufferCount,
				   LPDWORD lpNumberOfBytesRecvd,
				   LPDWORD lpFlags,
				   LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
				   LPWSATHREADID lpThreadId,
				   LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

int WSPAPI WSPRecvFrom(SOCKET s,
					   LPWSABUF lpBuffers,
					   DWORD dwBufferCount,
					   LPDWORD lpNumberOfBytesRecvd,
					   LPDWORD lpFlags,
					   sockaddr *lpFrom,
					   LPINT lpFromlen,
					   LPWSAOVERLAPPED lpOverlapped,
					   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
					   LPWSATHREADID lpThreadId,
					   LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

int WSPAPI WSPSend(SOCKET s,
				   LPWSABUF lpBuffers,
				   DWORD dwBufferCount,
				   LPDWORD lpNumberOfBytesSent,
				   DWORD dwFlags,
				   LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
				   LPWSATHREADID lpThreadId,
				   LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

int WSPAPI WSPSendTo(SOCKET s,
					 LPWSABUF lpBuffers,
					 DWORD dwBufferCount,
					 LPDWORD lpNumberOfBytesSent,
					 DWORD dwFlags,
					 const sockaddr* lpTo,
					 int iTolen,
					 LPWSAOVERLAPPED lpOverlapped,
					 LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
					 LPWSATHREADID lpThreadId,
					 LPINT lpErrno);

// ------------------------------------------------------------------------------------------------------------------------

DWORD WINAPI ComCallThreadProc(LPVOID lpParameter);

// ========================================================================================================================

CRITICAL_SECTION g_LspCriticalSection;
DWORD g_LspStartupCount = 0;

WSPUPCALLTABLE g_UpCallTable;

HANDLE g_ComCallThread = NULL;
HANDLE g_ComCallThreadSignalEvent = NULL;
HANDLE g_ComCallThreadCompletionEvent = NULL;
HANDLE g_ComCallThreadMutex = NULL;
HANDLE g_ComCallThreadExitEvent = NULL;

bool g_ComCallAvailable = false;
COM_CALL *g_ComCallParameter = NULL;

DWORD g_ProviderCount = 0;
PROVIDER *g_Providers = NULL;
PROVIDER *g_LspProvider = NULL;

// ========================================================================================================================

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	BOOL bResult = TRUE;
	int errNo = 0;

	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			__try
			{
				InitializeCriticalSection(&g_LspCriticalSection);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				bResult = FALSE;
			}
			break;
		
		case DLL_THREAD_ATTACH:
			break;
		
		case DLL_THREAD_DETACH:
			break;
		
		case DLL_PROCESS_DETACH:
			__try
			{
				while(WSPCleanup(&errNo) != SOCKET_ERROR);
				DeleteCriticalSection(&g_LspCriticalSection);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				bResult = FALSE;
			}
			break;

		default:
			break;
	}

	return bResult;
}

// ========================================================================================================================

void WSPAPI GetLspGuid(LPGUID lpGuid)
{
	RtlCopyMemory(lpGuid, &g_cProviderGuid, sizeof(GUID));
}

// ========================================================================================================================

int WSPAPI WSPStartup(WORD wVersion,
					  LPWSPDATA lpWSPData,
					  LPWSAPROTOCOL_INFOW lpProtocolInfo,
					  WSPUPCALLTABLE UpCallTable,
					  LPWSPPROC_TABLE lpProcTable)
{
	int iResult = WSAEPROVIDERFAILEDINIT;
	EnterCriticalSection(&g_LspCriticalSection);

	DWORD dwProtocolInfoBufferLength = 0;
	DWORD dwProtocolInfoCount = 0;
	LPWSAPROTOCOL_INFOW lpProtocolInfoBuffer = NULL;
	int errNo = 0;

	try
	{
		if(g_LspStartupCount++ == 0)
		{
			if((WSCEnumProtocols(NULL, lpProtocolInfoBuffer, &dwProtocolInfoBufferLength, &errNo) == SOCKET_ERROR) &&
			   (errNo == WSAENOBUFS))
			{
				lpProtocolInfoBuffer = reinterpret_cast<LPWSAPROTOCOL_INFOW>(new char[dwProtocolInfoBufferLength]);
				SecureZeroMemory(lpProtocolInfoBuffer, dwProtocolInfoBufferLength);

				if((dwProtocolInfoCount = WSCEnumProtocols(NULL,
														   lpProtocolInfoBuffer,
														   &dwProtocolInfoBufferLength,
														   &errNo)) == SOCKET_ERROR)
				{
					throw L"WSCEnumProtocols(lpProtocolBuffer) Failed.";
				}
			}
			else
			{
				throw L"WSCEnumProtocols(NULL) Failed.";
			}

			if((g_ComCallThreadSignalEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
			{
				throw L"CreateEvent() Failed.";
			}

			if((g_ComCallThreadCompletionEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
			{
				throw L"CreateEvent() Failed.";
			}

			if((g_ComCallThreadMutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
			{
				throw L"CreateEvent() Failed.";
			}

			if((g_ComCallThreadExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
			{
				throw L"CreateEvent() Failed.";
			}

			if((g_ComCallThread = CreateThread(NULL, 0, ComCallThreadProc, NULL, 0, NULL)) == NULL)
			{
				throw L"CreateThread() Failed.";
			}

			if(WaitForSingleObject(g_ComCallThreadCompletionEvent, 2500) != WAIT_OBJECT_0)
			{
				OutputDebugString(L"<LspPyFilterProvider::WSPStartup()> WARNING: WaitForSingleObject(g_ComCallThreadCompletionEvent) Failed.\n");
			}

			g_ProviderCount = 0;
			g_Providers = new PROVIDER[dwProtocolInfoCount];
			SecureZeroMemory(g_Providers, sizeof(PROVIDER) * g_ProviderCount);

			PROVIDER *lpProvider = &g_Providers[0];
			for(DWORD i = 0; i < dwProtocolInfoCount; ++i)
			{
				if(RtlEqualMemory(&lpProtocolInfoBuffer[i].ProviderId, &g_cProviderGuid, sizeof(GUID)))
				{
					RtlCopyMemory(&lpProvider->WsaProtocolInfo, &lpProtocolInfoBuffer[i], sizeof(WSAPROTOCOL_INFOW));
					g_LspProvider = lpProvider++;
					g_ProviderCount++;
					break;
				}
			}

			for(DWORD i = 0; i < dwProtocolInfoCount; ++i)
			{
				InitializeCriticalSection(&g_Providers[i].ProviderCriticalSection);
				InitializeListHead(&g_Providers[i].SocketList);
				g_Providers[i].StartupCount = 0;

				for(DWORD j = 0; j < static_cast<DWORD>(lpProtocolInfoBuffer[i].ProtocolChain.ChainLen); ++j)
				{
					if(lpProtocolInfoBuffer[i].ProtocolChain.ChainEntries[j] ==
					   g_LspProvider->WsaProtocolInfo.dwCatalogEntryId)
					{
						for(DWORD k = 0; k < dwProtocolInfoCount; ++k)
						{
							if(lpProtocolInfoBuffer[k].dwCatalogEntryId ==
							   lpProtocolInfoBuffer[i].ProtocolChain.ChainEntries[j + 1])
							{
								RtlCopyMemory(&lpProvider->WsaProtocolInfo,
											  &lpProtocolInfoBuffer[k],
											  sizeof(WSAPROTOCOL_INFOW));
								lpProvider->dwChainCatalogEntryId = lpProtocolInfoBuffer[i].dwCatalogEntryId;
								lpProvider++;
								g_ProviderCount++;
								break;
							}
						}
					}
				}
			}

			RtlCopyMemory(&g_UpCallTable, &UpCallTable, sizeof(WSPUPCALLTABLE));
		}

		for(DWORD i = 0; i < g_ProviderCount; ++i)
		{
			if(g_Providers[i].dwChainCatalogEntryId == lpProtocolInfo->dwCatalogEntryId)
			{
				if(g_Providers[i].StartupCount++ == 0)
				{
					int iProviderPathLen = MAX_PATH;
					wchar_t wszProviderPath[MAX_PATH];
					wchar_t wszExpandedProviderPath[MAX_PATH];

					if(WSCGetProviderPath(&g_Providers[i].WsaProtocolInfo.ProviderId,
										  reinterpret_cast<wchar_t *>(&wszProviderPath),
										  &iProviderPathLen,
										  &errNo) == SOCKET_ERROR)
					{
						throw L"WSCGetProviderPath() Failed.";
					}

					if(ExpandEnvironmentStrings(reinterpret_cast<wchar_t *>(&wszProviderPath),
												reinterpret_cast<wchar_t *>(&wszExpandedProviderPath),
												MAX_PATH) == 0)
					{
						throw L"ExpandEnvironmentStrings() Failed.";
					}

					if((g_Providers[i].hModule = LoadLibrary(wszExpandedProviderPath)) == NULL)
					{
						throw L"LoadLibrary() Failed.";
					}

					g_Providers[i].fpWSPStartup = reinterpret_cast<LPWSPSTARTUP>(GetProcAddress(g_Providers[i].hModule,
																								"WSPStartup"));
					if(g_Providers[i].fpWSPStartup == NULL)
					{
						throw L"GetProcAddress(\"WSPStartup\") Failed.";
					}
				}

				if((iResult = g_Providers[i].fpWSPStartup(wVersion,
														  lpWSPData,
														  lpProtocolInfo,
														  UpCallTable,
														  lpProcTable)) == NO_ERROR)
				{
					RtlCopyMemory(&g_Providers[i].NextProcTable, lpProcTable, sizeof(WSPPROC_TABLE));

					lpProcTable->lpWSPCleanup = WSPCleanup;

					lpProcTable->lpWSPAccept = WSPAccept;
					lpProcTable->lpWSPConnect = WSPConnect;
					lpProcTable->lpWSPCloseSocket = WSPCloseSocket;
					lpProcTable->lpWSPSocket = WSPSocket;

					lpProcTable->lpWSPGetSockOpt = WSPGetSockOpt;
					lpProcTable->lpWSPIoctl = WSPIoctl;
					
					lpProcTable->lpWSPRecv = WSPRecv;
					lpProcTable->lpWSPRecvFrom = WSPRecvFrom;
					lpProcTable->lpWSPSend = WSPSend;
					lpProcTable->lpWSPSendTo = WSPSendTo;
				}

				break;
			}
		}
	}
	catch(const wchar_t *e)
	{
		iResult = WSAEPROVIDERFAILEDINIT;

		OutputDebugString(L"<LspPyFilterProvider::WSPStartup()> Fatal Error: ");
		OutputDebugString(e);
		OutputDebugString(L"\n");
	}

	if(lpProtocolInfoBuffer != NULL)
	{
		delete [] lpProtocolInfoBuffer;
		lpProtocolInfoBuffer = NULL;
	}

	LeaveCriticalSection(&g_LspCriticalSection);
	return iResult;
}

// ========================================================================================================================

int WSPAPI WSPCleanup(LPINT lpErrno)
{
	if(g_LspStartupCount == 0)
	{
		*lpErrno = WSANOTINITIALISED;
		return SOCKET_ERROR;
	}
	
	int iResult = NO_ERROR;
	EnterCriticalSection(&g_LspCriticalSection);
	
	if(--g_LspStartupCount == 0)
	{
		if(g_ComCallThreadExitEvent != NULL)
		{
			SetEvent(g_ComCallThreadExitEvent);
			if(g_ComCallThread != NULL)
			{
				if(WaitForSingleObject(g_ComCallThread, 5000) != WAIT_OBJECT_0)
				{
					OutputDebugString(L"<LspPyFilterProvider::WSPCleanup()> WARNING: WaitForSingleObject(g_ComCallThread) Failed.\n");
				}
				CloseHandle(g_ComCallThread);
				g_ComCallThread = NULL;
			}
			CloseHandle(g_ComCallThreadExitEvent);
			g_ComCallThreadExitEvent = NULL;
		}

		if(g_ComCallThreadSignalEvent != NULL)
		{
			CloseHandle(g_ComCallThreadSignalEvent);
			g_ComCallThreadSignalEvent = NULL;
		}

		if(g_ComCallThreadCompletionEvent != NULL)
		{
			CloseHandle(g_ComCallThreadCompletionEvent);
			g_ComCallThreadCompletionEvent = NULL;
		}

		if(g_ComCallThreadMutex != NULL)
		{
			CloseHandle(g_ComCallThreadMutex);
			g_ComCallThreadMutex = NULL;
		}

		for(DWORD i = 0; i < g_ProviderCount; ++i)
		{
			while(g_Providers[i].StartupCount--)
			{
				g_Providers[i].NextProcTable.lpWSPCleanup(lpErrno);
			}

			if(g_Providers[i].hModule != NULL)
			{
				FreeLibrary(g_Providers[i].hModule);
				g_Providers[i].hModule = NULL;
			}

			while(!IsListEmpty(&g_Providers[i].SocketList))
			{
				LIST_ENTRY *lPtr = RemoveHeadList(&g_Providers[i].SocketList);
				SOCKET_CONTEXT *socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
				
				if(socketContext->ReceiveBufferQueue != NULL)
				{
					while(!socketContext->ReceiveBufferQueue->empty())
					{
						BSTR queuedRecvBuffer = socketContext->ReceiveBufferQueue->front();
						socketContext->ReceiveBufferQueue->pop();
						SysFreeString(queuedRecvBuffer);
					}
					delete socketContext->ReceiveBufferQueue;
				}
				delete socketContext;
			}
			DeleteCriticalSection(&g_Providers[i].ProviderCriticalSection);
		}

		delete [] g_Providers;
		g_ProviderCount = 0;
	}

	LeaveCriticalSection(&g_LspCriticalSection);
	return iResult;
}

// ========================================================================================================================

SOCKET WSPAPI WSPAccept(SOCKET s,
						sockaddr* addr,
						LPINT addrlen,
						LPCONDITIONPROC lpfnCondition,
						DWORD dwCallbackData,
						LPINT lpErrno)
{
	SOCKET socketResult = INVALID_SOCKET;
	SOCKET_CONTEXT *socketContext = NULL;

	for(DWORD i = 0; (i < g_ProviderCount) && (socketContext == NULL); ++i)
	{
		EnterCriticalSection(&g_Providers[i].ProviderCriticalSection);
		for(LIST_ENTRY *lPtr = g_Providers[i].SocketList.Flink; lPtr != &g_Providers[i].SocketList; lPtr = lPtr->Flink)
		{
			socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
			if(socketContext->Socket == s)
			{
				break;
			}
			socketContext = NULL;
		}
		LeaveCriticalSection(&g_Providers[i].ProviderCriticalSection);
	}

	if(socketContext == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPAccept()> WARNING: Returning WSAENOTSOCK.\n");

		*lpErrno = WSAENOTSOCK;
		return socketResult;
	}

	try
	{
		SOCKET_CONTEXT *acceptSocketContext = new SOCKET_CONTEXT;
		SecureZeroMemory(acceptSocketContext, sizeof(SOCKET_CONTEXT));
		acceptSocketContext->Provider = socketContext->Provider;
		acceptSocketContext->ReceiveBufferQueue = new std::queue<BSTR>;

		if((acceptSocketContext->Socket = socketContext->Provider->NextProcTable.lpWSPAccept(s,
																							 addr,
																							 addrlen,
																							 lpfnCondition,
																							 dwCallbackData,
																							 lpErrno)) == INVALID_SOCKET)
		{
			throw L"NextProcTable.lpWSPAccept() Failed.";
		}

		EnterCriticalSection(&acceptSocketContext->Provider->ProviderCriticalSection);
		InsertHeadList(&acceptSocketContext->Provider->SocketList, &acceptSocketContext->Link);
		LeaveCriticalSection(&acceptSocketContext->Provider->ProviderCriticalSection);

		acceptSocketContext->Socket = g_UpCallTable.lpWPUModifyIFSHandle(acceptSocketContext->Provider->dwChainCatalogEntryId,
																		 acceptSocketContext->Socket,
																		 lpErrno);
		if(acceptSocketContext->Socket == INVALID_SOCKET)
		{
			throw L"g_UpCallTable.lpWPUModifyIFSHandle() Failed.";
		}

		socketResult = acceptSocketContext->Socket;
	}
	catch(const wchar_t *e)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPAccept()> Fatal Error: ");
		OutputDebugString(e);
		OutputDebugString(L"\n");
	}

	return socketResult;
}

// ========================================================================================================================

int WSPAPI WSPCloseSocket(SOCKET s, LPINT lpErrno)
{
	int result = SOCKET_ERROR;
	SOCKET_CONTEXT *socketContext = NULL;

	for(DWORD i = 0; (i < g_ProviderCount) && (socketContext == NULL); ++i)
	{
		EnterCriticalSection(&g_Providers[i].ProviderCriticalSection);
		for(LIST_ENTRY *lPtr = g_Providers[i].SocketList.Flink; lPtr != &g_Providers[i].SocketList; lPtr = lPtr->Flink)
		{
			socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
			if(socketContext->Socket == s)
			{
				RemoveEntryList(&socketContext->Link);
				break;
			}
			socketContext = NULL;
		}
		LeaveCriticalSection(&g_Providers[i].ProviderCriticalSection);
	}

	if(socketContext == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPCloseSocket()> WARNING: Returning WSAENOTSOCK.\n");

		*lpErrno = WSAENOTSOCK;
		return result;
	}

	try
	{
		if((result = socketContext->Provider->NextProcTable.lpWSPCloseSocket(s, lpErrno)) == SOCKET_ERROR)
		{
			throw L"NextProcTable.lpWSPCloseSocket() Failed.";
		}
	}
	catch(const wchar_t *e)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPCloseSocket()> Fatal Error: ");
		OutputDebugString(e);
		OutputDebugString(L"\n");
	}

	if(g_ComCallAvailable)
	{
		COM_CALL comCall;
		comCall.Type = DisconnectFilter;
		comCall.Thread = GetCurrentThreadId();
		comCall.Socket = s;

		if(WaitForSingleObject(g_ComCallThreadMutex, INFINITE) == WAIT_OBJECT_0)
		{
			g_ComCallParameter = &comCall;

			SetEvent(g_ComCallThreadSignalEvent);

			if(WaitForSingleObject(g_ComCallThreadCompletionEvent, INFINITE) == WAIT_OBJECT_0)
			{
				if(SUCCEEDED(g_ComCallParameter->hResult))
				{
					
				}
			}

			ReleaseMutex(g_ComCallThreadMutex);
		}
	}

	if(socketContext->ReceiveBufferQueue != NULL)
	{
		while(!socketContext->ReceiveBufferQueue->empty())
		{
			BSTR queuedRecvBuffer = socketContext->ReceiveBufferQueue->front();
			socketContext->ReceiveBufferQueue->pop();
			SysFreeString(queuedRecvBuffer);
		}
		delete socketContext->ReceiveBufferQueue;
	}
	delete socketContext;
	return result;
}

// ========================================================================================================================

int WSPAPI WSPConnect(SOCKET s,
					  const sockaddr* name,
					  int namelen,
					  LPWSABUF lpCallerData,
					  LPWSABUF lpCalleeData,
					  LPQOS lpSQOS,
					  LPQOS lpGQOS,
					  LPINT lpErrno)
{
	sockaddr_in newName;
	if(namelen != sizeof(sockaddr_in))
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPConnect()> WARNING: namelen != sizeof(sockaddr_in).\n");
	}
	else
	{
		RtlCopyMemory(&newName, name, namelen);
	}

	SOCKET socketResult = INVALID_SOCKET;
	SOCKET_CONTEXT *socketContext = NULL;

	for(DWORD i = 0; (i < g_ProviderCount) && (socketContext == NULL); ++i)
	{
		EnterCriticalSection(&g_Providers[i].ProviderCriticalSection);
		for(LIST_ENTRY *lPtr = g_Providers[i].SocketList.Flink; lPtr != &g_Providers[i].SocketList; lPtr = lPtr->Flink)
		{
			socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
			if(socketContext->Socket == s)
			{
				break;
			}
			socketContext = NULL;
		}
		LeaveCriticalSection(&g_Providers[i].ProviderCriticalSection);
	}

	if(socketContext == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPConnect()> WARNING: Returning WSAENOTSOCK.\n");

		*lpErrno = WSAENOTSOCK;
		return socketResult;
	}

	if(g_ComCallAvailable)
	{
		COM_CALL comCall;
		comCall.Type = ConnectFilter;
		comCall.Thread = GetCurrentThreadId();
		comCall.Socket = s;
		comCall.Ip = ntohl(newName.sin_addr.s_addr);
		comCall.Port = ntohs(newName.sin_port);

		if(WaitForSingleObject(g_ComCallThreadMutex, INFINITE) == WAIT_OBJECT_0)
		{
			g_ComCallParameter = &comCall;

			SetEvent(g_ComCallThreadSignalEvent);

			if(WaitForSingleObject(g_ComCallThreadCompletionEvent, INFINITE) == WAIT_OBJECT_0)
			{
				if(SUCCEEDED(g_ComCallParameter->hResult))
				{
					newName.sin_addr.s_addr = htonl(comCall.Ip);
					newName.sin_port = htons(comCall.Port);
				}
			}

			ReleaseMutex(g_ComCallThreadMutex);
		}
	}

	return socketContext->Provider->NextProcTable.lpWSPConnect(s,
															   reinterpret_cast<sockaddr *>(&newName),
															   namelen,
															   lpCallerData,
															   lpCalleeData,
															   lpSQOS,
															   lpGQOS,
															   lpErrno);
}

// ========================================================================================================================

BOOL PASCAL ExtConnectEx(SOCKET s,
						 const sockaddr *name,
						 int namelen,
						 PVOID lpSendBuffer,
						 DWORD dwSendDataLength,
						 LPDWORD lpdwBytesSent,
						 LPOVERLAPPED lpOverlapped)
{
	SOCKET socketResult = INVALID_SOCKET;
	SOCKET_CONTEXT *socketContext = NULL;

	for(DWORD i = 0; (i < g_ProviderCount) && (socketContext == NULL); ++i)
	{
		EnterCriticalSection(&g_Providers[i].ProviderCriticalSection);
		for(LIST_ENTRY *lPtr = g_Providers[i].SocketList.Flink; lPtr != &g_Providers[i].SocketList; lPtr = lPtr->Flink)
		{
			socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
			if(socketContext->Socket == s)
			{
				break;
			}
			socketContext = NULL;
		}
		LeaveCriticalSection(&g_Providers[i].ProviderCriticalSection);
	}

	if(socketContext == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::ExtConnectEx()> WARNING: Returning WSAENOTSOCK.\n");
		return FALSE;
	}

	if(socketContext->Provider->NextProcTableEx.lpfnConnectEx == NULL)
	{
		GUID ConnectExGuid = WSAID_CONNECTEX;
		DWORD dwBytes;
		int errNo;
		int ioCtl = socketContext->Provider->NextProcTable.lpWSPIoctl(s,
																	  SIO_GET_EXTENSION_FUNCTION_POINTER,
																	  &ConnectExGuid,
															 		  sizeof(GUID),
															 		  &socketContext->Provider->NextProcTableEx.lpfnConnectEx,
															 		  sizeof(FARPROC),
															 		  &dwBytes,
															 		  NULL,
															 		  NULL,
															 		  NULL,
															 		  &errNo);
		if(ioCtl == SOCKET_ERROR)
		{
			OutputDebugString(L"<LspPyFilterProvider::ExtConnectEx()> WARNING: WSAIoctl(SIO_GET_EXTENSION_FUNCTION) Failed\n");
			return FALSE;
		}
	}

	return socketContext->Provider->NextProcTableEx.lpfnConnectEx(s,
																  name,
																  namelen,
																  lpSendBuffer,
																  dwSendDataLength,
																  lpdwBytesSent,
																  lpOverlapped);
}

// ========================================================================================================================

SOCKET WSPAPI WSPSocket(int af,
						int type,
						int protocol,
						LPWSAPROTOCOL_INFO lpProtocolInfo,
						GROUP g,
						DWORD dwFlags,
						LPINT lpErrno)
{
	SOCKET socketResult = INVALID_SOCKET;

	PROVIDER *lpProvider = NULL;
	for(DWORD i = 0; i < g_ProviderCount; ++i)
	{
		if(g_Providers[i].dwChainCatalogEntryId == lpProtocolInfo->dwCatalogEntryId)
		{
			lpProvider = &g_Providers[i];
			break;
		}
	}
	if(lpProvider == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPSocket()> WARNING: No lpProvider.\n");
		return socketResult;
	}
	
	try
	{
		WSAPROTOCOL_INFO passProtocolInfo;
		RtlCopyMemory(&passProtocolInfo, lpProtocolInfo, sizeof(WSAPROTOCOL_INFO));
		if(lpProvider->WsaProtocolInfo.ProtocolChain.ChainLen == BASE_PROTOCOL)
		{
			RtlCopyMemory(&passProtocolInfo, &lpProvider->WsaProtocolInfo, sizeof(WSAPROTOCOL_INFO));
		}

		SOCKET_CONTEXT *socketContext = new SOCKET_CONTEXT;
		SecureZeroMemory(socketContext, sizeof(SOCKET_CONTEXT));
		socketContext->Provider = lpProvider;
		socketContext->ReceiveBufferQueue = new std::queue<BSTR>;
		if((socketContext->Socket = lpProvider->NextProcTable.lpWSPSocket(af,
																		  type,
																		  protocol,
																		  &passProtocolInfo,
																		  g,
																		  dwFlags,
																		  lpErrno)) == INVALID_SOCKET)
		{
			throw L"NextProcTable.lpWSPSocket() Failed.";
		}

		EnterCriticalSection(&lpProvider->ProviderCriticalSection);
		InsertHeadList(&lpProvider->SocketList, &socketContext->Link);
		LeaveCriticalSection(&lpProvider->ProviderCriticalSection);

		if((socketContext->Socket = g_UpCallTable.lpWPUModifyIFSHandle(socketContext->Provider->dwChainCatalogEntryId,
																	   socketContext->Socket,
																	   lpErrno)) == INVALID_SOCKET)
		{
			throw L"g_UpCallTable.lpWPUModifyIFSHandle() Failed.";
		}

		socketResult = socketContext->Socket;
	}
	catch(const wchar_t *e)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPSocket()> Fatal Error: ");
		OutputDebugString(e);
		OutputDebugString(L"\n");
	}

	return socketResult;
}

// ========================================================================================================================

int WSPAPI WSPGetSockOpt(SOCKET s, int level, int optname, char* optval, LPINT optlen, LPINT lpErrno)
{
	SOCKET_CONTEXT *socketContext = NULL;
	for(DWORD i = 0; (i < g_ProviderCount) && (socketContext == NULL); ++i)
	{
		EnterCriticalSection(&g_Providers[i].ProviderCriticalSection);
		for(LIST_ENTRY *lPtr = g_Providers[i].SocketList.Flink; lPtr != &g_Providers[i].SocketList; lPtr = lPtr->Flink)
		{
			socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
			if(socketContext->Socket == s)
			{
				break;
			}
			socketContext = NULL;
		}
		LeaveCriticalSection(&g_Providers[i].ProviderCriticalSection);
	}

	if(socketContext == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPGetSockOpt()> WARNING: Returning WSAENOTSOCK.\n");
		*lpErrno = WSAENOTSOCK;
		return SOCKET_ERROR;
	}

	if((level == SOL_SOCKET) &&
	   ((optname == SO_PROTOCOL_INFOA) || (optname == SO_PROTOCOL_INFOW)))
	{
		switch(optname)
		{
			case SO_PROTOCOL_INFOA:
				if(*optlen < sizeof(WSAPROTOCOL_INFOA))
				{
					*optlen = sizeof(WSAPROTOCOL_INFOA);
					*lpErrno = WSAEFAULT;
					return SOCKET_ERROR;
				}

				*optlen = sizeof(WSAPROTOCOL_INFOA);
				RtlCopyMemory(optval, &socketContext->Provider->WsaProtocolInfo, sizeof(WSAPROTOCOL_INFOA));
				WideCharToMultiByte(CP_ACP,
									0,
									socketContext->Provider->WsaProtocolInfo.szProtocol,
									-1,
									reinterpret_cast<WSAPROTOCOL_INFOA *>(optval)->szProtocol,
									WSAPROTOCOL_LEN + 1,
									NULL,
									NULL);
				break;

			case SO_PROTOCOL_INFOW:
				if(*optlen < sizeof(WSAPROTOCOL_INFOW))
				{
					*optlen = sizeof(WSAPROTOCOL_INFOW);
					*lpErrno = WSAEFAULT;
					return SOCKET_ERROR;
				}

				*optlen = sizeof(WSAPROTOCOL_INFOA);
				RtlCopyMemory(optval, &socketContext->Provider->WsaProtocolInfo, sizeof(WSAPROTOCOL_INFOW));
				break;
		}
		return NO_ERROR;
	}

	return socketContext->Provider->NextProcTable.lpWSPGetSockOpt(s, level, optname, optval, optlen, lpErrno);
}

// ========================================================================================================================

int WSPAPI WSPIoctl(SOCKET s,
					DWORD dwIoControlCode,
					LPVOID lpvInBuffer,
					DWORD cbInBuffer,
					LPVOID lpvOutBuffer,
					DWORD cbOutBuffer,
					LPDWORD lpcbBytesReturned,
					LPWSAOVERLAPPED lpOverlapped,
					LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
					LPWSATHREADID lpThreadId,
					LPINT lpErrno)
{
	SOCKET_CONTEXT *socketContext = NULL;
	for(DWORD i = 0; (i < g_ProviderCount) && (socketContext == NULL); ++i)
	{
		EnterCriticalSection(&g_Providers[i].ProviderCriticalSection);
		for(LIST_ENTRY *lPtr = g_Providers[i].SocketList.Flink; lPtr != &g_Providers[i].SocketList; lPtr = lPtr->Flink)
		{
			socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
			if(socketContext->Socket == s)
			{
				break;
			}
			socketContext = NULL;
		}
		LeaveCriticalSection(&g_Providers[i].ProviderCriticalSection);

		if(socketContext != NULL)
		{
			break;
		}
	}

	if(socketContext == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPIoctl()> WARNING: Returning WSAENOTSOCK.\n");
		*lpErrno = WSAENOTSOCK;
		return SOCKET_ERROR;
	}

	if(dwIoControlCode == SIO_GET_EXTENSION_FUNCTION_POINTER)
	{
		GUID ConnectExGuid = WSAID_CONNECTEX;
		if(RtlEqualMemory(lpvInBuffer, &ConnectExGuid, sizeof(GUID)))
		{
			if(cbOutBuffer < sizeof(LPFN_CONNECTEX))
			{
				OutputDebugString(L"<LspPyFilterProvider::WSPIoctl()> WARNING: Returning WSAEFAULT.\n");
				*lpcbBytesReturned = sizeof(LPFN_CONNECTEX);
				*lpErrno = WSAEFAULT;
				return SOCKET_ERROR;
			}
			*lpcbBytesReturned = sizeof(LPFN_CONNECTEX);
			*reinterpret_cast<DWORD_PTR *>(lpvOutBuffer) = reinterpret_cast<DWORD_PTR>(ExtConnectEx);
			return NO_ERROR;
		}
	}

	return socketContext->Provider->NextProcTable.lpWSPIoctl(s,
															 dwIoControlCode,
															 lpvInBuffer,
															 cbInBuffer,
															 lpvOutBuffer,
															 cbOutBuffer,
															 lpcbBytesReturned,
															 lpOverlapped,
															 lpCompletionRoutine,
															 lpThreadId,
															 lpErrno);
}

// ========================================================================================================================

int WSPAPI WSPRecv(SOCKET s,
				   LPWSABUF lpBuffers,
				   DWORD dwBufferCount,
				   LPDWORD lpNumberOfBytesRecvd,
				   LPDWORD lpFlags,
				   LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
				   LPWSATHREADID lpThreadId,
				   LPINT lpErrno)
{
	if((lpOverlapped != NULL) || (lpCompletionRoutine != NULL))
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPRecv()> WARNING: lpOverlapped or lpCompletionRoutine Specified.\n");
	}

	SOCKET_CONTEXT *socketContext = NULL;
	for(DWORD i = 0; (i < g_ProviderCount) && (socketContext == NULL); ++i)
	{
		EnterCriticalSection(&g_Providers[i].ProviderCriticalSection);
		for(LIST_ENTRY *lPtr = g_Providers[i].SocketList.Flink; lPtr != &g_Providers[i].SocketList; lPtr = lPtr->Flink)
		{
			socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
			if(socketContext->Socket == s)
			{
				break;
			}
			socketContext = NULL;
		}
		LeaveCriticalSection(&g_Providers[i].ProviderCriticalSection);
	}

	if(socketContext == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPRecv()> WARNING: Returning WSAENOTSOCK.\n");
		*lpErrno = WSAENOTSOCK;
		return SOCKET_ERROR;
	}

	char *lpAmalgamatedReceiveBuffer = NULL;
	DWORD dwAmalgamatedReceiveBufferCount = 0;
	DWORD dwAmalgamatedReceiveBufferSize = 0;

	DWORD *lpBufferCounts = new DWORD[dwBufferCount];
	WSABUF *lpReceiveBuffers = new WSABUF[dwBufferCount];
	for(DWORD i = 0; i < dwBufferCount; ++i)
	{
		lpBufferCounts[i] = 0;
		lpReceiveBuffers[i].len = lpBuffers[i].len;
		lpReceiveBuffers[i].buf = new char[lpReceiveBuffers[i].len];
		dwAmalgamatedReceiveBufferSize += lpReceiveBuffers[i].len;
	}

	lpAmalgamatedReceiveBuffer = new char[dwAmalgamatedReceiveBufferSize];

	int wspRecvResult = 0;
	DWORD dwBytesRecvd = 0;
	DWORD dwBytesRecvdProcessed = 0;
	DWORD dwBytesRecvdRemaining = 0;

	do
	{
		dwBytesRecvd = 0;
		dwBytesRecvdProcessed = 0;
		
		wspRecvResult = socketContext->Provider->NextProcTable.lpWSPRecv(s,
																		 lpReceiveBuffers,
																		 dwBufferCount,
																		 &dwBytesRecvd,
																		 lpFlags,
																		 lpOverlapped,
																		 lpCompletionRoutine,
																		 lpThreadId,
																		 lpErrno);
		if((wspRecvResult != SOCKET_ERROR) || (dwBytesRecvd != 0))
		{
			dwBytesRecvdRemaining = 0;

			if(dwAmalgamatedReceiveBufferCount + dwBytesRecvd > dwAmalgamatedReceiveBufferSize)
			{
				dwAmalgamatedReceiveBufferSize = dwAmalgamatedReceiveBufferCount + dwBytesRecvd;

				char *tmpAmalgamatedReceiveBuffer = new char[dwAmalgamatedReceiveBufferSize];
				RtlCopyMemory(tmpAmalgamatedReceiveBuffer, lpAmalgamatedReceiveBuffer, dwAmalgamatedReceiveBufferCount);
				delete [] lpAmalgamatedReceiveBuffer;
				lpAmalgamatedReceiveBuffer = tmpAmalgamatedReceiveBuffer;
			}

			for(DWORD i = 0; (i < dwBufferCount) && (dwBytesRecvdProcessed < dwBytesRecvd); ++i)
			{
				DWORD dwCurrentReceiveBufferCount = min(dwBytesRecvd - dwBytesRecvdProcessed, lpReceiveBuffers[i].len);
				dwBytesRecvdProcessed += dwCurrentReceiveBufferCount;
				
				RtlCopyMemory(lpAmalgamatedReceiveBuffer + dwAmalgamatedReceiveBufferCount,
							  lpReceiveBuffers[i].buf,
							  dwCurrentReceiveBufferCount);
				dwAmalgamatedReceiveBufferCount += dwCurrentReceiveBufferCount;
			}

			if(dwAmalgamatedReceiveBufferCount > 0)
			{
				BSTR bstrCurrentReceiveBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(lpAmalgamatedReceiveBuffer),
																	  dwAmalgamatedReceiveBufferCount);
				BSTR bstrCurrentReceiveBufferModified = NULL;

				if(g_ComCallAvailable)
				{
					COM_CALL comCall;
					comCall.Type = ReceiveFilter;
					comCall.Thread = GetCurrentThreadId();
					comCall.Socket = s;
					comCall.InputBuffer = &bstrCurrentReceiveBuffer;
					comCall.OutputBuffer = &bstrCurrentReceiveBufferModified;

					if(WaitForSingleObject(g_ComCallThreadMutex, INFINITE) == WAIT_OBJECT_0)
					{
						g_ComCallParameter = &comCall;
						SetEvent(g_ComCallThreadSignalEvent);

						if(WaitForSingleObject(g_ComCallThreadCompletionEvent, INFINITE) == WAIT_OBJECT_0)
						{
							if(SUCCEEDED(g_ComCallParameter->hResult))
							{
								if(bstrCurrentReceiveBufferModified != NULL)
								{
									EnterCriticalSection(&socketContext->Provider->ProviderCriticalSection);
									socketContext->ReceiveBufferQueue->push(bstrCurrentReceiveBufferModified);
									LeaveCriticalSection(&socketContext->Provider->ProviderCriticalSection);

									SysFreeString(bstrCurrentReceiveBuffer);
									bstrCurrentReceiveBuffer = NULL;
								}
								else if(g_ComCallParameter->Remaining != 0)
								{
									dwBytesRecvdRemaining = g_ComCallParameter->Remaining;

									SysFreeString(bstrCurrentReceiveBuffer);
									bstrCurrentReceiveBuffer = NULL;
								}
							}
						}
						ReleaseMutex(g_ComCallThreadMutex);
					}
				}

				if(bstrCurrentReceiveBuffer != NULL)
				{
					EnterCriticalSection(&socketContext->Provider->ProviderCriticalSection);
					socketContext->ReceiveBufferQueue->push(bstrCurrentReceiveBuffer);
					LeaveCriticalSection(&socketContext->Provider->ProviderCriticalSection);
					bstrCurrentReceiveBuffer = NULL;
				}
			}
		}
		else if(*lpErrno != WSAEWOULDBLOCK)
		{
			dwBytesRecvdRemaining = 0;

			if(dwAmalgamatedReceiveBufferCount > 0)
			{
				BSTR bstrCurrentReceiveBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(lpAmalgamatedReceiveBuffer),
																	  dwAmalgamatedReceiveBufferCount);
				EnterCriticalSection(&socketContext->Provider->ProviderCriticalSection);
				socketContext->ReceiveBufferQueue->push(bstrCurrentReceiveBuffer);
				LeaveCriticalSection(&socketContext->Provider->ProviderCriticalSection);
				bstrCurrentReceiveBuffer = NULL;
			}
		}
	}
	while(dwBytesRecvdRemaining > 0);

	DWORD dwTotalBytesRecvd = 0;

	EnterCriticalSection(&socketContext->Provider->ProviderCriticalSection);
	while(!socketContext->ReceiveBufferQueue->empty())
	{
		BSTR queuedRecvBuffer = socketContext->ReceiveBufferQueue->front();
		char *lpQueuedRecvBuffer = reinterpret_cast<char *>(&(queuedRecvBuffer[0]));
		DWORD dwQueuedRecvBufferLen = SysStringByteLen(queuedRecvBuffer);
		DWORD dwQueuedRecvBufferWritten = 0;

		for(DWORD i = 0; (i < dwBufferCount) && (dwQueuedRecvBufferWritten < dwQueuedRecvBufferLen); ++i)
		{
			DWORD dwCurrentBufferFree = (lpBuffers[i].len - lpBufferCounts[i]);
			if(dwCurrentBufferFree > 0)
			{
				DWORD dwBytesToWrite = min(dwCurrentBufferFree, dwQueuedRecvBufferLen - dwQueuedRecvBufferWritten);
				RtlCopyMemory(lpBuffers[i].buf + lpBufferCounts[i],
							  lpQueuedRecvBuffer + dwQueuedRecvBufferWritten,
							  dwBytesToWrite);

				dwQueuedRecvBufferWritten += dwBytesToWrite;
				lpBufferCounts[i] += dwBytesToWrite;
				dwTotalBytesRecvd += dwBytesToWrite;
			}
		}

		if(dwQueuedRecvBufferWritten < dwQueuedRecvBufferLen)
		{
			socketContext->ReceiveBufferQueue->front() = SysAllocStringByteLen(lpQueuedRecvBuffer + dwQueuedRecvBufferWritten,
																			   dwQueuedRecvBufferLen - dwQueuedRecvBufferWritten);
			SysFreeString(queuedRecvBuffer);
			break;
		}
		else
		{
			socketContext->ReceiveBufferQueue->pop();
			SysFreeString(queuedRecvBuffer);
		}
	}
	LeaveCriticalSection(&socketContext->Provider->ProviderCriticalSection);

	for(DWORD i = 0; i < dwBufferCount; ++i)
	{
		delete [] lpReceiveBuffers[i].buf;
		lpReceiveBuffers[i].buf = NULL;
		lpReceiveBuffers[i].len = 0;
	}
	delete [] lpReceiveBuffers;
	lpReceiveBuffers = NULL;

	delete [] lpBufferCounts;
	lpBufferCounts = NULL;

	if(lpAmalgamatedReceiveBuffer != NULL)
	{
		delete [] lpAmalgamatedReceiveBuffer;
		lpAmalgamatedReceiveBuffer = NULL;
	}

	*lpNumberOfBytesRecvd = dwTotalBytesRecvd;
	if((dwTotalBytesRecvd > 0) && (wspRecvResult == SOCKET_ERROR))
	{
		wspRecvResult = 0;
	}

	return wspRecvResult;
}

// ========================================================================================================================

int WSPAPI WSPRecvFrom(SOCKET s,
					   LPWSABUF lpBuffers,
					   DWORD dwBufferCount,
					   LPDWORD lpNumberOfBytesRecvd,
					   LPDWORD lpFlags,
					   sockaddr *lpFrom,
					   LPINT lpFromlen,
					   LPWSAOVERLAPPED lpOverlapped,
					   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
					   LPWSATHREADID lpThreadId,
					   LPINT lpErrno)
{
	sockaddr_in newFrom;
	if(*lpFromlen != sizeof(sockaddr_in))
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPRecvFrom()> WARNING: lpFromlen != sizeof(sockaddr_in).\n");
	}
	else
	{
		RtlCopyMemory(&newFrom, lpFrom, *lpFromlen);
	}

	SOCKET_CONTEXT *socketContext = NULL;
	for(DWORD i = 0; (i < g_ProviderCount) && (socketContext == NULL); ++i)
	{
		EnterCriticalSection(&g_Providers[i].ProviderCriticalSection);
		for(LIST_ENTRY *lPtr = g_Providers[i].SocketList.Flink; lPtr != &g_Providers[i].SocketList; lPtr = lPtr->Flink)
		{
			socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
			if(socketContext->Socket == s)
			{
				break;
			}
			socketContext = NULL;
		}
		LeaveCriticalSection(&g_Providers[i].ProviderCriticalSection);
	}

	if(socketContext == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPRecvFrom()> WARNING: Returning WSAENOTSOCK.\n");

		*lpErrno = WSAENOTSOCK;
		return SOCKET_ERROR;
	}

	if(g_ComCallAvailable)
	{
		COM_CALL comCall;
		comCall.Type = ConnectFilter;
		comCall.Thread = GetCurrentThreadId();
		comCall.Socket = s;
		comCall.Ip = ntohl(newFrom.sin_addr.s_addr);
		comCall.Port = ntohs(newFrom.sin_port);

		if(WaitForSingleObject(g_ComCallThreadMutex, INFINITE) == WAIT_OBJECT_0)
		{
			g_ComCallParameter = &comCall;

			SetEvent(g_ComCallThreadSignalEvent);

			if(WaitForSingleObject(g_ComCallThreadCompletionEvent, INFINITE) == WAIT_OBJECT_0)
			{
				if(SUCCEEDED(g_ComCallParameter->hResult))
				{
					newFrom.sin_addr.s_addr = htonl(comCall.Ip);
					newFrom.sin_port = htons(comCall.Port);
				}
			}

			ReleaseMutex(g_ComCallThreadMutex);
		}
	}
	
	char *lpAmalgamatedReceiveBuffer = NULL;
	DWORD dwAmalgamatedReceiveBufferCount = 0;
	DWORD dwAmalgamatedReceiveBufferSize = 0;

	DWORD *lpBufferCounts = new DWORD[dwBufferCount];
	WSABUF *lpReceiveBuffers = new WSABUF[dwBufferCount];
	for(DWORD i = 0; i < dwBufferCount; ++i)
	{
		lpBufferCounts[i] = 0;
		lpReceiveBuffers[i].len = lpBuffers[i].len;
		lpReceiveBuffers[i].buf = new char[lpReceiveBuffers[i].len];
		dwAmalgamatedReceiveBufferSize += lpReceiveBuffers[i].len;
	}

	lpAmalgamatedReceiveBuffer = new char[dwAmalgamatedReceiveBufferSize];

	int wspRecvFromResult = 0;
	DWORD dwBytesRecvd = 0;
	DWORD dwBytesRecvdProcessed = 0;
	DWORD dwBytesRecvdRemaining = 0;

	do
	{
		dwBytesRecvd = 0;
		dwBytesRecvdProcessed = 0;
		
		wspRecvFromResult = socketContext->Provider->NextProcTable.lpWSPRecvFrom(s,
																				 lpReceiveBuffers,
																				 dwBufferCount,
																				 &dwBytesRecvd,
																				 lpFlags,
																				 reinterpret_cast<sockaddr *>(&newFrom),
																				 lpFromlen,
																				 lpOverlapped,
																				 lpCompletionRoutine,
																				 lpThreadId,
																				 lpErrno);
		if((wspRecvFromResult != SOCKET_ERROR) || (dwBytesRecvd != 0))
		{
			dwBytesRecvdRemaining = 0;

			if(dwAmalgamatedReceiveBufferCount + dwBytesRecvd > dwAmalgamatedReceiveBufferSize)
			{
				dwAmalgamatedReceiveBufferSize = dwAmalgamatedReceiveBufferCount + dwBytesRecvd;

				char *tmpAmalgamatedReceiveBuffer = new char[dwAmalgamatedReceiveBufferSize];
				RtlCopyMemory(tmpAmalgamatedReceiveBuffer, lpAmalgamatedReceiveBuffer, dwAmalgamatedReceiveBufferCount);
				delete [] lpAmalgamatedReceiveBuffer;
				lpAmalgamatedReceiveBuffer = tmpAmalgamatedReceiveBuffer;
			}

			for(DWORD i = 0; (i < dwBufferCount) && (dwBytesRecvdProcessed < dwBytesRecvd); ++i)
			{
				DWORD dwCurrentReceiveBufferCount = min(dwBytesRecvd - dwBytesRecvdProcessed, lpReceiveBuffers[i].len);
				dwBytesRecvdProcessed += dwCurrentReceiveBufferCount;
				
				RtlCopyMemory(lpAmalgamatedReceiveBuffer + dwAmalgamatedReceiveBufferCount,
							  lpReceiveBuffers[i].buf,
							  dwCurrentReceiveBufferCount);
				dwAmalgamatedReceiveBufferCount += dwCurrentReceiveBufferCount;
			}

			if(dwAmalgamatedReceiveBufferCount > 0)
			{
				BSTR bstrCurrentReceiveBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(lpAmalgamatedReceiveBuffer),
																	  dwAmalgamatedReceiveBufferCount);
				BSTR bstrCurrentReceiveBufferModified = NULL;

				if(g_ComCallAvailable)
				{
					COM_CALL comCall;
					comCall.Type = ReceiveFilter;
					comCall.Thread = GetCurrentThreadId();
					comCall.Socket = s;
					comCall.InputBuffer = &bstrCurrentReceiveBuffer;
					comCall.OutputBuffer = &bstrCurrentReceiveBufferModified;

					if(WaitForSingleObject(g_ComCallThreadMutex, INFINITE) == WAIT_OBJECT_0)
					{
						g_ComCallParameter = &comCall;
						SetEvent(g_ComCallThreadSignalEvent);

						if(WaitForSingleObject(g_ComCallThreadCompletionEvent, INFINITE) == WAIT_OBJECT_0)
						{
							if(SUCCEEDED(g_ComCallParameter->hResult))
							{
								if(bstrCurrentReceiveBufferModified != NULL)
								{
									EnterCriticalSection(&socketContext->Provider->ProviderCriticalSection);
									socketContext->ReceiveBufferQueue->push(bstrCurrentReceiveBufferModified);
									LeaveCriticalSection(&socketContext->Provider->ProviderCriticalSection);

									SysFreeString(bstrCurrentReceiveBuffer);
									bstrCurrentReceiveBuffer = NULL;
								}
								else if(g_ComCallParameter->Remaining != 0)
								{
									dwBytesRecvdRemaining = g_ComCallParameter->Remaining;

									SysFreeString(bstrCurrentReceiveBuffer);
									bstrCurrentReceiveBuffer = NULL;
								}
							}
						}
						ReleaseMutex(g_ComCallThreadMutex);
					}
				}

				if(bstrCurrentReceiveBuffer != NULL)
				{
					EnterCriticalSection(&socketContext->Provider->ProviderCriticalSection);
					socketContext->ReceiveBufferQueue->push(bstrCurrentReceiveBuffer);
					LeaveCriticalSection(&socketContext->Provider->ProviderCriticalSection);
					bstrCurrentReceiveBuffer = NULL;
				}
			}
		}
		else if(*lpErrno != WSAEWOULDBLOCK)
		{
			dwBytesRecvdRemaining = 0;

			if(dwAmalgamatedReceiveBufferCount > 0)
			{
				BSTR bstrCurrentReceiveBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(lpAmalgamatedReceiveBuffer),
																	  dwAmalgamatedReceiveBufferCount);
				EnterCriticalSection(&socketContext->Provider->ProviderCriticalSection);
				socketContext->ReceiveBufferQueue->push(bstrCurrentReceiveBuffer);
				LeaveCriticalSection(&socketContext->Provider->ProviderCriticalSection);
				bstrCurrentReceiveBuffer = NULL;
			}
		}
	}
	while(dwBytesRecvdRemaining > 0);

	DWORD dwTotalBytesRecvd = 0;

	EnterCriticalSection(&socketContext->Provider->ProviderCriticalSection);
	while(!socketContext->ReceiveBufferQueue->empty())
	{
		BSTR queuedRecvBuffer = socketContext->ReceiveBufferQueue->front();
		char *lpQueuedRecvBuffer = reinterpret_cast<char *>(&(queuedRecvBuffer[0]));
		DWORD dwQueuedRecvBufferLen = SysStringByteLen(queuedRecvBuffer);
		DWORD dwQueuedRecvBufferWritten = 0;

		for(DWORD i = 0; (i < dwBufferCount) && (dwQueuedRecvBufferWritten < dwQueuedRecvBufferLen); ++i)
		{
			DWORD dwCurrentBufferFree = (lpBuffers[i].len - lpBufferCounts[i]);
			if(dwCurrentBufferFree > 0)
			{
				DWORD dwBytesToWrite = min(dwCurrentBufferFree, dwQueuedRecvBufferLen - dwQueuedRecvBufferWritten);
				RtlCopyMemory(lpBuffers[i].buf + lpBufferCounts[i],
							  lpQueuedRecvBuffer + dwQueuedRecvBufferWritten,
							  dwBytesToWrite);

				dwQueuedRecvBufferWritten += dwBytesToWrite;
				lpBufferCounts[i] += dwBytesToWrite;
				dwTotalBytesRecvd += dwBytesToWrite;
			}
		}

		if(dwQueuedRecvBufferWritten < dwQueuedRecvBufferLen)
		{
			socketContext->ReceiveBufferQueue->front() = SysAllocStringByteLen(lpQueuedRecvBuffer + dwQueuedRecvBufferWritten,
																			   dwQueuedRecvBufferLen - dwQueuedRecvBufferWritten);
			SysFreeString(queuedRecvBuffer);
			break;
		}
		else
		{
			socketContext->ReceiveBufferQueue->pop();
			SysFreeString(queuedRecvBuffer);
		}
	}
	LeaveCriticalSection(&socketContext->Provider->ProviderCriticalSection);

	for(DWORD i = 0; i < dwBufferCount; ++i)
	{
		delete [] lpReceiveBuffers[i].buf;
		lpReceiveBuffers[i].buf = NULL;
		lpReceiveBuffers[i].len = 0;
	}
	delete [] lpReceiveBuffers;
	lpReceiveBuffers = NULL;

	delete [] lpBufferCounts;
	lpBufferCounts = NULL;

	if(lpAmalgamatedReceiveBuffer != NULL)
	{
		delete [] lpAmalgamatedReceiveBuffer;
		lpAmalgamatedReceiveBuffer = NULL;
	}

	*lpNumberOfBytesRecvd = dwTotalBytesRecvd;
	if((dwTotalBytesRecvd > 0) && (wspRecvFromResult == SOCKET_ERROR))
	{
		wspRecvFromResult = 0;
	}

	return wspRecvFromResult;
} 

// ========================================================================================================================

int WSPAPI WSPSend(SOCKET s,
				   LPWSABUF lpBuffers,
				   DWORD dwBufferCount,
				   LPDWORD lpNumberOfBytesSent,
				   DWORD dwFlags,
				   LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
				   LPWSATHREADID lpThreadId,
				   LPINT lpErrno)
{
	if((lpOverlapped != NULL) || (lpCompletionRoutine != NULL))
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPSend()> WARNING: lpOverlapped or lpCompletionRoutine Specified.\n");
	}

	SOCKET_CONTEXT *socketContext = NULL;
	for(DWORD i = 0; (i < g_ProviderCount) && (socketContext == NULL); ++i)
	{
		EnterCriticalSection(&g_Providers[i].ProviderCriticalSection);
		for(LIST_ENTRY *lPtr = g_Providers[i].SocketList.Flink; lPtr != &g_Providers[i].SocketList; lPtr = lPtr->Flink)
		{
			socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
			if(socketContext->Socket == s)
			{
				break;
			}
			socketContext = NULL;
		}
		LeaveCriticalSection(&g_Providers[i].ProviderCriticalSection);
	}

	if(socketContext == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPSend()> WARNING: Returning WSAENOTSOCK.\n");

		*lpErrno = WSAENOTSOCK;
		return SOCKET_ERROR;
	}

	LPWSABUF lpModifiedBuffers = NULL;
	DWORD dwReportBytesSent = 0;

	if(g_ComCallAvailable)
	{
		DWORD dwByteCount = 0;
		lpModifiedBuffers = new WSABUF[dwBufferCount];

		for(DWORD i = 0; i < dwBufferCount; ++i)
		{
			dwReportBytesSent += lpBuffers[i].len;
			BSTR sendBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(lpBuffers[i].buf), lpBuffers[i].len);
			BSTR modifiedSendBuffer = NULL;

			lpModifiedBuffers[i].len = lpBuffers[i].len;
			lpModifiedBuffers[i].buf = new char[lpModifiedBuffers[i].len];
			RtlCopyMemory(lpModifiedBuffers[i].buf, lpBuffers[i].buf, lpModifiedBuffers[i].len);

			COM_CALL comCall;
			comCall.Type = SendFilter;
			comCall.Thread = GetCurrentThreadId();
			comCall.Socket = s;
			comCall.InputBuffer = &sendBuffer;
			comCall.OutputBuffer = &modifiedSendBuffer;

			if(WaitForSingleObject(g_ComCallThreadMutex, INFINITE) == WAIT_OBJECT_0)
			{
				g_ComCallParameter = &comCall;

				SetEvent(g_ComCallThreadSignalEvent);

				if(WaitForSingleObject(g_ComCallThreadCompletionEvent, INFINITE) == WAIT_OBJECT_0)
				{
					if(SUCCEEDED(g_ComCallParameter->hResult))
					{
						if(modifiedSendBuffer != NULL)
						{
							lpModifiedBuffers[i].len = SysStringByteLen(modifiedSendBuffer);
							lpModifiedBuffers[i].buf = new char[lpModifiedBuffers[i].len];
							RtlCopyMemory(lpModifiedBuffers[i].buf,
										  reinterpret_cast<char *>(&(modifiedSendBuffer[0])),
										  lpModifiedBuffers[i].len);
							SysFreeString(modifiedSendBuffer);
						}
					}
				}

				ReleaseMutex(g_ComCallThreadMutex);
			}

			SysFreeString(sendBuffer);
		}
	}
	
	int wspSendResult = socketContext->Provider->NextProcTable.lpWSPSend(s,
																		 ((lpModifiedBuffers != NULL) ? lpModifiedBuffers : lpBuffers),
																		 dwBufferCount,
																		 lpNumberOfBytesSent,
																		 dwFlags,
																		 lpOverlapped,
																		 lpCompletionRoutine,
																		 lpThreadId,
																		 lpErrno);

	if(lpModifiedBuffers != NULL)
	{
		*lpNumberOfBytesSent = dwReportBytesSent;

		for(DWORD i = 0; i < dwBufferCount; ++i)
		{
			delete [] lpModifiedBuffers[i].buf;
			lpModifiedBuffers[i].buf = 0;
			lpModifiedBuffers[i].len = 0;
		}

		delete [] lpModifiedBuffers;
		lpModifiedBuffers = NULL;
	}

	return wspSendResult;
}

// ========================================================================================================================

int WSPAPI WSPSendTo(SOCKET s,
					 LPWSABUF lpBuffers,
					 DWORD dwBufferCount,
					 LPDWORD lpNumberOfBytesSent,
					 DWORD dwFlags,
					 const sockaddr* lpTo,
					 int iTolen,
					 LPWSAOVERLAPPED lpOverlapped,
					 LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
					 LPWSATHREADID lpThreadId,
					 LPINT lpErrno)
{
	sockaddr_in newTo;
	if(iTolen != sizeof(sockaddr_in))
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPSendTo()> - Error: iTolen != sizeof(sockaddr_in).\n");
	}
	else
	{
		RtlCopyMemory(&newTo, lpTo, iTolen);
	}

	SOCKET_CONTEXT *socketContext = NULL;
	for(DWORD i = 0; (i < g_ProviderCount) && (socketContext == NULL); ++i)
	{
		EnterCriticalSection(&g_Providers[i].ProviderCriticalSection);
		for(LIST_ENTRY *lPtr = g_Providers[i].SocketList.Flink; lPtr != &g_Providers[i].SocketList; lPtr = lPtr->Flink)
		{
			socketContext = CONTAINING_RECORD(lPtr, SOCKET_CONTEXT, Link);
			if(socketContext->Socket == s)
			{
				break;
			}
			socketContext = NULL;
		}
		LeaveCriticalSection(&g_Providers[i].ProviderCriticalSection);
	}

	if(socketContext == NULL)
	{
		OutputDebugString(L"<LspPyFilterProvider::WSPSendTo()> WARNING: Returning WSAENOTSOCK.\n");

		*lpErrno = WSAENOTSOCK;
		return SOCKET_ERROR;
	}

	if(g_ComCallAvailable)
	{
		COM_CALL comCall;
		comCall.Type = ConnectFilter;
		comCall.Thread = GetCurrentThreadId();
		comCall.Socket = s;
		comCall.Ip = ntohl(newTo.sin_addr.s_addr);
		comCall.Port = ntohs(newTo.sin_port);

		if(WaitForSingleObject(g_ComCallThreadMutex, INFINITE) == WAIT_OBJECT_0)
		{
			g_ComCallParameter = &comCall;

			SetEvent(g_ComCallThreadSignalEvent);

			if(WaitForSingleObject(g_ComCallThreadCompletionEvent, INFINITE) == WAIT_OBJECT_0)
			{
				if(SUCCEEDED(g_ComCallParameter->hResult))
				{
					newTo.sin_addr.s_addr = htonl(comCall.Ip);
					newTo.sin_port = htons(comCall.Port);
				}
			}

			ReleaseMutex(g_ComCallThreadMutex);
		}
	}

	LPWSABUF lpModifiedBuffers = NULL;
	DWORD dwReportBytesSent = 0;

	DWORD dwByteCount = 0;
	lpModifiedBuffers = new WSABUF[dwBufferCount];

	for(DWORD i = 0; i < dwBufferCount; ++i)
	{
		dwReportBytesSent += lpBuffers[i].len;
		BSTR sendBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(lpBuffers[i].buf), lpBuffers[i].len);
		BSTR modifiedSendBuffer = NULL;

		lpModifiedBuffers[i].len = lpBuffers[i].len;
		lpModifiedBuffers[i].buf = new char[lpModifiedBuffers[i].len];
		RtlCopyMemory(lpModifiedBuffers[i].buf, lpBuffers[i].buf, lpModifiedBuffers[i].len);

		COM_CALL comCall;
		comCall.Type = SendFilter;
		comCall.Thread = GetCurrentThreadId();
		comCall.Socket = s;
		comCall.InputBuffer = &sendBuffer;
		comCall.OutputBuffer = &modifiedSendBuffer;

		if(WaitForSingleObject(g_ComCallThreadMutex, INFINITE) == WAIT_OBJECT_0)
		{
			g_ComCallParameter = &comCall;

			SetEvent(g_ComCallThreadSignalEvent);

			if(WaitForSingleObject(g_ComCallThreadCompletionEvent, INFINITE) == WAIT_OBJECT_0)
			{
				if(SUCCEEDED(g_ComCallParameter->hResult))
				{
					if(modifiedSendBuffer != NULL)
					{
						lpModifiedBuffers[i].len = SysStringByteLen(modifiedSendBuffer);
						lpModifiedBuffers[i].buf = new char[lpModifiedBuffers[i].len];
						RtlCopyMemory(lpModifiedBuffers[i].buf,
									  reinterpret_cast<char *>(&(modifiedSendBuffer[0])),
									  lpModifiedBuffers[i].len);
						SysFreeString(modifiedSendBuffer);
					}
				}
			}

			ReleaseMutex(g_ComCallThreadMutex);
		}

		SysFreeString(sendBuffer);
	}
	
	int wspSendToResult = socketContext->Provider->NextProcTable.lpWSPSendTo(s,
																			 ((lpModifiedBuffers != NULL) ? lpModifiedBuffers : lpBuffers),
																			 dwBufferCount,
																			 lpNumberOfBytesSent,
																			 dwFlags,
																			 reinterpret_cast<sockaddr *>(&newTo),
																			 iTolen,
																			 lpOverlapped,
																			 lpCompletionRoutine,
																			 lpThreadId,
																			 lpErrno);
		
	if(lpModifiedBuffers != NULL)
	{
		*lpNumberOfBytesSent = dwReportBytesSent;

		for(DWORD i = 0; i < dwBufferCount; ++i)
		{
			delete [] lpModifiedBuffers[i].buf;
			lpModifiedBuffers[i].buf = 0;
			lpModifiedBuffers[i].len = 0;
		}

		delete [] lpModifiedBuffers;
		lpModifiedBuffers = NULL;
	}

	return wspSendToResult;
}

// ========================================================================================================================

DWORD WINAPI ComCallThreadProc(LPVOID lpParameter)
{
	ILspPyFilter *pLspPyFilter = NULL;

	HRESULT hInitCom = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if((hInitCom == S_OK) || (hInitCom == S_FALSE) || (hInitCom == RPC_E_CHANGED_MODE))
	{
		while(WaitForSingleObject(g_ComCallThreadExitEvent, 0) != WAIT_OBJECT_0)
		{
			IUnknown *pClassFactoryUnknown = NULL;
			if(SUCCEEDED(CoGetClassObject(CLSID_LspPyFilter,
										  CLSCTX_LOCAL_SERVER,
										  NULL,
										  IID_IUnknown,
										  reinterpret_cast<LPVOID *>(&pClassFactoryUnknown))))
			{
				IClassFactory *pClassFactory = NULL;
				if(SUCCEEDED(pClassFactoryUnknown->QueryInterface(IID_IClassFactory,
																  reinterpret_cast<LPVOID *>(&pClassFactory))))
				{
					IUnknown *pLspPyFilterUnknown = NULL;
					if(SUCCEEDED(pClassFactory->CreateInstance(NULL,
															   IID_IUnknown,
															   reinterpret_cast<LPVOID *>(&pLspPyFilterUnknown))))
					{
						if(FAILED(pLspPyFilterUnknown->QueryInterface(IID_ILspPyFilter,
																		 reinterpret_cast<LPVOID *>(&pLspPyFilter))))
						{
							OutputDebugString(L"<LspPyFilterProvider::ComCallThreadProc()> WARNING: pLspPyFilterUnknown->QueryInterface() Failed.\n");
							pLspPyFilter = NULL;
						}

						pLspPyFilterUnknown->Release();
						pLspPyFilterUnknown = NULL;
					}
					else
					{
						OutputDebugString(L"<LspPyFilterProvider::ComCallThreadProc()> WARNING: pClassFactory->CreateInstance() Failed.\n");
					}
					pClassFactory->Release();
					pClassFactory = NULL;
				}
				else
				{
					OutputDebugString(L"<LspPyFilterProvider::ComCallThreadProc()> WARNING: pClassFactoryUnknown->QueryInterface() Failed.\n");
				}
				pClassFactoryUnknown->Release();
				pClassFactoryUnknown = NULL;
			}

			SetEvent(g_ComCallThreadCompletionEvent);

			if(pLspPyFilter == NULL)
			{
				Sleep(2500);
			}
			else
			{
				ResetEvent(g_ComCallThreadCompletionEvent);
				g_ComCallAvailable = true;

				HANDLE hWaitHandles[2];
				hWaitHandles[0] = g_ComCallThreadExitEvent;
				hWaitHandles[1] = g_ComCallThreadSignalEvent;

				while(true)
				{
					bool bInCallLoop = false;
					switch(WaitForMultipleObjects(sizeof(hWaitHandles) / sizeof(HANDLE),
												  reinterpret_cast<const HANDLE *>(&hWaitHandles),
												  FALSE,
												  INFINITE))
					{
						case WAIT_OBJECT_0:
							break;

						case WAIT_OBJECT_0 + 1:
							bInCallLoop = true;
							break;

						default:
							break;
					}
					if(!bInCallLoop)
					{
						break;
					}

					g_ComCallParameter->Remaining = 0;

					switch(g_ComCallParameter->Type)
					{
						case ConnectFilter:
							g_ComCallParameter->hResult = pLspPyFilter->ConnectFilter(GetCurrentProcessId(),
																					  g_ComCallParameter->Thread,
																					  g_ComCallParameter->Socket,
																					  &g_ComCallParameter->Ip,
																					  &g_ComCallParameter->Port);
							break;

						case DisconnectFilter:
							g_ComCallParameter->hResult = pLspPyFilter->DisconnectFilter(GetCurrentProcessId(),
																						 g_ComCallParameter->Thread,
																						 g_ComCallParameter->Socket);
							break;

						case ReceiveFilter:
							g_ComCallParameter->hResult = pLspPyFilter->RecvFilter(GetCurrentProcessId(),
																				   g_ComCallParameter->Thread,
																				   g_ComCallParameter->Socket,
																				   g_ComCallParameter->InputBuffer,
																				   g_ComCallParameter->OutputBuffer,
																				   &g_ComCallParameter->Remaining);
																					
							break;

						case SendFilter:
							g_ComCallParameter->hResult = pLspPyFilter->SendFilter(GetCurrentProcessId(),
																				   g_ComCallParameter->Thread,
																				   g_ComCallParameter->Socket,
																				   g_ComCallParameter->InputBuffer,
																				   g_ComCallParameter->OutputBuffer);
							break;

						default:
							OutputDebugString(L"<LspPyFilterProvider::ComCallThreadProc()> WARNING: g_ComCallParameter->Type Unknown.\n");
							break;
					}

					if(g_ComCallParameter->hResult == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE))
					{
						g_ComCallAvailable = false;

						OutputDebugString(L"<LspPyFilterProvider::ComCallThreadProc()> WARNING: pLspPyFilter Call Returned RPC_S_SERVER_UNAVAILABLE.\n");
						SetEvent(g_ComCallThreadCompletionEvent);
						break;
					}
					else
					{
						SetEvent(g_ComCallThreadCompletionEvent);
					}
				}

				pLspPyFilter->Release();
				pLspPyFilter = NULL;				
			}
		}
	}
	else
	{
		OutputDebugString(L"<LspPyFilterProvider::ComCallThreadProc()> WARNING: CoInitializeEx() Failed.\n");
	}

	if(SUCCEEDED(hInitCom))
	{
		CoUninitialize();
	}

	return 0;
}

// ========================================================================================================================
