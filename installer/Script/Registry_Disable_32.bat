del "%SystemRoot%\System32\EasyHook32.dll"
del "%SystemRoot%\System32\gdipp_common_32.dll"
del "%SystemRoot%\System32\gdipp_setting.xml"

reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "AppInit_DLLs" /t REG_SZ /d "" /f
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "LoadAppInit_DLLs" /t REG_DWORD /d 0 /f
