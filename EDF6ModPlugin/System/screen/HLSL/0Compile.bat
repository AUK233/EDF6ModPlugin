@echo off
set FXC="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\fxc.exe"

%FXC% /T cs_5_0 /nologo /E CS_main /Fo "Z:\TEMP\ToLinearDepth.cso" ToLinearDepth.hlsl

pause