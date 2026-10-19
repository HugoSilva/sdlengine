@echo off
rem Fail if any translation unit in this repository has a naming violation.
rem The rules are in .clang-tidy; see docs/CodingStandards.md.
rem
rem Needs a configured build, because clang-tidy reads the compile commands:
rem   cmake --preset x64-debug
rem Usage: check-naming.bat [preset]     (default x64-debug)
setlocal EnableDelayedExpansion
call "%~dp0_llvm.bat" || exit /b 1
pushd "%~dp0.." || exit /b 1

set "PRESET=%~1"
if "%PRESET%"=="" set "PRESET=x64-debug"
if not exist "out\%PRESET%\compile_commands.json" (
  echo [check-naming] out\%PRESET%\compile_commands.json is missing.
  echo [check-naming] Configure the build first: cmake --preset %PRESET%
  popd
  exit /b 1
)

set "LOG=%TEMP%\rse-check-naming-%PRESET%.log"
del "%LOG%" >nul 2>&1
set /a N=0
for /f "delims=" %%f in ('git ls-files ^| findstr /b /c:"Engine/" /c:"Editor/" /c:"Game/" /c:"UnitTests/"') do call :CheckOne "%%f"
echo [check-naming] examined !N! translation units in out\%PRESET%

findstr /i /c:"warning:" /c:"error:" "%LOG%" >nul 2>&1
if not errorlevel 1 (
  echo [check-naming] violations:
  findstr /i /c:"warning:" /c:"error:" "%LOG%"
  popd
  exit /b 1
)
echo [check-naming] no naming violations
popd
exit /b 0

:CheckOne
set "EXT=%~x1"
if /i not "%EXT%"==".cpp" exit /b 0
set /a N+=1
"%LLVM_BIN%\clang-tidy.exe" -p "out\%PRESET%" --config-file=".clang-tidy" "%~1" >>"%LOG%" 2>&1
exit /b 0
