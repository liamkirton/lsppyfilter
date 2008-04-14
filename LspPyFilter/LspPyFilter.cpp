// ========================================================================================================================
// LspPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// LspPyFilter.cpp
//
// Created: 15/05/2007
// ========================================================================================================================

#include "LspPyFilter.h"

#include <iostream>

#include "Guid.h"
#include "PyInstance.h"

// ========================================================================================================================

LspPyFilter::LspPyFilter() : dwObjRefCount_(1)
{
	
}

// ========================================================================================================================

LspPyFilter::~LspPyFilter()
{
	
}

// ========================================================================================================================

STDMETHODIMP LspPyFilter::QueryInterface(REFIID iid, void **ppvObject)
{
	*ppvObject = NULL;
	HRESULT hResult = E_NOINTERFACE;

	if(IsEqualIID(iid, IID_IUnknown))
	{
		*ppvObject = this;
	}
	else if(IsEqualIID(iid, IID_ILspPyFilter))
	{
		*ppvObject = dynamic_cast<ILspPyFilter *>(this);
	}
	
	if(*ppvObject != NULL)
	{
		AddRef();
		hResult = S_OK;
	}
	return hResult;
}

// ========================================================================================================================

STDMETHODIMP_(DWORD) LspPyFilter::AddRef()
{
	return ++dwObjRefCount_;
}

// ========================================================================================================================

STDMETHODIMP_(DWORD) LspPyFilter::Release()
{
	if(--dwObjRefCount_ == 0)
	{
		delete this;
		return 0;
	}
	return dwObjRefCount_;
}

// ========================================================================================================================

STDMETHODIMP LspPyFilter::ConnectFilter(unsigned int process,
										unsigned int thread,
										unsigned int socket,
										unsigned int *ip,
										unsigned short *port)
{
	PyInstance *pyInstance = PyInstance::GetInstance();
	if(pyInstance != NULL)
	{
		pyInstance->ConnectFilter(process, thread, socket, ip, port);
	}
	else
	{
		std::cout << "Error: PyInstance::GetInstance() Failed." << std::endl;
	}
	return S_OK;
}

// ========================================================================================================================

STDMETHODIMP LspPyFilter::DisconnectFilter(unsigned int process, unsigned int thread, unsigned int socket)
{
	PyInstance *pyInstance = PyInstance::GetInstance();
	if(pyInstance != NULL)
	{
		pyInstance->DisconnectFilter(process, thread, socket);
	}
	else
	{
		std::cout << "Error: PyInstance::GetInstance() Failed." << std::endl;
	}
	return S_OK;
}

// ========================================================================================================================

STDMETHODIMP LspPyFilter::RecvFilter(unsigned int process,
									 unsigned int thread,
									 unsigned int socket,
									 BSTR *recvBuffer,
									 BSTR *modifiedRecvBuffer,
									 unsigned int *remaining)
{
	PyInstance *pyInstance = PyInstance::GetInstance();
	if(pyInstance != NULL)
	{
		pyInstance->RecvFilter(process, thread, socket, recvBuffer, modifiedRecvBuffer, remaining);
	}
	else
	{
		std::cout << "Error: PyInstance::GetInstance() Failed." << std::endl;
	}
	return S_OK;
}

// ========================================================================================================================

STDMETHODIMP LspPyFilter::SendFilter(unsigned int process,
									 unsigned int thread,
									 unsigned int socket,
									 BSTR *sendBuffer,
									 BSTR *modifiedSendBuffer)
{
	PyInstance *pyInstance = PyInstance::GetInstance();
	if(pyInstance != NULL)
	{
		pyInstance->SendFilter(process, thread, socket, sendBuffer, modifiedSendBuffer);
	}
	else
	{
		std::cout << "Error: PyInstance::GetInstance() Failed." << std::endl;
	}
	return S_OK;
}

// ========================================================================================================================
