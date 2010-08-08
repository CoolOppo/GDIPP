del "%SystemRoot%\SysWOW64\EasyHook32.dll"
del "%SystemRoot%\SysWOW64\gdipp_common_32.dll"
del "%SystemRoot%\SysWOW64\gdipp_setting.xml"
del "%SystemRoot%\System32\EasyHook64.dll"
del "%SystemRoot%\System32\gdipp_common_64.dll"
del "%SystemRoot%\System32\gdipp_setting.xml"

reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "AppInit_DLLs" /t REG_SZ /d "" /f
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "LoadAppInit_DLLs" /t REG_DWORD /d 0 /f
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Windows NT\CurrentVersion\Windows" /v "AppInit_DLLs" /t REG_SZ /d "" /f
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Windows NT\CurrentVersion\Windows" /v "LoadAppInit_DLLs" /t REG_DWORD /d 0 /f
