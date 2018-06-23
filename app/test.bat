@echo off  
setlocal enabledelayedexpansion  
for /f "delims=" %%i in ('dir /a-d /b *.*') do (  
 echo %%i:   %%~zi×Ö½Ú&echo.  
)  