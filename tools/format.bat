@echo off
rem Reformat every source file in this repository in place, using the
rem repository's .clang-format. See docs/CodingStandards.md.
rem
rem Only our own sources are touched: Thirdparty/ is vendored, and CMake files,
rem shaders and assets are outside the standard's scope.
setlocal EnableDelayedExpansion
call "%~dp0_llvm.bat" || exit /b 1
pushd "%~dp0.." || exit /b 1

set /a N=0
for /f "delims=" %%f in ('git ls-files ^| findstr /b /c:"Engine/" /c:"Editor/" /c:"Game/" /c:"UnitTests/"') do call :FormatOne "%%f"
echo [format] reformatted !N! files with clang-format
popd
exit /b 0

:FormatOne
set "EXT=%~x1"
if /i not "%EXT%"==".cpp" if /i not "%EXT%"==".h" if /i not "%EXT%"==".hpp" exit /b 0
"%LLVM_BIN%\clang-format.exe" -i --style=file "%~1"
if errorlevel 1 (
  echo [format] FAILED on %~1
  popd
  exit /b 1
)
set /a N+=1
exit /b 0
