@echo off
set vc_vars="%VS100COMNTOOLS%..\..\VC\vcvarsall.bat"

call %vc_vars% x86
cd Win32
echo.
midl /nologo /win32 /W3 /acf ..\gdipp_rpc.acf ..\gdipp_rpc.idl
echo.
cd ..

call %vc_vars% x64
cd x64
echo.
midl /nologo /x64 /W3 /acf ..\gdipp_rpc.acf ..\gdipp_rpc.idl
echo.
cd ..

pause
