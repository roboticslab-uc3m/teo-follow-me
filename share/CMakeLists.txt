# Copyright: UC3M 2016
# Authors: Juan G Victores & Raul de Santos Rico
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
# 

find_package(YARP REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
include(YarpInstallationHelpers)
yarp_configure_external_installation(follow-me)

### Go through single applications

#add_subdirectory(prog)

