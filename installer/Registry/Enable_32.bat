if defined gdipp_registry (
  setx Path "%Path%;%~f1" /m
) else (
  reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "AppInit_DLLs" /t REG_SZ /d "gdimm_32.dll" /f
  reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "LoadAppInit_DLLs" /t REG_DWORD /d 1 /f

  set gdipp_registry=1
  call "%0" "%~dp0.."
  set gdipp_registry=
)