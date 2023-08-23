@echo off

wget -nc https://github.com/espressif/idf-installer/releases/download/online-2.22/esp-idf-tools-setup-online-2.22.exe

set idf_install_exe=esp-idf-tools-setup-online-2.22.exe
set idf_install_url=https://dl.espressif.com/dl/idf-installer/%idf_install_exe%

REM prompt if user wants to install %idf_install_exe%
echo %idf_install_exe% is required to install the ESP32 toolchain.
set /p install_idf=Do you want to install it now (y/n)? 
if %install_idf%==y (
    powershell -Command "Invoke-WebRequest -Uri %idf_install_url% -OutFile %idf_install_exe%"
    cmd /c %idf_install_exe%
    del %idf_install_exe%
)

REM # Workaround a bug for the fmt library in managed components
