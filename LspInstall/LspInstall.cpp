// ========================================================================================================================
// LspInstall
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// LspInstall.cpp
//
// Created: 03/05/2007
// ========================================================================================================================

#include <winsock2.h>
#include <ws2spi.h>
#include <sporder.h>

#include <windows.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// ========================================================================================================================

const wchar_t *c_LspInstallVersion = L"0.2.1";

// ========================================================================================================================

typedef void (WSPAPI *GetLspGuidProc)(LPGUID);

// ========================================================================================================================

void InstallLsp(std::string baseCatalogIdListStr, std::string lspName, std::string lspDllPath, bool ifsLsp);
void UninstallLsp(std::string baseCatalogIdListStr);;

void PrintProtocols();
void PrintUsage();

// ========================================================================================================================

int main(int argc, char *argv[])
{
	std::wcout << std::endl
			   << L"LspInstall " << c_LspInstallVersion << std::endl
			   << L"Copyright \xB8" << L"2007 Liam Kirton <liam@int3.ws>" << std::endl << std::endl
			   << L"Built at " << __TIME__ << L" on " << __DATE__ << std::endl << std::endl;

	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::wcout << L"Error: WSAStartup() Failed." << std::endl;
		return -1;
	}

	try
	{
		if(argc <= 1)
		{
			PrintUsage();
			throw L"Invalid Command Line.";
		}

		std::string baseCatalogIdListStr;
		std::string lspName;
		std::string lspDllPath;
		bool ifsLsp = false;

		for(int i = 1; i < argc; ++i)
		{
			std::string cmd = argv[i];
			std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

			if((cmd == "/install") && ((i + 3) < argc))
			{
				baseCatalogIdListStr = argv[++i];
				lspName = argv[++i];
				lspDllPath = argv[++i];
			}
			else if((cmd == "/uninstall") && ((i + 1) < argc))
			{
				baseCatalogIdListStr = argv[++i];
			}
			else if(cmd == "/ifs")
			{
				ifsLsp = true;
			}
			else if(cmd == "/print")
			{
				PrintProtocols();
				break;
			}
			else
			{
				PrintUsage();
				throw L"Invalid Command Line.";
			}
		}

		if(!baseCatalogIdListStr.empty() && !lspName.empty() && !lspDllPath.empty())
		{
			InstallLsp(baseCatalogIdListStr, lspName, lspDllPath, ifsLsp);
		}
		else if(!baseCatalogIdListStr.empty())
		{
			UninstallLsp(baseCatalogIdListStr);
		}
	}
	catch(const wchar_t *e)
	{
		std::wcout << L"Fatal Error: " << e << std::endl;
	}
	catch(...)
	{
		std::wcout << L"Fatal Error: Unhandled Exception." << std::endl;
	}

	WSACleanup();

	return 0;
}

// ========================================================================================================================

void InstallLsp(std::string baseCatalogIdListStr, std::string lspName, std::string lspDllPath, bool ifsLsp)
{
	DWORD dwBufferLength = 0;
	DWORD dwProtocolCount = 0;
	DWORD dwDummyProtocolCatalogEntryId = 0;
	LPWSAPROTOCOL_INFOW lpProtocolBuffer = NULL;
	LPWSAPROTOCOL_INFOW lpDummyProtocol = NULL;

	wchar_t *wLspName = NULL;
	wchar_t *wLspDllPath = NULL;

	GUID lspGuid;

	try
	{
		int cchWideChar;
		if((cchWideChar = MultiByteToWideChar(CP_UTF8, 0, lspName.c_str(), lspName.size(), NULL, 0)) != 0)
		{
			wLspName = new wchar_t[cchWideChar + 1];
			if((cchWideChar = MultiByteToWideChar(CP_UTF8,
												  0,
												  lspName.c_str(),
												  lspName.size(),
												  wLspName,
												  cchWideChar)) == 0)
			{
				throw L"MultiByteToWideChar(wLspName) Failed.";
			}
			wLspName[cchWideChar] = L'\0';
		}
		else
		{
			throw L"MultiByteToWideChar(NULL) Failed.";
		}

		if((cchWideChar = MultiByteToWideChar(CP_UTF8, 0, lspDllPath.c_str(), lspDllPath.size(), NULL, 0)) != 0)
		{
			wLspDllPath = new wchar_t[cchWideChar + 1];
			if((cchWideChar = MultiByteToWideChar(CP_UTF8,
												  0,
												  lspDllPath.c_str(),
												  lspDllPath.size(),
												  wLspDllPath,
												  cchWideChar)) == 0)
			{
				throw L"MultiByteToWideChar(wLspDllPath) Failed.";
			}
			wLspDllPath[cchWideChar] = L'\0';
		}
		else
		{
			throw L"MultiByteToWideChar(NULL) Failed.";
		}

		std::vector<int> baseCatalogIdList;
		size_t baseIdSeperatorOffset = 0;
		
		while(true)
		{
			size_t nextBaseIdSeparatorOffset = baseCatalogIdListStr.find(",", baseIdSeperatorOffset);
			baseCatalogIdList.push_back(strtol(baseCatalogIdListStr.substr(baseIdSeperatorOffset,
																		   nextBaseIdSeparatorOffset - baseIdSeperatorOffset).c_str(),
											   NULL,
											   10));
			if(nextBaseIdSeparatorOffset == std::string::npos)
			{
				break;
			}
			baseIdSeperatorOffset = nextBaseIdSeparatorOffset + 1;
		}
		if(baseCatalogIdList.empty())
		{
			throw L"Invalid Base Catalog Id List.";
		}

		std::wcout << L"Installing LSP \"" << wLspName << L"\" [" << wLspDllPath << L"]:" << std::endl << std::endl;

		int errNo = 0;
		if((WSCEnumProtocols(NULL, NULL, &dwBufferLength, &errNo) != SOCKET_ERROR) || (errNo != WSAENOBUFS))
		{
			throw L"WSCEnumProtocols(NULL) Failed.";
		}

		lpProtocolBuffer = reinterpret_cast<LPWSAPROTOCOL_INFOW>(new char[dwBufferLength]);
		SecureZeroMemory(lpProtocolBuffer, dwBufferLength);

		if((dwProtocolCount = WSCEnumProtocols(NULL, lpProtocolBuffer, &dwBufferLength, &errNo)) == SOCKET_ERROR)
		{
			throw L"WSCEnumProtocols(lpProtocolBuffer) Failed.";
		}

		for(int i = 0; i < static_cast<int>(dwProtocolCount); ++i)
		{
			if(std::find(baseCatalogIdList.begin(),
						 baseCatalogIdList.end(),
						 lpProtocolBuffer[i].dwCatalogEntryId) != baseCatalogIdList.end())
			{
				if(lpDummyProtocol == NULL)
				{
					HMODULE hLspDll = LoadLibrary(wLspDllPath);
					if(hLspDll != NULL)
					{
						GetLspGuidProc fpGetLspGuid = reinterpret_cast<GetLspGuidProc>(GetProcAddress(hLspDll,
																									  "GetLspGuid"));
						if(fpGetLspGuid != NULL)
						{
							fpGetLspGuid(&lspGuid);
						}
						FreeLibrary(hLspDll);

						if(fpGetLspGuid == NULL)
						{
							throw L"GetProcAddress(\"GetLspGuid\") Failed.";
						}
					}
					else
					{
						throw L"LoadLibrary(wLspDllPath) Failed.";
					}

					lpDummyProtocol = new WSAPROTOCOL_INFOW;
					RtlCopyMemory(lpDummyProtocol, &lpProtocolBuffer[i], sizeof(WSAPROTOCOL_INFOW));
					RtlCopyMemory(&lpDummyProtocol->ProviderId, &lspGuid, sizeof(GUID));
					lpDummyProtocol->dwCatalogEntryId = 0;
					lpDummyProtocol->dwProviderFlags |= PFL_HIDDEN;
					lpDummyProtocol->dwProviderFlags &= (~PFL_MATCHES_PROTOCOL_ZERO);
					lpDummyProtocol->dwServiceFlags1 |= XP1_IFS_HANDLES;
					if(!ifsLsp)
					{
						lpDummyProtocol->dwServiceFlags1 &= (~XP1_IFS_HANDLES);
					}
					lpDummyProtocol->iSocketType = 0;
					lpDummyProtocol->iProtocol   = 0;
					lpDummyProtocol->ProtocolChain.ChainLen = LAYERED_PROTOCOL;
					wcsncpy_s(lpDummyProtocol->szProtocol, WSAPROTOCOL_LEN, wLspName, WSAPROTOCOL_LEN);
					
					if(WSCInstallProvider(&lspGuid, wLspDllPath, lpDummyProtocol, 1, &errNo) == SOCKET_ERROR)
					{
						throw L"WSCInstallProvider(lpDummyProtocol) Failed.";
					}
					
					delete [] lpProtocolBuffer;
					lpProtocolBuffer = NULL;
					delete [] lpDummyProtocol;
					lpDummyProtocol = NULL;

					if((WSCEnumProtocols(NULL, lpProtocolBuffer, &dwBufferLength, &errNo) == SOCKET_ERROR) &&
					   (errNo == WSAENOBUFS))
					{
						lpProtocolBuffer = reinterpret_cast<LPWSAPROTOCOL_INFOW>(new char[dwBufferLength]);
						SecureZeroMemory(lpProtocolBuffer, dwBufferLength);

						if((dwProtocolCount = WSCEnumProtocols(NULL,
															   lpProtocolBuffer,
															   &dwBufferLength,
															   &errNo)) == SOCKET_ERROR)
						{
							throw L"WSCEnumProtocols(lpProtocolBuffer) Failed.";
						}

						for(DWORD j = 0; j < dwProtocolCount; ++j)
						{
							if(memcmp(&lpProtocolBuffer[j].ProviderId, &lspGuid, sizeof(GUID)) == 0)
							{
								lpDummyProtocol = &lpProtocolBuffer[j];
								break;
							}
						}
					}
					else
					{
						throw L"WSCEnumProtocols(NULL) Failed.";
					}

					if(lpDummyProtocol == NULL)
					{
						throw L"lpDummyProtocol NULL.";
					}

					dwDummyProtocolCatalogEntryId = lpDummyProtocol->dwCatalogEntryId;

					std::wcout << dwDummyProtocolCatalogEntryId;
					if(ifsLsp)
					{
						std::wcout << L" +IFS ";
					}
					else
					{
						std::wcout << L" -IFS ";
					}
					std::wcout << L"LSP   \"" << lpDummyProtocol->szProtocol << "\"" << std::endl;

					i = -1;
					continue;
				}

				if(lpProtocolBuffer[i].ProtocolChain.ChainLen != 1)
				{
					throw L"Base Provider Expected.";
				}

				WSAPROTOCOL_INFO chainProtocol;
				RtlCopyMemory(&chainProtocol, &lpProtocolBuffer[i], sizeof(WSAPROTOCOL_INFO));
				chainProtocol.ProtocolChain.ChainLen = 2;
				chainProtocol.ProtocolChain.ChainEntries[0] = lpDummyProtocol->dwCatalogEntryId;
				chainProtocol.ProtocolChain.ChainEntries[1] = lpProtocolBuffer[i].dwCatalogEntryId;
				wcsncpy_s(chainProtocol.szProtocol, WSAPROTOCOL_LEN, wLspName, WSAPROTOCOL_LEN);
				wcsncat_s(chainProtocol.szProtocol, WSAPROTOCOL_LEN, L" Over ", 6);
				wcsncat_s(chainProtocol.szProtocol, WSAPROTOCOL_LEN, lpProtocolBuffer[i].szProtocol, WSAPROTOCOL_LEN);
				chainProtocol.dwServiceFlags1 |= XP1_IFS_HANDLES;
				if(!ifsLsp)
				{
					chainProtocol.dwServiceFlags1 &= (~XP1_IFS_HANDLES);
				}

				if(UuidCreate(&chainProtocol.ProviderId) != RPC_S_OK)
				{
					throw L"UuidCreate() Failed.";
				}
				
				int errNo;
				if(WSCInstallProvider(&chainProtocol.ProviderId, wLspDllPath, &chainProtocol, 1, &errNo) == SOCKET_ERROR)
				{
					throw L"WSCInstallProvider(chainProtocol) Failed.";
				}
			}
		}
		
		if(lpProtocolBuffer != NULL)
		{
			delete [] lpProtocolBuffer;
			lpProtocolBuffer = NULL;
			lpDummyProtocol = NULL;
		}
		
		if((WSCEnumProtocols(NULL, lpProtocolBuffer, &dwBufferLength, &errNo) == SOCKET_ERROR) && (errNo == WSAENOBUFS))
		{
			lpProtocolBuffer = reinterpret_cast<LPWSAPROTOCOL_INFOW>(new char[dwBufferLength]);
			SecureZeroMemory(lpProtocolBuffer, dwBufferLength);

			if((dwProtocolCount = WSCEnumProtocols(NULL, lpProtocolBuffer, &dwBufferLength, &errNo)) == SOCKET_ERROR)
			{
				throw L"WSCEnumProtocols(lpProtocolBuffer) Failed.";
			}

			DWORD *pdwCatIds = new DWORD[dwProtocolCount];
			DWORD dwCatIdCount = 0;

			for(DWORD i = 0; i < dwProtocolCount; ++i)
			{
				if((lpProtocolBuffer[i].ProtocolChain.ChainLen > 1) &&
				   (lpProtocolBuffer[i].ProtocolChain.ChainEntries[0] == dwDummyProtocolCatalogEntryId))
				{
					std::wcout << lpProtocolBuffer[i].dwCatalogEntryId;
					if(lpProtocolBuffer[i].dwServiceFlags1 & XP1_IFS_HANDLES)
					{
						std::wcout << L" +IFS ";
					}
					else
					{
						std::wcout << L" -IFS ";
					}
					std::wcout << L"Chain \"" << lpProtocolBuffer[i].szProtocol << "\"" << std::endl;

					pdwCatIds[dwCatIdCount++] = lpProtocolBuffer[i].dwCatalogEntryId;
				}
			}

			for(DWORD i = 0; i < dwProtocolCount; ++i)
			{
				if((lpProtocolBuffer[i].ProtocolChain.ChainLen <= 1) ||
				   (lpProtocolBuffer[i].ProtocolChain.ChainEntries[0] != dwDummyProtocolCatalogEntryId))
				{
					pdwCatIds[dwCatIdCount++] = lpProtocolBuffer[i].dwCatalogEntryId;
				}
			}

			errNo = WSCWriteProviderOrder(pdwCatIds, dwCatIdCount);
			delete [] pdwCatIds;

			if(errNo != ERROR_SUCCESS)
			{
				throw L"WSCWriteProviderOrder() Failed.";
			}
		}
	}
	catch(const wchar_t *e)
	{
		std::wcout << L"Error: " << e << std::endl;
	}

	if(wLspName != NULL)
	{
		delete [] wLspName;
		wLspName = NULL;
	}
	if(wLspDllPath != NULL)
	{
		delete [] wLspDllPath;
		wLspDllPath = NULL;
	}
	if(lpProtocolBuffer != NULL)
	{
		delete [] lpProtocolBuffer;
		lpProtocolBuffer = NULL;
	}
	if(lpDummyProtocol == NULL)
	{
		delete [] lpDummyProtocol;
		lpDummyProtocol = NULL;
	}

	std::wcout << std::endl;
}

// ========================================================================================================================

void UninstallLsp(std::string baseCatalogIdListStr)
{
	DWORD dwBufferLength = 0;
	DWORD dwProtocolCount = 0;
	LPWSAPROTOCOL_INFOW lpProtocolBuffer = NULL;

	try
	{
		std::vector<int> baseCatalogIdList;
		size_t seperatorOffset = 0;
		
		while(true)
		{
			size_t nextSeparatorOffset = baseCatalogIdListStr.find(",", seperatorOffset);
			baseCatalogIdList.push_back(strtol(baseCatalogIdListStr.substr(seperatorOffset,
																		   nextSeparatorOffset - seperatorOffset).c_str(),
											   NULL,
											   10));
			if(nextSeparatorOffset == std::string::npos)
			{
				break;
			}
			seperatorOffset = nextSeparatorOffset + 1;
		}

		if(baseCatalogIdList.empty())
		{
			throw L"Invalid CatalogId List.";
		}

		int errNo = 0;
		if((WSCEnumProtocols(NULL, lpProtocolBuffer, &dwBufferLength, &errNo) == SOCKET_ERROR) && (errNo == WSAENOBUFS))
		{
			lpProtocolBuffer = reinterpret_cast<LPWSAPROTOCOL_INFOW>(new char[dwBufferLength]);
			SecureZeroMemory(lpProtocolBuffer, dwBufferLength);

			if((dwProtocolCount = WSCEnumProtocols(NULL, lpProtocolBuffer, &dwBufferLength, &errNo)) == SOCKET_ERROR)
			{
				throw L"WSCEnumProtocols(lpProtocolBuffer) Failed.";
			}

			for(std::vector<int>::iterator i = baseCatalogIdList.begin(); i != baseCatalogIdList.end(); ++i)
			{
				int id = (*i);
				LPWSAPROTOCOL_INFOW pDelProtocol = NULL;

				for(DWORD j = 0; j < dwProtocolCount; ++j)
				{
					if(lpProtocolBuffer[j].dwCatalogEntryId == id)
					{
						pDelProtocol = &lpProtocolBuffer[j];
						break;
					}
				}

				if(pDelProtocol == NULL)
				{
					throw L"No Such Provider.";
				}

				std::wcout << L"Uninstalling LSP \"" << pDelProtocol->szProtocol << "\":" << std::endl << std::endl;

				for(DWORD j = 0; j < dwProtocolCount; ++j)
				{
					if((lpProtocolBuffer[j].ProtocolChain.ChainLen > 1) &&
					   (lpProtocolBuffer[j].ProtocolChain.ChainEntries[0] == pDelProtocol->dwCatalogEntryId))
					{
						std::wcout << lpProtocolBuffer[j].dwCatalogEntryId;
						if(lpProtocolBuffer[j].dwServiceFlags1 & XP1_IFS_HANDLES)
						{
							std::wcout << L" +IFS ";
						}
						else
						{
							std::wcout << L" -IFS ";
						}
						std::wcout << L"Chain \"" << lpProtocolBuffer[j].szProtocol << "\"" << std::endl;

						if(WSCDeinstallProvider(&lpProtocolBuffer[j].ProviderId, &errNo) == SOCKET_ERROR)
						{
							throw L"WSCDeinstallProvider(CHAIN) Failed.";
						}
					}
				}

				for(DWORD j = 0; j < dwProtocolCount; ++j)
				{
					if(lpProtocolBuffer[j].dwCatalogEntryId == id)
					{
						std::wcout << lpProtocolBuffer[j].dwCatalogEntryId;
						if(lpProtocolBuffer[j].dwServiceFlags1 & XP1_IFS_HANDLES)
						{
							std::wcout << L" +IFS ";
						}
						else
						{
							std::wcout << L" -IFS ";
						}
						std::wcout << L"LSP   \"" << lpProtocolBuffer[j].szProtocol << "\"" << std::endl;

						if(WSCDeinstallProvider(&lpProtocolBuffer[j].ProviderId, &errNo) == SOCKET_ERROR)
						{
							throw L"WSCDeinstallProvider(CHAIN) Failed.";
						}
					}
				}
			}
		}
	}
	catch(const wchar_t *e)
	{
		std::wcout << L"Error: " << e << std::endl;
	}

	if(lpProtocolBuffer != NULL)
	{
		delete [] lpProtocolBuffer;
		lpProtocolBuffer = NULL;
	}
}

// ========================================================================================================================

void PrintProtocols()
{
	DWORD dwBufferLength = 0;
	LPWSAPROTOCOL_INFOW lpProtocolBuffer = NULL;
	DWORD dwProtocolCount = 0;

	try
	{
		int errNo = 0;
		if((WSCEnumProtocols(NULL, lpProtocolBuffer, &dwBufferLength, &errNo) == SOCKET_ERROR) && (errNo == WSAENOBUFS))
		{
			lpProtocolBuffer = reinterpret_cast<LPWSAPROTOCOL_INFOW>(new char[dwBufferLength]);
			SecureZeroMemory(lpProtocolBuffer, dwBufferLength);

			if((dwProtocolCount = WSCEnumProtocols(NULL, lpProtocolBuffer, &dwBufferLength, &errNo)) == SOCKET_ERROR)
			{
				throw L"WSCEnumProtocols(lpProtocolBuffer) Failed.";
			}

			for(DWORD i = 0; i < dwProtocolCount; ++i)
			{
				std::wcout << lpProtocolBuffer[i].dwCatalogEntryId << " ";
				if(lpProtocolBuffer[i].dwServiceFlags1 & XP1_IFS_HANDLES)
				{
					std::wcout << L"+IFS ";
				}
				else
				{
					std::wcout << L"-IFS ";
				}
				if(lpProtocolBuffer[i].ProtocolChain.ChainLen == 0)
				{
					std::wcout << L"LSP   ";
				}
				else if(lpProtocolBuffer[i].ProtocolChain.ChainLen == 1)
				{
					std::wcout << L"Base  ";
				}
				else
				{
					std::wcout << L"Chain ";
				}

				wchar_t wszProviderPath[MAX_PATH];
				INT iProviderPathLength = MAX_PATH;

				if(WSCGetProviderPath(&lpProtocolBuffer[i].ProviderId, wszProviderPath, &iProviderPathLength, &errNo) != SOCKET_ERROR)
				{
					std::wcout << L"\"" << wszProviderPath << L"\" ";
				}

				std::wcout << L"\"" << lpProtocolBuffer[i].szProtocol << L"\"" << std::endl;
			}
		}
		else
		{
			throw L"WSCEnumProtocols(NULL) Failed.";
		}
	}
	catch(...)
	{
		if(lpProtocolBuffer != NULL)
		{
			delete [] lpProtocolBuffer;
		}
		throw;
	}
}

// ========================================================================================================================

void PrintUsage()
{
	std::wcout << L"Usage: LspInstall.exe [/Install <Base Catalog Ids> <Name> <Path>] | [/Uninstall <Lsp Catalog Ids>] | [/Ifs] | [/Print]" << std::endl << std::endl
			   << "Descriptions:" << std::endl << std::endl
			   << "  /Install - Install specified LSP over specified base providers." << std::endl
			   << "  /Uninstall - Uninstall specified LSP." << std::endl
			   << "  /Ifs - Specified LSP employs IFS handles." << std::endl
			   << "  /Print - Print all installed providers and chains." << std::endl
			   << std::endl;
}

// ========================================================================================================================
