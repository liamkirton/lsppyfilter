// ========================================================================================================================
// LspPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// ClassFactory.h
//
// Created: 15/05/2007
// ========================================================================================================================

#pragma once

// ========================================================================================================================

#include <windows.h>

// ========================================================================================================================

class ClassFactory : public IClassFactory
{
public:
	ClassFactory(CLSID clsid);
	virtual ~ClassFactory();
	
	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();

	STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
	STDMETHODIMP LockServer(BOOL fLock);

private:
	CLSID clsid_;
	DWORD dwObjRefCount_;
};

// ========================================================================================================================
