// ========================================================================================================================
// LspPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// PyInstance.cpp
//
// Created: 15/05/2007
// ========================================================================================================================

#include "PyInstance.h"

#include <iostream>

// ========================================================================================================================

static PyInstance g_PyInstance;

// ========================================================================================================================

static PyMethodDef LspPyFilterMethods[] =
{
	{"set_connect_filter", PyInstance::SetConnectFilter, METH_VARARGS, NULL},
	{"set_disconnect_filter", PyInstance::SetDisconnectFilter, METH_VARARGS, NULL},
    {"set_recv_filter", PyInstance::SetRecvFilter, METH_VARARGS, NULL},
	{"set_send_filter", PyInstance::SetSendFilter, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

// ========================================================================================================================

PyInstance::PyInstance() : pyConnectFilter_(NULL),
						   pyDisconnectFilter_(NULL),
						   pyRecvFilter_(NULL),
						   pySendFilter_(NULL)
{
	if((hMutex_ = CreateMutex(NULL, FALSE, NULL)) == NULL)
	{
		std::cout << "Error: CreateMutex() Failed." << std::endl;
		return;
	}
}

// ========================================================================================================================

PyInstance::~PyInstance()
{
	Unload();

	if(hMutex_ != NULL)
	{
		CloseHandle(hMutex_);
		hMutex_ = NULL;
	}
}

// ========================================================================================================================

void PyInstance::Load(const std::string &path)
{
	std::cout << "Loading \"" << path << "\"." << std::endl;

	__try
	{
		Lock();
	
		Py_Initialize();
		Py_InitModule("lsppyfilter", LspPyFilterMethods);
	
		HANDLE hPyFilter = INVALID_HANDLE_VALUE;
		if((hPyFilter = CreateFileA(path.c_str(),
								    GENERIC_READ,
								    FILE_SHARE_READ,
								    NULL,
								    OPEN_EXISTING,
								    FILE_ATTRIBUTE_NORMAL,
								    NULL)) != INVALID_HANDLE_VALUE)
		{
			HANDLE hPyFilterMapping = NULL;
			if((hPyFilterMapping = CreateFileMapping(hPyFilter,
													 NULL,
													 PAGE_READONLY,
													 0,
													 GetFileSize(hPyFilter, NULL),
													 NULL)) != NULL)
			{
				char *pPyFilter = reinterpret_cast<char *>(MapViewOfFile(hPyFilterMapping,
																		 FILE_MAP_READ,
																		 0,
																		 0,
																		 0));
				if(pPyFilter != NULL)
				{
					char *pPyBuffer = new char[GetFileSize(hPyFilter, NULL) + 1];
					RtlCopyMemory(pPyBuffer, pPyFilter, GetFileSize(hPyFilter, NULL));
					pPyBuffer[GetFileSize(hPyFilter, NULL)] = '\0';
					PyRun_SimpleString(pPyBuffer);
					delete [] pPyBuffer;
					
					if((pyConnectFilter_ == NULL) || (pyRecvFilter_ == NULL) || (pySendFilter_ == NULL))
					{
						std::cout << "Error: Python LspPyFilter.set_connect_filter, set_recv_filter or set_send_filter Failed." << std::endl;
					}
	
					UnmapViewOfFile(pPyFilter);
				}
				else
				{
					std::cout << "Error: MapViewOfFile() Failed." << std::endl;
					return;
				}
				CloseHandle(hPyFilterMapping);
			}
			else
			{
				std::cout << "Error: CreateFileMapping() Failed." << std::endl;
			}
	
			CloseHandle(hPyFilter);
		}
		else
		{
			std::cout << "Error: CreateFile() Failed." << std::endl;
		}
	
		std::cout << std::endl;
	}
	__finally
	{
		Unlock();
	}
}

// ========================================================================================================================

void PyInstance::Unload()
{
	if(pyConnectFilter_ != NULL)
	{
		Py_DECREF(pyConnectFilter_);
		pyConnectFilter_ = NULL;
	}
	if(pyDisconnectFilter_ != NULL)
	{
		Py_DECREF(pyDisconnectFilter_);
		pyDisconnectFilter_ = NULL;
	}
	if(pyRecvFilter_ != NULL)
	{
		Py_DECREF(pyRecvFilter_);
		pyRecvFilter_ = NULL;
	}
	if(pySendFilter_ != NULL)
	{
		Py_DECREF(pySendFilter_);
		pySendFilter_ = NULL;
	}
	
	Py_Finalize();
}

// ========================================================================================================================

void PyInstance::ConnectFilter(unsigned int process,
							   unsigned int thread,
							   unsigned int socket,
							   unsigned int *ip,
							   unsigned short *port)
{
	__try
	{
		Lock();

		PyObject *arglist = Py_BuildValue("(I,I,I,I,H)", process, thread, socket, *ip, *port);
		PyObject *result = PyEval_CallObject(pyConnectFilter_, arglist);

		Py_DECREF(arglist);
		arglist = NULL;

		if(result != NULL)
		{
			if(result != Py_None)
			{
				unsigned int newIp = 0;
				unsigned short newPort = 0;
				
				if(PyArg_ParseTuple(result, "IH", &newIp, &newPort))
				{
					*ip = static_cast<unsigned int>(newIp);
					*port = static_cast<unsigned short>(newPort);
				}
				else
				{
					PyErr_WriteUnraisable(pyConnectFilter_);
				}

				PyErr_Clear();
			}
			Py_DECREF(result);
		}
		else
		{
			PyErr_WriteUnraisable(pyConnectFilter_);
		}
	}
	__finally
	{
		Unlock();
	}
}

// ========================================================================================================================

void PyInstance::DisconnectFilter(unsigned int process, unsigned int thread, unsigned int socket)
{
	__try
	{
		Lock();

		PyObject *arglist = Py_BuildValue("(I,I,I)", process, thread, socket);
		PyObject *result = PyEval_CallObject(pyDisconnectFilter_, arglist);

		Py_DECREF(arglist);
		arglist = NULL;

		if(result != NULL)
		{
			if(result != Py_None)
			{
				PyErr_Clear();
			}
			Py_DECREF(result);
		}
		else
		{
			PyErr_WriteUnraisable(pyDisconnectFilter_);
		}
	}
	__finally
	{
		Unlock();
	}
}

// ========================================================================================================================

void PyInstance::RecvFilter(unsigned int process,
							unsigned int thread,
							unsigned int socket,
							BSTR *recvBuffer,
							BSTR *modifiedRecvBuffer,
							unsigned int *remaining)
{
	__try
	{
		Lock();

		char *pBstr = reinterpret_cast<char *>(&(*recvBuffer[0]));
		unsigned int pyBufferLen = SysStringByteLen(*recvBuffer);
		
		PyObject *arglist = Py_BuildValue("(I,I,I,s#,i)", process, thread, socket, pBstr, pyBufferLen, pyBufferLen);
		PyObject *result = PyEval_CallObject(pyRecvFilter_, arglist);
		Py_DECREF(arglist);
		arglist = NULL;

		if(result != NULL)
		{
			if(result != Py_None)
			{
				PyObject *pReturnBuffer = NULL;
				unsigned int pReturnBufferLen = 0;
				if(PyArg_Parse(result, "s#", &pReturnBuffer, &pReturnBufferLen))
				{
					*modifiedRecvBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(pReturnBuffer), pReturnBufferLen);
				}
				else if(PyArg_Parse(result, "I", remaining))
				{
					*modifiedRecvBuffer = NULL;
				}
				else
				{
					*modifiedRecvBuffer = NULL;
					PyErr_WriteUnraisable(pyRecvFilter_);
				}
				PyErr_Clear();
			}

			Py_DECREF(result);
		}
		else
		{
			PyErr_WriteUnraisable(pyRecvFilter_);
			*modifiedRecvBuffer = NULL;
		}
	}
	__finally
	{
		Unlock();
	}
}


// ========================================================================================================================

void PyInstance::SendFilter(unsigned int process,
							unsigned int thread,
							unsigned int socket,
							BSTR *sendBuffer,
							BSTR *modifiedSendBuffer)
{
	__try
	{
		Lock();

		char *pBstr = reinterpret_cast<char *>(&(*sendBuffer[0]));
		unsigned int pyBufferLen = SysStringByteLen(*sendBuffer);
		
		PyObject *arglist = Py_BuildValue("(I,I,I,s#,i)", process, thread, socket, pBstr, pyBufferLen, pyBufferLen);
		PyObject *result = PyEval_CallObject(pySendFilter_, arglist);
		Py_DECREF(arglist);
		arglist = NULL;

		if(result != NULL)
		{
			if(result != Py_None)
			{
				PyObject *pReturnBuffer = NULL;
				unsigned int pReturnBufferLen = 0;
				if(PyArg_Parse(result, "s#", &pReturnBuffer, &pReturnBufferLen))
				{
					*modifiedSendBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(pReturnBuffer), pReturnBufferLen);
				}
				else
				{
					PyErr_WriteUnraisable(pySendFilter_);
				}
				PyErr_Clear();
			}

			Py_DECREF(result);
		}
		else
		{
			PyErr_WriteUnraisable(pySendFilter_);
			*modifiedSendBuffer = NULL;
		}
	}
	__finally
	{
		Unlock();
	}
}

// ========================================================================================================================

PyInstance *PyInstance::GetInstance()
{
	return &g_PyInstance;
}

// ========================================================================================================================

PyObject *PyInstance::SetConnectFilter(PyObject *dummy, PyObject *args)
{
	PyObject *pyResult = NULL;

	__try
	{
		g_PyInstance.Lock();

		if(PyArg_ParseTuple(args, "O", &g_PyInstance.pyConnectFilter_))
		{
			if(!PyCallable_Check(g_PyInstance.pyConnectFilter_))
			{
				PyErr_SetString(PyExc_TypeError, "Error: SetConnectFilter() - Parameter Must Be Callable.");
			}
			else
			{
				Py_XINCREF(g_PyInstance.pyConnectFilter_); 
				Py_INCREF(Py_None);
				pyResult = Py_None;
			}
		}
	}
	__finally
	{
		g_PyInstance.Unlock();
	}
    return pyResult;
}

// ========================================================================================================================

PyObject *PyInstance::SetDisconnectFilter(PyObject *dummy, PyObject *args)
{
	PyObject *pyResult = NULL;

	__try
	{
		g_PyInstance.Lock();

		if(PyArg_ParseTuple(args, "O", &g_PyInstance.pyDisconnectFilter_))
		{
			if(!PyCallable_Check(g_PyInstance.pyDisconnectFilter_))
			{
				PyErr_SetString(PyExc_TypeError, "Error: SetDisconnectFilter() - Parameter Must Be Callable.");
			}
			else
			{
				Py_XINCREF(g_PyInstance.pyDisconnectFilter_); 
				Py_INCREF(Py_None);
				pyResult = Py_None;
			}
		}
	}
	__finally
	{
		g_PyInstance.Unlock();
	}
    return pyResult;
}

// ========================================================================================================================

PyObject *PyInstance::SetRecvFilter(PyObject *dummy, PyObject *args)
{
	PyObject *pyResult = NULL;

	__try
	{
		g_PyInstance.Lock();

		if(PyArg_ParseTuple(args, "O", &g_PyInstance.pyRecvFilter_))
		{
			if(!PyCallable_Check(g_PyInstance.pyRecvFilter_))
			{
				PyErr_SetString(PyExc_TypeError, "Error: SetRecvFilter() - Parameter Must Be Callable.");
			}
			else
			{
				Py_XINCREF(g_PyInstance.pyRecvFilter_); 
				Py_INCREF(Py_None);
				pyResult = Py_None;
			}
		}
	}
	__finally
	{
		g_PyInstance.Unlock();
	}
    return pyResult;
}

// ========================================================================================================================

PyObject *PyInstance::SetSendFilter(PyObject *dummy, PyObject *args)
{
	PyObject *pyResult = NULL;

	__try
	{
		g_PyInstance.Lock();

		if(PyArg_ParseTuple(args, "O", &g_PyInstance.pySendFilter_))
		{
			if(!PyCallable_Check(g_PyInstance.pySendFilter_))
			{
				PyErr_SetString(PyExc_TypeError, "Error: SetSendFilter() - Parameter Must Be Callable.");
			}
			else
			{
				Py_XINCREF(g_PyInstance.pySendFilter_); 
				Py_INCREF(Py_None);
				pyResult = Py_None;
			}
		}
	}
	__finally
	{
		g_PyInstance.Unlock();
	}
    return pyResult;
}

// ========================================================================================================================

void PyInstance::Lock()
{
	if(WaitForSingleObject(hMutex_, 2500) != WAIT_OBJECT_0)
	{
		std::cout << "Warning: WaitForSingleObject(hMutex_) Failed." << std::endl;
	}
}

// ========================================================================================================================

void PyInstance::Unlock()
{
	ReleaseMutex(hMutex_);
}

// ========================================================================================================================
