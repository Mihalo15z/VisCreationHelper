@echo off
echo %time%
setlocal

set LINUX_MULTIARCH_ROOT=C:\UnrealToolchains\v17_clang-10.0.1-centos7\
set Configuration=%1
set Platform=%2
set StageDir=%3
set ArchiveDir=%4
set DebugInfo=%5
set CrashReporter=%6

for /f "tokens=6,* delims= " %%a in ("%*") do set AdditionalParams=%%b
REM The above line loads all params from the 7th onward into AdditionalParams

IF [%Configuration%] == [] (
    set Configuration=Development
)
IF [%Platform%] == [] (
    set Platform=Win64
)

@if "%Platform%" == "Win64" (set Platform_Dir=WindowsNoEditor)
@if "%Platform%" == "Linux" (set Platform_Dir=LinuxNoEditor)
@if "%Platform%" == "Mac" (set Platform_Dir=MacNoEditor)
@if "%Platform%" == "XboxOne" (set Platform_Dir=XboxOne)
@if "%Platform%" == "PS4" (set Platform_Dir=PS4)

IF [%StageDir%] == [] (
    set StageDir=%~dp0..\..\K1\Build\%Platform_Dir%\Staging\
)
IF [%ArchiveDir%] == [] (
    set ArchiveDir=%~dp0..\..\K1\Build\%Platform_Dir%\
)
IF [%DebugInfo%] == [] (
    IF %Platform==Linux (
        set DebugInfo=false
    ) ELSE (
        set DebugInfo=true
    )
)
IF [%CrashReporter%] == [] (
    IF %Platform==Linux (
        set CrashReporter=false
    ) ELSE (
        set CrashReporter=true
    )
)

IF %DebugInfo%==false (
    set AdditionalParams=%AdditionalParams% -nodebuginfo
)
IF %CrashReporter%==true (
    set AdditionalParams=%AdditionalParams% -CrashReporter
)
echo Configuration=%Configuration%
echo Platform=%Platform%
echo StageDir=%StageDir%
echo ArchiveDir=%ArchiveDir%
echo DebugInfo=%DebugInfo%
echo CrashReporter=%CrashReporter%
echo AdditionalParams=%AdditionalParams%

cmd /c "..\..\Engine\Build\BatchFiles\RunUAT.bat -ScriptsForProject=%~dp0..\..\K1\K1.uproject BuildCookRun -nop4 -project=%~dp0..\..\K1\K1.uproject -cook -stage -stagingdirectory=%StageDir% -archive -archivedirectory=%ArchiveDir% -clientconfig=%Configuration% -serverconfig=%Configuration% -package -compressed -pak -prereqs -platform=%Platform% -build -utf8output -compile -utf8output %AdditionalParams%"

endlocal
echo %time%
