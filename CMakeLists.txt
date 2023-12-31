cmake_minimum_required(VERSION 3.19)

#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#------------ SUPERBUILD/PREBUILD  script------------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

# THIS SECTION MUST STAY AT THE TOP OF ROOT CMakeLists.txt
# Use -DUSE_SUPERBUILD=OFF to skip the SECTION 0

# SUPERBUILD is ON by default but can be disabled by -DUSE_SUPERBUILD=OFF
option(USE_SUPERBUILD "flag that a superbuild should be invoked" ON)
message(STATUS "SUPERBUILD IS ${USE_SUPERBUILD}")

if (USE_SUPERBUILD)
  project(superbuild VERSION 1.0)

  # FLAG THE LOCAL BUILD SYSTEM
  message(STATUS "THE LOCAL BUILD SYSTEM NAME IS : ${CMAKE_SYSTEM_NAME}")
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX_BUILD ON)
    message(STATUS  "LINUX_BUILD is starting")
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOS_BUILD ON)
    message(STATUS  "MACOS_BUILD is starting")
  elseif((${CMAKE_SYSTEM_NAME} MATCHES "Windows") AND MINGW)
    set(WINDOWS_BUILD ON)
    message(STATUS  "WINDOWS_BUILD is starting")
  else()
    message(FATAL_ERROR
  "The build is only configured for Linux, Windows(MINGW), and Darwin(Macos)."
    )
  endif()
  # Set pre_build user parameters
  set(DEPS_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/../external_lib")
  set(DEPS_DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/../external_download")
  set(DEPS_BUILD_DIR "${PROJECT_BINARY_DIR}/../external_build")

  message(STATUS "THE LOCAL BUILD SYSTEM NAME IS : ${CMAKE_SYSTEM_NAME}")
  add_subdirectory(cmake/SuperBuildLinux)
  return()
endif(USE_SUPERBUILD)


#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#------------ lapkt PROJECT cmake script ------------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

project(lapkt VERSION 2.0)

# FLAG THE LOCAL BUILD SYSTEM
message(STATUS "THE LOCAL BUILD SYSTEM NAME IS : ${CMAKE_SYSTEM_NAME}")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX_BUILD ON)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOS_BUILD ON)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND MINGW)
  set(WINDOWS_BUILD ON)
else()
  message(FATAL_ERROR
"The build is only configured for Linux, Windows(MINGW), and Darwin(Macos)."
  )
endif()
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#---------- SECTION 1.0  PROJECT OPTIONS ------------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

# LOG
message(STATUS "The CMAKE_INSTALL_PREFIX is " ${CMAKE_INSTALL_PREFIX})
message(STATUS "Building lapkt in " ${CMAKE_BUILD_TYPE} " mode")

#---- SECTION 1.1 CONFIGURE DEFAULT PARAMETER VALUES ----#
set(CMAKE_CXX_STANDARD 17)
# Global flag to cause add_library() to create shared libraries
set(BUILD_SHARED_LIBS ON)
# static link with stdc++ and libgcc
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
  set(CMAKE_SHARED_LINKER_FLAGS  "-static-libstdc++") 
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
  set(CMAKE_SHARED_LINKER_FLAGS  "-static-libstdc++ -static-libgcc") 
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
  message(WARNING "The build for Intel compiler hasn't been tested. Proceed with caution")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  message(WARNING "The build for MSVC hasn't been tested. Proceed with caution")
endif()
if(MINGW)
  # set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-multiple-definition -Wl,-Bstatic,--whole-archive -lpthread  -Wl,-Bdynamic,--no-whole-archive")
  # set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-multiple-definition -Wl,-Bstatic,--whole-archive -lpthread  -Wl,-Bdynamic,--no-whole-archive")
  set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-multiple-definition -static -lpthread") 
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-multiple-definition -static -lpthread")
endif(MINGW)
# The Find<Package>.cmake file are located here
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

#---- SECTION 1.2 Choose features to add to the lapkt library  ----#

option(CMAKE_FF_CXX "Include the pure CXX lapkt extensions to support usage of FF parser" OFF)

option(CMAKE_LEGACY_PLANNER "Old is gold! Build some legacy pure CXX planners" OFF)
if(CMAKE_LEGACY_PLANNER AND NOT CMAKE_FF_CXX)
  message(FATAL_ERROR "CMAKE_FF_CXX must be ON to install legacy planners")
endif()

if(CMAKE_FF_CXX)
  add_definitions(-DUSE_FF)
endif()

# Install FD pddl and include the wrapper over it which
#   acts as a pipe between output of FD tarnslate and lapkt
option(CMAKE_FD "Install FD pddl" ON)

# Builds a wrapper over lapkt library and installs a python module which 
# handles the intregration of Tarski grounder and lapkt library
option(CMAKE_TARSKI "Build and install the extension for tarski" ON)

# Create Sphinx Docs (pddld from Doxygen XML)
option(GEN_SPHINX_DOCS "GEN_SPHINX_DOCS" OFF)

# Create Doxygen html Docs
option(GEN_DOXYGEN_DOCS "GEN_DOXYGEN_DOCS" OFF)


# https://stackoverflow.com/a/17032841/7248295
# set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

#---- SECTION 1.2 CUSTOM METHODS ----#
function(cat FROM TO)
  file(READ ${FROM} CONTENTS)
  file(APPEND ${TO} "${CONTENTS}")
endfunction()

#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#---- SECTION 2.0 - External packages/libraries -----#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

#---- Python  ----#
# find_package(Python 3.6...<3.11  COMPONENTS Interpreter Development)
# if(${Python_VERSION} GREATER 3.10 OR ${Python_VERSION} LESS 3.6)
#     message(SEND_ERROR 
#         "INCOMPATIBLE PYTHON VERSION, expected 3.6.x < 3.9.x but found - " 
#         ${Python_VERSION})
# endif()
if(MACOS_BUILD)
  set(Python_FIND_FRAMEWORK "FIRST")
  set(ENV{DYLD_FRAMEWORK_PATH} "/usr/local/Frameworks/")
  # set(ENV{DYLD_FRAMEWORK_PATH} "/Applications/Xcode_14.0.1.app/Contents/Developer/Library/Frameworks")
endif(MACOS_BUILD)
if(NOT DEFINED PY_VERSION)
  find_package(Python COMPONENTS Interpreter Development)
else()
  string(REPLACE "." ";" PY_VERSION_PART ${PY_VERSION})
  LIST(GET PY_VERSION_PART 0 PY_VMAJOR)
  LIST(GET PY_VERSION_PART 1 PY_VMINOR)
  MATH(EXPR PY_VMINOR_NEXT "${PY_VMINOR} + 1")
  # find_package(Python ${PY_VMAJOR}...<${PY_VMINOR} COMPONENTS Interpreter Development)
  find_package(Python ${PY_VMAJOR}.${PY_VMINOR}...<${PY_VMAJOR}.${PY_VMINOR_NEXT} 
    COMPONENTS Interpreter Development)
endif()
# if(${Python_VERSION} VERSION_LESS 3.6.0)
#   message(SEND_ERROR 
#     "UNTESTED PYTHON VERSION, expected >3.6.x but found - " ${Python_VERSION})
# endif()

message("The PYTHON_VERSION is ${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}")

#---- Pybind11  ----#
# MUST BE CALLED AFTER FINDPYTHON: CMake 3.12+ (3.15+ recommended, 3.18.2+ ideal) 
# added a new module called FindPython that had a highly improved search algorithm 
# and modern targets and tools. If you use FindPython, pybind11 will detect this 
# and use the existing targets instead:

#Fetch path to Pybind11
execute_process(COMMAND ${Python_EXECUTABLE} -m pybind11 --cmakedir 
  OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
  ERROR_VARIABLE PYBIND11_CMAKE_DIR_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)

if(PYBIND11_CMAKE_DIR_ERROR)
  message(FATAL_ERROR "Failed to find pybind11 cmake directory from pip\n ERROR: ${PYBIND11_CMAKE_DIR_ERROR}")
endif()

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PYBIND11_CMAKE_DIR})
message(STATUS "PYBIND11 CMAKE_DIR PATH IS: ${PYBIND11_CMAKE_DIR}")
message(STATUS "CMAKE_MODULE_PATH IS: ${CMAKE_MODULE_PATH}")

set(pybind11_ROOT ${PYBIND11_CMAKE_DIR})
# set(PYBIND11_NOPYTHON ON)
find_package(pybind11 REQUIRED
  # As of cmake-v3.12 <PackageName>_ROOT is the first path that is checked
  # PATHS ${PYBIND11_CMAKE_DIR} 
  # NO_DEFAULT_PATH
)

#---- Boost ----#
# LOGS
message(STATUS "BOOST_ROOT is set to ${BOOST_ROOT}")
message(STATUS "BOOST_INCLUDEDIR is set to ${BOOST_INCLUDEDIR}")
message(STATUS "BOOST_LIBRARYDIR is set to ${BOOST_LIBRARYDIR}")
message(STATUS "CMAKE_MODULE_PATH is set to ${CMAKE_MODULE_PATH}")
set(Boost_VERBOSE ON)
set(Boost_DEBUG ON)

# Link with static Boost libraries instead of dynamic
option(CMAKE_STATIC_BOOST "Link with boost-c++ static libraries" ON)
if(CMAKE_STATIC_BOOST)
  set(Boost_USE_STATIC_LIBS ON)
  set(BOOST_PYTHON_STATIC_LIB ON)
endif(CMAKE_STATIC_BOOST)

message(STATUS "Boost_USE_STATIC_LIBS is ${Boost_USE_STATIC_LIBS}")
message(STATUS "BOOST_PYTHON_STATIC_LIB is ${BOOST_PYTHON_STATIC_LIB}")

# set(BOOST_ROOT C:/Boost)
# set(BOOST_INCLUDEDIR ${BOOST_ROOT}/include)
# set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib)
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "C:/Boost/lib/cmake/Boost-1.78.0")
list(APPEND Boost_Required_Components
  program_options
  random
)
find_package(Boost 
  REQUIRED COMPONENTS ${Boost_Required_Components} 
  # As of cmake-v3.12 <PackageName>_ROOT is the first path that is checked
  # PATHS ${Boost_ROOT} 
  # NO_DEFAULT_PATH
  )
message("THE BOOST INCLUDE dirs search path is " ${Boost_INCLUDE_DIRS} )

#---- Catch2 ----#

if(CMAKE_TESTING_ENABLED)
  find_package(Catch2 3 REQUIRED
  # As of cmake-v3.12 <PackageName>_ROOT is the first path that is checked
  # PATHS ${Catch2_ROOT} 
  # NO_DEFAULT_PATH
  )
  enable_testing()
endif()

#[[ Alternative FetchContent method to add Catch2 targets to the build
Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.0-preview4
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
]]

#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#------ SECTION 3.0  lapkt LIBRARIES and DOCS -------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

#------------ SECTION 3.1  CONFIG OPTIONS -------------#
# PATHS relative to CMAKE_INSTALL_PREFIX and CMAKE_BINARY_DIR
# lapkt cxx library
set(REL_LAPKT_ROOT lapkt)
set(REL_CMAKE_DIR ${REL_LAPKT_ROOT}/cmake)
# core and planner component
set(REL_CORE_LIB_DIR ${REL_LAPKT_ROOT}/core/lib)
set(REL_CORE_INC_DIR ${REL_LAPKT_ROOT}/core/include)
set(REL_PYEXT_LIB_DIR ${REL_LAPKT_ROOT}/core/lib)
set(REL_PYEXT_INC_DIR ${REL_LAPKT_ROOT}/core/include)
# pypi package
set(REL_PYPI_ROOT _package)
set(REL_PYPI_LAPKT_ROOT ${REL_PYPI_ROOT}/site_package/lapkt)

#[[
Note- When creating Python extensions, NEITHER the py extensions NOR the 
lapkt libs need to be linked to Python shared libraries. Including the
headers is sufficient. The reference to Python symbols will be populated by
when the library is loaded by the python interpreter. This would free the 
python extensions from VERSION specific Python shared libs.
]]

#------------- SECTION 3.2  BUILD--------------#
#[[
The 'lapkt' library is a pure cxx library containing the core structures,
and algorithms. 
All non-pure extensions should link with the 'liblapkt.so' shared library. 
This library should have minimal dependencies. 
]]

#---- Generate Source Documentation ----#
add_subdirectory(cmake/docs)

#---- DECLARE library targets and set properties ----#

#---- core - pure cxx component ----#
if(LINUX_BUILD)
  # set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};$ORIGIN")
elseif(WINDOWS_BUILD)
  # set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};$ORIGIN")
elseif(MACOS_BUILD)
  message(STATUS "Setting up RPATH on macos build")
  # message(STATUS "CMAKE_SKIP_RPATH TRUE")
  # set(CMAKE_SKIP_RPATH TRUE)
  # message(STATUS "CMAKE_MACOS_RPATH TRUE")
  # set(CMAKE_MACOSX_RPATH TRUE)
  # message(STATUS "CMAKE_INSTALL_NAME_DIR @executable_path")
  # set(CMAKE_INSTALL_NAME_DIR @executable_path)
  message(STATUS "CMAKE_INSTALL_NAME_DIR @loader_path")
  set(CMAKE_INSTALL_NAME_DIR @loader_path)
  # message(STATUS "CMAKE_BUILD_WITH_INSTALL_RPATH TRUE")
  # set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
  # message(STATUS "CMAKE_INSTALL_RPATH @executable_path")
  # set(CMAKE_INSTALL_RPATH @executable_path)
  # message(STATUS "CMAKE_INSTALL_RPATH @loader_path")
  # set(CMAKE_INSTALL_RPATH @loader_path)
  # message(STATUS "CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE")
  # set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

endif()

add_library(core SHARED)

# set_target_properties(core PROPERTIES
#     #### CHECK IT - Doesn't work in Windows(MINGW)
#   LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${REL_CORE_LIB_DIR}
# )

#---- Python extensions ----#

#---- CXX Wrappers over LAPKT lib ----#

if(WINDOWS_BUILD)
  pybind11_add_module(wrapper SHARED)
elseif(LINUX_BUILD)
  add_library(wrapper SHARED)
  target_link_libraries(wrapper PUBLIC
    pybind11::pybind11
  )
endif()

set_target_properties(wrapper PROPERTIES
  PREFIX ""
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN" # CHECK THIS FOR MACOS
)

target_link_libraries(wrapper PUBLIC
  core
  ${Boost_LIBRARIES}
)

#---- Translate PDDL ----#
if(WINDOWS_BUILD)
  pybind11_add_module(pddl SHARED)
elseif(LINUX_BUILD)
  add_library(pddl SHARED)
endif()
set_target_properties(pddl PROPERTIES
  PREFIX ""
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN"
)
target_link_libraries(pddl PRIVATE
  core
  wrapper
)

#---- Planner ----#
if(WINDOWS_BUILD)
  pybind11_add_module(planner SHARED)
elseif(LINUX_BUILD)
  add_library(planner SHARED)
endif()

set_target_properties(planner PROPERTIES
  PREFIX ""
  INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN"
)

target_link_libraries(planner PRIVATE
  core
  wrapper
  ${Boost_LIBRARIES}
)

# target_include_directories(planner PRIVATE
#   ${Python_INCLUDE_DIRS}
# )

#---- Add sub directories with source ----#

# Add src
add_subdirectory(src)

# Unit tests
if(CMAKE_TESTING_ENABLED)
  add_subdirectory(test)
endif()

#------------ SECTION 3.3  Install -------------#

# core
install(TARGETS core 
  EXPORT lapkt-core-Targets
  DESTINATION ${REL_CORE_LIB_DIR}
  COMPONENT lapkt
  INCLUDES DESTINATION
    ${REL_CORE_INC_DIR}
)
# Allows the developers to use find_package(lapkt COMPONENT core) 
#   to link lapkt library from their cmake project
install(
  EXPORT      lapkt-core-Targets
  FILE        lapkt-core-Targets.cmake
  DESTINATION ${REL_CMAKE_DIR}
  COMPONENT lapkt
)

# wrapper
install(TARGETS wrapper
  EXPORT lapkt-wrapper-Targets
  LIBRARY DESTINATION ${REL_PYEXT_LIB_DIR}
  COMPONENT lapkt    
  ARCHIVE DESTINATION ${REL_PYEXT_LIB_DIR}
  COMPONENT lapkt    
  RUNTIME  DESTINATION ${REL_PYEXT_LIB_DIR}
  COMPONENT lapkt    
  INCLUDES DESTINATION    
    ${REL_PYEXT_INC_DIR}
)
# Allows the developers to use find_package(lapkt COMPONENT planner) 
#   to link lapkt library from their cmake project
install(
  EXPORT      lapkt-wrapper-Targets
  FILE        lapkt-wrapper-Targets.cmake
  DESTINATION ${REL_CMAKE_DIR}
  COMPONENT lapkt
)
# planner
install(TARGETS planner
  EXPORT lapkt-planner-Targets
  DESTINATION ${REL_PYEXT_LIB_DIR}
  LIBRARY DESTINATION ${REL_PYEXT_LIB_DIR}
  COMPONENT lapkt    
  ARCHIVE DESTINATION ${REL_PYEXT_LIB_DIR}
  COMPONENT lapkt    
  RUNTIME  DESTINATION ${REL_PYEXT_LIB_DIR}
  COMPONENT lapkt    
  INCLUDES DESTINATION    
    ${REL_PYEXT_INC_DIR}
)
# Allows the developers to use find_package(lapkt COMPONENT planner) 
#   to link lapkt library from their cmake project
install(
  EXPORT      lapkt-planner-Targets
  FILE        lapkt-planner-Targets.cmake
  DESTINATION ${REL_CMAKE_DIR}
  COMPONENT lapkt
)
# # pddl
install(TARGETS pddl
  EXPORT lapkt-pddl-Targets
  LIBRARY DESTINATION ${REL_PYEXT_LIB_DIR}
  COMPONENT lapkt    
  ARCHIVE DESTINATION ${REL_PYEXT_LIB_DIR}
  COMPONENT lapkt    
  RUNTIME  DESTINATION ${REL_PYEXT_LIB_DIR}
  COMPONENT lapkt    
  INCLUDES DESTINATION    
    ${REL_PYEXT_INC_DIR}
)
# Allows the developers to use find_package(lapkt COMPONENT pddl) 
#   to link lapkt library from their cmake project
install(
  EXPORT      lapkt-pddl-Targets
  FILE        lapkt-pddl-Targets.cmake
  DESTINATION ${REL_CMAKE_DIR}
  COMPONENT lapkt
)

#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#----------- SECTION 4.0  INSTALL CONFIG ------------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

#---- SECTION 4.1 CMAKE EXPORT CONFIGURATION

## Allow developers using the lapkt library to fetch information, including library version
configure_file(${PROJECT_SOURCE_DIR}/cmake/lapkt_cmake_config.h.in 
  lapkt_cmake_config.h
)

install(FILES "${PROJECT_BINARY_DIR}/lapkt_cmake_config.h"
  DESTINATION ${REL_CORE_INC_DIR}
  COMPONENT lapkt 
)
include(CMakePackageConfigHelpers)
# generate the config file that includes the exports
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
  ${PROJECT_BINARY_DIR}/${REL_CMAKE_DIR}/lapktConfig.cmake
  INSTALL_DESTINATION ${REL_CMAKE_DIR}
  # NO_SET_AND_CHECK_MACRO
  # NO_CHECK_REQUIRED_COMPONENTS_MACRO
  # PATH_VARS INCLUDE_INSTALL_DIR 
)
# generate the version file for the config file
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/${REL_CMAKE_DIR}/lapktConfigVersion.cmake"
  VERSION "${lapkt_VERSION_MAJOR}.${lapkt_VERSION_MINOR}"
  COMPATIBILITY AnyNewerVersion
) 
install(FILES
  ${PROJECT_BINARY_DIR}/${REL_CMAKE_DIR}/lapktConfig.cmake
  ${PROJECT_BINARY_DIR}/${REL_CMAKE_DIR}/lapktConfigVersion.cmake
  DESTINATION ${REL_CMAKE_DIR}
  COMPONENT lapkt
)

#---- SECTION 4.2 CPACK CONFIGURATIOn

# include(InstallRequiredSystemLibraries)
# set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")
# set(CPACK_PACKAGE_VERSION_MAJOR "${lapkt_VERSION_MAJOR}")
# set(CPACK_PACKAGE_VERSION_MINOR "${lapkt_VERSION_MINOR}")
# include(CPack)

#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#--------- SECTION 5.0  POST BUILD THINGS -----------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

install(DIRECTORY
    ${PROJECT_BINARY_DIR}/${REL_PYPI_ROOT}/
    USE_SOURCE_PERMISSIONS
    DESTINATION ${REL_PYPI_ROOT}
    COMPONENT PYPACKAGE EXCLUDE_FROM_ALL 
)

install(PROGRAMS
    ${PROJECT_SOURCE_DIR}/src/python/lapkt.py
    DESTINATION ${CMAKE_SOURCE_DIR}/lapkt_package
    COMPONENT PYPACKAGE EXCLUDE_FROM_ALL 
)

install(PROGRAMS
${PROJECT_SOURCE_DIR}/src/python/lapkt.py
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lapkt_package
    COMPONENT PYPACKAGE EXCLUDE_FROM_ALL 
)

install(DIRECTORY
  ${CMAKE_INSTALL_PREFIX}/${REL_LAPKT_ROOT}/
  USE_SOURCE_PERMISSIONS
  DESTINATION ${REL_PYPI_LAPKT_ROOT}
  COMPONENT PYPACKAGE 
)
install(DIRECTORY
  ${CMAKE_INSTALL_PREFIX}/${REL_PYPI_LAPKT_ROOT}
  USE_SOURCE_PERMISSIONS
  DESTINATION ${CMAKE_SOURCE_DIR}/lapkt_package
  COMPONENT PYPACKAGE 
)

install(DIRECTORY
  ${CMAKE_INSTALL_PREFIX}/${REL_PYPI_LAPKT_ROOT}
  USE_SOURCE_PERMISSIONS
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lapkt_package
  COMPONENT PYPACKAGE
)

add_custom_command(
    TARGET planner POST_BUILD
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --component lapkt
    BYPRODUCTS ${REL_PYPI_LAPKT_ROOT}
)

add_custom_command(
  TARGET planner POST_BUILD
  COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --component PYPACKAGE
)

# The script will run during the installation(We want this at the bottom of all install target)
## Install lapkt using python pip module
install(SCRIPT ${PROJECT_SOURCE_DIR}/cmake/PostInstallScript.cmake)
