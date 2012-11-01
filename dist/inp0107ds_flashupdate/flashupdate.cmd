@echo off

::::::::::::::::: Search for LibUSB-WIN32 :::::::::::::::::::::

FOR /F "tokens=2*" %%A IN ('REG.EXE QUERY "hklm\software\microsoft\windows\currentversion\uninstall\libusb-win32_is1" /v InstallLocation 2^>NUL ^| FIND "REG_SZ"') DO SET lpath=%%B

IF NOT "%lpath%"==""  GOTO foundlibusb

echo .
echo ******************  ERROR  ***********************
echo **            LibUSB-WIN32 not found            **
echo **************************************************
echo .
pause
exit 1


:foundlibusb

SET libusbinstallfilter="%lpath%\bin\install-filter.exe"
rem SET libusbinstallfilter="install-filter.exe"

::::::::::::::::: OS VERSION DETECT ::::::::::::::::::::::::::
VER | FINDSTR /C:" 5." 1>NUL 2>NUL

IF '%errorlevel%' == '0'  goto xp

::::::::::::::::: OS VISTA/7  ::::::::::::::::::::::::::::::::
:vista7

SET libusbinstallfilter=bin\elevate -c %libusbinstallfilter%
goto flashing

::::::::::::::::: OS XP       ::::::::::::::::::::::::::::::::
:: check if we have priveleges to manipulate libusb
:xp

NET FILE 1>NUL 2>NUL

IF '%errorlevel%' == '0'  goto flashing

::::::::::::::::: OS XP       ::::::::::::::::::::::::::::::::
:elevate
echo .
echo ******************  ERROR  ***********************
echo ** Please run this script with admin priveleges **
echo **************************************************
echo .
exit 1

::::::::::::::::: Flashing now :::::::::::::::::::::::::::::::
:flashing

echo .
echo Configuring libusb driver to handle FTDI chip...
echo .

%libusbinstallfilter% install "--device=Vid_0403&Pid_6001"
IF NOT '%errorlevel%' == '0'  goto elevate

echo .
echo **************************************************
echo *      Please attach device into USB port        *
echo **************************************************
echo .
pause

echo .
echo Waiting device to be configured...
echo .
ping 127.0.0.1 -n 6 1>NUL 2>NUL

bin\aducftdi.exe firmware\inp0107v23.hex

IF '%errorlevel%' == '0'  goto fallback

echo .
echo ******************  ERROR  ***********************
echo **   Some error happens :(   Please try again   **
echo **************************************************
echo .
pause
exit 1

:fallback

echo .
echo Rolling back default FTDI driver...
echo .

%libusbinstallfilter% uninstall "--device=Vid_0403&Pid_6001"
IF '%errorlevel%' === '0'  goto elevate

pause
