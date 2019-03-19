#!/bin/bash
usage="Usage: $(basename "$0") [-h] [-d] [-n] [-c] [-v] source.cpp
-- build and run single-file allolib applications.

options:
  -h show this help text
  -d build debug version and run in debugger
  -n build only. Don't run.
  -c clean build
  -v verbose build. Prints full cmake log and verbose build.
"

APP_ROOT=${PWD} # gives absolute path
# echo "Script executed from: ${APP_ROOT}"

# BASH_SOURCE has the script's path
# could be absolute, could be relative
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})

FIRSTCHAR=${SCRIPT_PATH:0:1}
if [ ${FIRSTCHAR} == "/" ]; then
  # it's asolute path
  AL_LIB_PATH=${SCRIPT_PATH}
else
  # SCRIPT_PATH was relative
  AL_LIB_PATH=${APP_ROOT}/${SCRIPT_PATH}
fi

# Get the number of processors on OS X; Linux; or MSYS2, or take a best guess.
NPROC=$(grep --count ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu || nproc || echo 2)
# Save one core for the gui.
PROC_FLAG=$((NPROC - 1))

# resolve flags
BUILD_TYPE=Release
DO_CLEAN=0
IS_VERBOSE=0
VERBOSE_FLAG=OFF
RUN_MPI=0
RUN_APP=1

if [ $# == 0 ]; then
  echo "$usage"
  exit 1
fi

while getopts "adncvhjx" opt; do
  case "${opt}" in
  a)
    RUN_MPI=1
    RUN_APP=0
    ;;
  d)
    BUILD_TYPE=Debug
    POSTFIX=d # if release, there's no postfix
    ;;
  n)
    RUN_APP=0
    ;;
  c)
    DO_CLEAN=1
    ;;
  v)
    IS_VERBOSE=1
    VERBOSE_MAKEFILE=-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
    VERBOSE_FLAG=ON
    ;;
  h) echo "$usage"
  exit
  ;;
  j)
if [ $(uname -s) == "Darwin" ]; then
  CURRENT_OS="MACOS"
  # echo "running on macOS"
  # Check if ninja available
  command -v ninja >/dev/null 2>&1 && { echo "Using Ninja"; export GENERATOR='-G Ninja'; }
fi

if [ $(uname -s) == "Linux" ]; then
  CURRENT_OS="LINUX"
  # Check if ninja available
  command -v ninja >/dev/null 2>&1 && { echo "Using Ninja"; export GENERATOR='-G Ninja'; }
fi
  ;;
  x) 
  GENERATOR_PLATFORM=x86
    ;;
  \?) echo "$usage" >&2
    exit 1
    ;;
  esac
done

# consume options that were parsed
shift $(expr $OPTIND - 1 )

if [ $(uname -s) == "Darwin" ]; then
  BUILD_FLAGS="${BUILD_FLAGS} -j${PROC_FLAG}"
elif [ $(uname -s) == "Linux" ]; then
  BUILD_FLAGS="${BUILD_FLAGS} -j${PROC_FLAG}"
elif [ $(uname -s) != MINGW64* ] && [ "${GENERATOR_PLATFORM}" != "x86" ]; then
  WINDOWS_FLAGS=-DCMAKE_GENERATOR_PLATFORM=x64
fi

if [ ${IS_VERBOSE} == 1 ]; then
  echo "BUILD TYPE: ${BUILD_TYPE}"
fi

APP_FILE_INPUT="$1" # first argument (assumming we consumed all the options above)
APP_PATH=$(dirname ${APP_FILE_INPUT})
APP_FILE=$(basename ${APP_FILE_INPUT})
APP_NAME=$(basename ${APP_FILE_INPUT} | sed 's/\.[^.]*$//') # remove extension

# echo "app path: ${APP_PATH}"
# echo "app file: ${APP_FILE}"
# echo "app name: ${APP_NAME}"

(
  if [ ${DO_CLEAN} == 1 ]; then
    if [ ${IS_VERBOSE} == 1 ]; then
      echo "cleaning build"
    fi
    rm -r build
  fi
  mkdir -p build
  cd build
  mkdir -p ${APP_NAME}
  cd ${APP_NAME}
  mkdir -p ${BUILD_TYPE}
  cd ${BUILD_TYPE}

  echo cmake ${GENERATOR} ${WINDOWS_FLAGS} -Wno-deprecated -DBUILD_EXAMPLES=0 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DAL_APP_FILE=../../../${APP_PATH}/${APP_FILE} -DAL_VERBOSE_OUTPUT=${VERBOSE_FLAG} ${VERBOSE_MAKEFILE} -DAL_APP_RUN=${RUN_APP} ${AL_LIB_PATH}/cmake/single_file

  cmake ${GENERATOR} ${WINDOWS_FLAGS} -Wno-deprecated -DBUILD_EXAMPLES=0 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DAL_APP_ROOT=${APP_ROOT} -DAL_APP_FILE=${APP_ROOT}/${APP_PATH}/${APP_FILE} -DAL_VERBOSE_OUTPUT=${VERBOSE_FLAG} ${VERBOSE_MAKEFILE} -DAL_APP_RUN=${RUN_APP} ${AL_LIB_PATH}/cmake/single_file > cmake_log.txt

  if [ ${RUN_APP} == 1 ]; then
    APP_NAME=${APP_NAME}_run  
  fi
  if [ ${BUILD_TYPE} == "Debug" ]; then
    APP_NAME=${APP_NAME}_debug
  fi

  echo cmake --build . --target ${APP_NAME} --config ${BUILD_TYPE} -- ${BUILD_FLAGS}

  cmake --build . --target ${APP_NAME} --config ${BUILD_TYPE} -- ${BUILD_FLAGS}
)

APP_BUILD_RESULT=$?
# if app failed to build, exit
if [ ${APP_BUILD_RESULT} != 0 ]; then
  echo "app ${APP_NAME} failed to build"
  exit 1
fi

# run app
# go to where the binary is so we have cwd there
# (app's cmake is set to put binary in 'bin')

if [ ${RUN_MPI}  != 0 ]; then
  cd ${APP_ROOT}
  cd ${APP_PATH}/bin
  echo Running MPI at: `pwd`

  mpirun --mca btl_tcp_if_include enp1s0 --hostfile ../../mpi_hosts.txt /usr/bin/env DISPLAY=:0 ./"${APP_NAME}${POSTFIX}"
# elif [ ${BUILD_TYPE} == "Debug" ]; then
#   echo "Running in debugger ${DEBUGGER}"
#   ${DEBUGGER} ./"${APP_NAME}${POSTFIX}"
# else
#   ./"${APP_NAME}${POSTFIX}"
fi
