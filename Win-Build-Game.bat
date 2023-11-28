@echo off
setlocal

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::  Proton2D Windows build game script.  :::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::    
:::::::::::::::::::::::::::::: Default build parameters :::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  

:: Default destination for the build output
set defaultBuildDirectory="build"

:: Default game project name
set defaultProjectName="sandbox"

:: 1 - Debug, 2 - Release, 3 - Distribution
set defaultBuildConfiguration=3

:: Delete all files from output build directory before copying game files.
:: If set to false, you'll need to decide whether to overwrite each file individually.
set clearOutputDirectory=true

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

:: Project Name
:askProject
echo Enter name of project to build (default=%defaultProjectName%):
set /p projectName=

if "%projectName%"=="" (
    set projectName=%defaultProjectName%
) else if not exist %projectName%\ (
    echo Project not found!
    goto askProject
)

:: Build Configuration
echo Select build configuration:
echo 1. Debug
echo 2. Release
echo 3. Distribution
echo:
set /p configChoice=Choice (default=%defaultBuildConfiguration%): 
echo:

if "%configChoice%"=="" (
    set configChoice=%defaultBuildConfiguration%
)
if "%configChoice%"=="1" (
    set buildConfiguration=Debug
) else if "%configChoice%"=="2" (
    set buildConfiguration=Release
) else if "%configChoice%"=="3" (
    set buildConfiguration=Distribution
) else (
    echo Invalid build configuration choice.
    pause
    exit /b
)

:: TODO: Build executable from source code here

:: Get executable path
set executablePath="bin\%buildConfiguration%-windows-x86_64\%projectName%\*.exe"
if not exist %executablePath% (
    echo "Error: Executable not compiled! Could not find %executablePath%"
    pause
    exit /b
)

set dbd=%defaultBuildDirectory%

:: Destination directory
:askDestination
echo Please enter the output directory path (default=%dbd%):
set /p destination=

if "%destination%"=="" set "destination=%dbd%\%projectName%"
if not exist "%dbd%\" (
    mkdir "%dbd%"
)
if not exist "%destination%\" (
    mkdir "%destination%"
    set clearOutputDirectory=false
)
if not exist "%destination%\" (
    echo The output directory does not exist!
    goto askDestination
)

:: Delete old files from output directory
if %clearOutputDirectory%==true (
    echo Deleting old files from output directory...
    del /s /q %dbd%\*.* >nul
)

:: Copy files
echo Copying files...

:: Copy content directory
xcopy "%projectName%\content\" "%destination%\content\" /E /I

:: Copy editor files for Debug and Release configuration
if "%configChoice%"=="1" (
    goto copyEditorFiles
)
if "%configChoice%"=="2" (
    goto copyEditorFiles
)
goto copyOtherFiles

:: Copy editor files
:copyEditorFiles
xcopy "%projectName%\editor\content" "%destination%\editor\content\" /E /I
copy "%projectName%\imgui.ini" "%destination%\"
mkdir "%destination%\editor\cache"

:: Copy other files
:copyOtherFiles
copy "%projectName%\app-config.json" "%destination%\"

:: Copy executable
copy %executablePath% %destination%

echo Building done.
pause

:end
endlocal
