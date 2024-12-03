echo off

set build_folder=build

set clean=""
set prepare=""
set build=""
set help=""

if "%1" == "make" (
  set clean=1
  set prepare=1
  set build=1
) else if "%1" == "rebuild" (
  set build=1
) else if "%1" == "cleanup" (
  set clean=1
) else if "%1" == "help" (
  set help=1
) else (
  echo No actions by default!
)

if "%clean%" == "1" (
  if exist "%build_folder%" (
    RMDIR "./%build_folder%" /S
  )
)

if "%prepare%" == "1" (
  mkdir %build_folder%
  cd %build_folder%
  cmake ..
  cd ..
)

if "%build%" == "1" (
  if exist %build_folder% (
    cmake --build %build_folder%
  ) else (
    echo Run "make" first to setup project to build.
  )
)

if "%help%" == "1" (
  echo Help:
  echo   make
  echo                  Setup everything in project with cleaning first and will compile.
  echo[
  echo   rebuild
  echo                  Cleanup everything, prepare building files and compile.
  echo[
  echo   cleanup
  echo                  Make sure there is no temporary build files.
  echo[
)