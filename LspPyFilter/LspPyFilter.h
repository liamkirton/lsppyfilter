// ========================================================================================================================
// LspPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// LspPyFilter.h
//
// Created: 15/05/2007
// ========================================================================================================================

#pragma once

// ========================================================================================================================

#include <windows.h>

#include "../LspPyFilterMarshal/ILspPyFilter_h.h"

// ========================================================================================================================

class LspPyFilter : public ILspPyFilter
{
public:
	LspPyFilter();
	~LspPyFilter();

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();

	STDMETHODIMP ConnectFilter(unsigned int process,
							   unsigned int thread,
							   unsigned int socket,
							   unsigned int *ip,
							   unsigned short *port);

	STDMETHODIMP DisconnectFilter(unsigned int process, unsigned int thread, unsigned int socket);

	STDMETHODIMP RecvFilter(unsigned int process,
							unsigned int thread,
							unsigned int socket,
							BSTR *recvBuffer,
							BSTR *modifiedRecvBuffer,
							unsigned int *remaining);

	STDMETHODIMP SendFilter(unsigned int process,
							unsigned int thread,
							unsigned int socket,
							BSTR *sendBuffer,
							BSTR *modifiedSendBuffer);

private:
	DWORD dwObjRefCount_;
};

// ========================================================================================================================
