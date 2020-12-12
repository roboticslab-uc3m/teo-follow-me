# Authors: see AUTHORS.md at project root.
# CopyPolicy: released under the terms of the LGPLv2.1, see LICENSE at project root.
# URL: https://github.com/roboticslab-uc3m/follow-me

yarp_install(FILES applications/ymanager.ini
                   applications/follow-me_english_micro-off.xml
                   applications/follow-me_english_micro-off_sim.xml
                   applications/follow-me_english_micro-on.xml
                   applications/follow-me_spanish_micro-off.xml
                   applications/follow-me_spanish_micro-on.xml
             DESTINATION ${ROBOTICSLAB-FOLLOW-ME_APPLICATIONS_INSTALL_DIR})
