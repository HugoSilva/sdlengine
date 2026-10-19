@echo off
rem Locate the LLVM tools (clang-format, clang-tidy) and export LLVM_BIN.
rem
rem This lives in one place so the formatting and naming gates can never
rem disagree about which clang they used. Precedence:
rem   1. %LLVM_BIN%     (explicit override)
rem   2. PATH           (a system-wide LLVM)
rem   3. the Visual Studio install ("C++ Clang tools for Windows" component)
setlocal EnableDelayedExpansion
set "FOUND="

if defined LLVM_BIN if exist "!LLVM_BIN!\clang-format.exe" set "FOUND=!LLVM_BIN!"

if not defined FOUND for %%p in (clang-format.exe) do (
  if not "%%~$PATH:p"=="" for %%i in ("%%~$PATH:p") do set "FOUND=%%~dpi"
)

if not defined FOUND (
  set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
  if exist "!VSWHERE!" for /f "usebackq delims=" %%i in (`"!VSWHERE!" -latest -property installationPath`) do (
    if exist "%%i\VC\Tools\Llvm\x64\bin\clang-format.exe" set "FOUND=%%i\VC\Tools\Llvm\x64\bin"
  )
)

if not defined FOUND (
  echo [style] clang-format was not found.
  echo [style] Install the Visual Studio component "C++ Clang tools for Windows", put the LLVM
  echo [style] bin directory on PATH, or set LLVM_BIN to the directory holding clang-format.exe.
  exit /b 1
)

endlocal & set "LLVM_BIN=%FOUND%"
exit /b 0
