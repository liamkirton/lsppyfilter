@echo off
set install_source=release
regsvr32 /s %install_source%\LspPyFilterMarshal.dll
%install_source%\LspInstall.exe /Ifs /Install 1001,1002 LspPyFilterProvider C:\LspPyFilter\%install_source%\LspPyFilterProvider.dll
%install_source%\LspInstall.exe /Print
