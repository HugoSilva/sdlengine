@echo off
rem Fail if any translation unit this branch changes has a naming violation.
rem This is what the pre-push hook runs (.githooks/pre-push); see
rem docs/CodingStandards.md. Only files added or changed since BASE are
rem examined, so the cost follows the diff rather than the tree.
rem
rem A header is not a translation unit, so a changed header is checked through
rem the .cpp files that include it. A header nothing includes is reported as
rem unchecked rather than silently passed.
rem
rem Needs a built tree, because clang-tidy reads the compile commands -- and
rem those carry MSVC .obj.modmap response files that only exist after a compile:
rem   cmake --preset x64-debug && cmake --build --preset x64-debug
rem
rem Usage: check-naming-changed.bat [preset] [base] [tip]
rem        tip defaults to HEAD; the hook passes the ref being pushed.
rem        base defaults to the upstream branch, then origin/HEAD, origin/dev,
rem        origin/master.
setlocal EnableDelayedExpansion
call "%~dp0_llvm.bat" || exit /b 1
pushd "%~dp0.." || exit /b 1

set "PRESET=%~1"
if "%PRESET%"=="" set "PRESET=x64-debug"
if not exist "out\%PRESET%\compile_commands.json" (
  echo [check-naming-changed] out\%PRESET%\compile_commands.json is missing.
  echo [check-naming-changed] Configure and build first: cmake --preset %PRESET% ^&^& cmake --build --preset %PRESET%
  popd
  exit /b 1
)

set "BASE=%~2"
if "%BASE%"=="" for /f "delims=" %%r in ('git rev-parse --abbrev-ref "@{upstream}" 2^>nul') do set "BASE=%%r"
if "%BASE%"=="" for %%c in (origin/HEAD origin/dev origin/master) do (
  if not defined BASE (
    git rev-parse --verify --quiet "%%c" >nul 2>&1
    if not errorlevel 1 set "BASE=%%c"
  )
)
if "%BASE%"=="" (
  echo [check-naming-changed] no branch to compare against. Pass a base explicitly, e.g.
  echo [check-naming-changed]   tools\check-naming-changed.bat %PRESET% origin/dev
  popd
  exit /b 1
)
git rev-parse --verify --quiet "%BASE%^{commit}" >nul 2>&1
if errorlevel 1 (
  echo [check-naming-changed] "%BASE%" is not a commit in this repository.
  popd
  exit /b 1
)

set "TIP=%~3"
if "%TIP%"=="" set "TIP=HEAD"
git rev-parse --verify --quiet "%TIP%^{commit}" >nul 2>&1
if errorlevel 1 (
  echo [check-naming-changed] "%TIP%" is not a commit in this repository.
  popd
  exit /b 1
)

set "LOG=%TEMP%\rse-check-naming-changed-%PRESET%.log"
set "TUS=%TEMP%\rse-check-naming-changed-%PRESET%.tus"
del "%LOG%" >nul 2>&1
del "%TUS%" >nul 2>&1

rem Collect the translation units: changed .cpp files directly, changed headers
rem through the .cpp files that include them. --no-renames makes a rename show
rem up as its added path, so an edited rename is checked like any other add.
for /f "usebackq delims=" %%f in (`git diff --name-only --no-renames --diff-filter=ACM "%BASE%" "%TIP%" -- Engine Editor Game UnitTests`) do call :Collect "%%f"

set /a N=0
set /a SKIPPED=0
if exist "%TUS%" for /f "usebackq delims=" %%f in ("%TUS%") do call :CheckOne "%%f"

if !N! equ 0 (
  echo [check-naming-changed] nothing to check since %BASE% -- !SKIPPED! skipped
  popd
  exit /b 0
)
echo [check-naming-changed] examined !N! translation units changed since %BASE% (!SKIPPED! skipped)

findstr /i /c:"warning:" /c:"error:" "%LOG%" >nul 2>&1
if not errorlevel 1 (
  echo [check-naming-changed] violations:
  findstr /i /c:"warning:" /c:"error:" "%LOG%"
  popd
  exit /b 1
)
echo [check-naming-changed] no naming violations
popd
exit /b 0

:Collect
set "EXT=%~x1"
if /i "%EXT%"==".cpp" (
  call :AddTu "%~1"
  exit /b 0
)
if /i not "%EXT%"==".h" if /i not "%EXT%"==".hpp" exit /b 0
rem Matched on the include line, so a mention in a comment does not count. The
rem match is by file name only: it can pick up an unrelated same-named header's
rem includers (harmless -- an extra translation unit checked), but it cannot
rem follow a transitive include.
set "MAPPED=0"
for /f "delims=" %%l in ('git grep -l -i -e "#include.*%~nx1" -- "Engine/*.cpp" "Editor/*.cpp" "Game/*.cpp" "UnitTests/*.cpp"') do (
  set "MAPPED=1"
  call :AddTu "%%l"
)
if "!MAPPED!"=="0" echo [check-naming-changed] no tracked .cpp includes %~nx1 -- not checked
exit /b 0

:AddTu
rem De-duplicate: one translation unit per run, however many changed files lead
rem to it.
if "%~1"=="" exit /b 0
findstr /x /c:"%~1" "%TUS%" >nul 2>&1
if not errorlevel 1 exit /b 0
>>"%TUS%" echo %~1
exit /b 0

:CheckOne
set "TU=%~1"
rem clang-tidy can only check a file the configure step has seen. A newly added
rem .cpp enters compile_commands.json only after a reconfigure, and a compile
rem command's response files only after a build.
set "TU_FWD=%TU:\=/%"
findstr /i /c:"%TU%" /c:"%TU_FWD%" "out\%PRESET%\compile_commands.json" >nul 2>&1
if errorlevel 1 (
  echo [check-naming-changed] %TU% is not in out\%PRESET%\compile_commands.json
  echo [check-naming-changed] reconfigure and build first: cmake --preset %PRESET% ^&^& cmake --build --preset %PRESET%
  set /a SKIPPED+=1
  exit /b 0
)
set /a N+=1
"%LLVM_BIN%\clang-tidy.exe" -p "out\%PRESET%" --config-file=".clang-tidy" "%TU%" >>"%LOG%" 2>&1
exit /b 0
