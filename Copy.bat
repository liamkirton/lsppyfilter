@echo off
mkdir Release
xcopy /c /e /y Z:\LspPyFilter\release .\release
mkdir Filters
xcopy /c /e /y Z:\LspPyFilter\Filters .\filters
copy /y Z:\LspPyFilter\Install.bat .