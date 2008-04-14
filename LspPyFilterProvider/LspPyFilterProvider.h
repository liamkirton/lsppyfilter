// ========================================================================================================================
// LspPyFilterProvider
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// LspPyFilterProvider.h
//
// Created: 03/05/2007
// ========================================================================================================================

#pragma once

// ========================================================================================================================

// {3F520527-48F3-44d5-9C94-451C5F08DB28}
static const GUID g_cProviderGuid = {0x3f520527, 0x48f3, 0x44d5, {0x9c, 0x94, 0x45, 0x1c, 0x5f, 0x8, 0xdb, 0x28}};

// ========================================================================================================================

typedef struct _WSPPROC_TABLE_EXT
{
    LPFN_ACCEPTEX lpfnAcceptEx;
    LPFN_TRANSMITFILE lpfnTransmitFile;
    LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;
    LPFN_TRANSMITPACKETS lpfnTransmitPackets;
    LPFN_CONNECTEX lpfnConnectEx;
    LPFN_DISCONNECTEX lpfnDisconnectEx;
    LPFN_WSARECVMSG lpfnWSARecvMsg;
} WSPPROC_TABLE_EXT;

// ------------------------------------------------------------------------------------------------------------------------

typedef struct _PROVIDER
{
	DWORD dwChainCatalogEntryId;
	WSAPROTOCOL_INFOW WsaProtocolInfo;
	
	HMODULE hModule;
	DWORD StartupCount;
	LPWSPSTARTUP fpWSPStartup;
	WSPPROC_TABLE NextProcTable;
	WSPPROC_TABLE_EXT NextProcTableEx;

	CRITICAL_SECTION ProviderCriticalSection;
	LIST_ENTRY SocketList;
} PROVIDER;

// ------------------------------------------------------------------------------------------------------------------------

typedef struct _SOCKET_CONTEXT
{
	SOCKET Socket;
	PROVIDER *Provider;

	LIST_ENTRY Link;

	std::queue<BSTR> *ReceiveBufferQueue;
} SOCKET_CONTEXT;

// ------------------------------------------------------------------------------------------------------------------------

typedef struct _COM_CALL
{
	unsigned short Type;
	
	unsigned int Thread;
	unsigned int Socket;

	unsigned int Ip;
	unsigned short Port;

	BSTR *InputBuffer;
	BSTR *OutputBuffer;
	unsigned int Remaining;

	HRESULT hResult;
} COM_CALL;

// ------------------------------------------------------------------------------------------------------------------------

typedef enum _COM_CALL_TYPE
{
	ConnectFilter,
	DisconnectFilter,
	ReceiveFilter,
	SendFilter
} COM_CALL_TYPE;

// ========================================================================================================================
