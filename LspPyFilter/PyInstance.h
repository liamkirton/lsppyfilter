// ========================================================================================================================
// LspPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// PyInstance.h
//
// Created: 15/05/2007
// ========================================================================================================================

#pragma once

// ========================================================================================================================

#include <windows.h>

#include <python.h>

#include <string>

// ========================================================================================================================

class PyInstance
{
public:
	PyInstance();
	~PyInstance();

	void Load(const std::string &path);
	void Unload();

	void ConnectFilter(unsigned int process,
					   unsigned int thread,
					   unsigned int socket,
					   unsigned int *ip,
					   unsigned short *port);

	void DisconnectFilter(unsigned int process, unsigned int thread, unsigned int socket);

	void RecvFilter(unsigned int process,
					unsigned int thread,
					unsigned int socket,
					BSTR *recvBuffer,
					BSTR *modifiedRecvBuffer,
					unsigned int *remaining);

	void SendFilter(unsigned int process,
					unsigned int thread,
					unsigned int socket,
					BSTR *sendBuffer,
					BSTR *modifiedSendBuffer);

	static PyInstance *GetInstance();
	
	static PyObject *PyInstance::SetConnectFilter(PyObject *dummy, PyObject *args);
	static PyObject *PyInstance::SetDisconnectFilter(PyObject *dummy, PyObject *args);
	static PyObject *PyInstance::SetRecvFilter(PyObject *dummy, PyObject *args);
	static PyObject *PyInstance::SetSendFilter(PyObject *dummy, PyObject *args);

private:
	void Lock();
	void Unlock();

	HANDLE hMutex_;
	
	PyObject *pyConnectFilter_;
	PyObject *pyDisconnectFilter_;
	PyObject *pyRecvFilter_;
	PyObject *pySendFilter_;
};

// ========================================================================================================================
