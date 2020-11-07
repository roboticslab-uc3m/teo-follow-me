# Authors: see AUTHORS.md at project root.
# CopyPolicy: released under the terms of the LGPLv2.1, see LICENSE at project root.
# URL: https://github.com/roboticslab-uc3m/follow-me

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(FOLLOW_ME LANGUAGES CXX)

### options: cpp programs
option(ENABLE_followMeArmSwing "Choose if you want to compile followMeArmSwing" TRUE)
option(ENABLE_followMeDialogueManager "Choose if you want to compile followMeDialogueManager" TRUE)
option(ENABLE_followMeExecutionCore "Choose if you want to compile followMeExecutionCore" TRUE)

# Add suffix for debug libraries.
if(MSVC)
    message(STATUS "Running on windows")
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

# Let the user specify a configuration (only single-config generators).
if(NOT CMAKE_CONFIGURATION_TYPES)
    # Possible values.
    set(_configurations Debug Release MinSizeRel RelWithDebInfo)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${_configurations})

    foreach(_conf ${_configurations})
        set(_conf_string "${_conf_string} ${_conf}")
    endforeach()

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING
                 "Choose the type of build, options are:${_conf_string}")

    if(NOT CMAKE_BUILD_TYPE)
        # Encourage the user to specify build type.
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE Release)
    endif()
endif()

# Pick up our cmake modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake
                              ${CMAKE_SOURCE_DIR}/cmake/find-modules)

# Hard dependencies.
find_package(YCM 0.11 REQUIRED)
find_package(YARP 3.3 REQUIRED COMPONENTS os dev)

# Configure installation paths for YARP resources.
yarp_configure_external_installation(roboticslab-follow-me)

# add main contents
add_subdirectory(share)
add_subdirectory(programs)

# Configure and create uninstall target.
include(AddUninstallTarget)
