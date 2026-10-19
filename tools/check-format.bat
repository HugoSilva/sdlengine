@echo off
rem Fail if any source file is not clang-format clean. This is the check CI
rem runs; it is complete-or-nothing, because a half-formatted tree looks like a
rem convention. See docs/CodingStandards.md.
rem
rem When it fails it prints clang-format's own message, so a failure can be
rem diagnosed rather than guessed at.
setlocal EnableDelayedExpansion
call "%~dp0_llvm.bat" || exit /b 1
pushd "%~dp0.." || exit /b 1

set "LOG=%TEMP%\rse-check-format.log"
del "%LOG%" >nul 2>&1
set /a N=0
set /a BAD=0
for /f "delims=" %%f in ('git ls-files ^| findstr /b /c:"Engine/" /c:"Editor/" /c:"Game/" /c:"UnitTests/"') do call :CheckOne "%%f"

echo [check-format] checked !N! files, !BAD! need formatting
if !BAD! neq 0 (
  echo [check-format] clang-format said:
  findstr /i /c:"warning" /c:"error" "%LOG%"
  popd
  exit /b 1
)
popd
exit /b 0

:CheckOne
set "EXT=%~x1"
if /i not "%EXT%"==".cpp" if /i not "%EXT%"==".h" if /i not "%EXT%"==".hpp" exit /b 0
set /a N+=1
"%LLVM_BIN%\clang-format.exe" --dry-run --Werror --style=file "%~1" >>"%LOG%" 2>&1
if errorlevel 1 (
  set /a BAD+=1
  echo [check-format] needs formatting: %~1
)
exit /b 0
