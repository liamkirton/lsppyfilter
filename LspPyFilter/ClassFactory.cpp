// ========================================================================================================================
// LspPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// ClassFactory.cpp
//
// Created: 15/05/2007
// ========================================================================================================================

#include "ClassFactory.h"

#include <iostream>

#include "Guid.h"
#include "LspPyFilter.h"

// ========================================================================================================================

ClassFactory::ClassFactory(CLSID clsid) : clsid_(clsid),
										  dwObjRefCount_(1)
{
	
}

// ========================================================================================================================

ClassFactory::~ClassFactory()
{
	
}

// ========================================================================================================================

STDMETHODIMP ClassFactory::QueryInterface(REFIID iid, void **ppvObject)
{
	*ppvObject = NULL;
	HRESULT hResult = E_NOINTERFACE;

	if(IsEqualIID(iid, IID_IUnknown))
	{
		*ppvObject = this;
	}
	else if(IsEqualIID(iid, IID_IClassFactory))
	{
		*ppvObject = dynamic_cast<IClassFactory *>(this);
	}
	
	if(*ppvObject != NULL)
	{
		AddRef();
		hResult = S_OK;
	}
	return hResult;
}

// ========================================================================================================================

STDMETHODIMP_(DWORD) ClassFactory::AddRef()
{
	return ++dwObjRefCount_;
}

// ========================================================================================================================

STDMETHODIMP_(DWORD) ClassFactory::Release()
{
	if(--dwObjRefCount_ == 0)
	{
		delete this;
		return 0;
	}
	return dwObjRefCount_;
}

// ========================================================================================================================

STDMETHODIMP ClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
	HRESULT hResult = E_FAIL;
	
	*ppvObject = NULL;

	if(pUnkOuter != NULL)
	{
		hResult = CLASS_E_NOAGGREGATION;
	}
	else
	{
		if(IsEqualCLSID(clsid_, CLSID_LspPyFilter))
		{
			LspPyFilter *lspPyFilter = new LspPyFilter();
			hResult = lspPyFilter->QueryInterface(riid, ppvObject);
			lspPyFilter->Release();
		}
	}
	
	return hResult;
}

// ========================================================================================================================

STDMETHODIMP ClassFactory::LockServer(BOOL fLock)
{
	return S_OK;
}

// ========================================================================================================================
