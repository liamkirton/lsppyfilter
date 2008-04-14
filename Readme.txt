================================================================================
LspPyFilter 0.2.1
Copyright ©2007 Liam Kirton <liam@int3.ws>

08th September 2007
http://int3.ws/
================================================================================

Overview:
---------

LspPyFilter offers the ability to dynamically and transparently modify incoming
and outgoing network traffic, as well as to redirect outgoing connection
requests. This is achieved through the use of a Windows Layered Service
Provider, together with an embedded Python interpreter instance.

Traffic is modified on-the-fly, by dynamically loadable Python scripts, and
LspPyFilter now includes the structure necessary to allow for the easy
transformation of HTTP traffic (i.e. Content-Length headers are fixed up
automatically). This ability is implemented entirely in Python!

Install:
--------

regsvr32 /s LspPyFilterMarshal.dll
LspInstall.exe /Ifs /Install 1001,1002 LspPyFilterProvider C:\LspPyFilter\LspPyFilterProvider.dll
LspInstall.exe /Print

Uninstall:
----------

regsvr32 /u LspPyFilterMarshal.dll
LspInstall /Print
LspInstall /Uninstall <id> [ Where <id> is the value from the line that contains "+IFS LSP \"LspPyFilterProvider\"" ]

Usage:
------

LspPyFilter.exe <Filter.py>

================================================================================