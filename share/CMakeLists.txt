# Authors: see AUTHORS.md at project root.
# CopyPolicy: released under the terms of the LGPLv2.1, see LICENSE at project root.
# URL: https://github.com/roboticslab-uc3m/follow-me

yarp_install(FILES applications/ymanager.ini
                   applications/follow-me_english_with_microphone.xml
                   applications/follow-me_spanish_with_microphone.xml
                   applications/follow-me_english.xml
                   applications/follow-me_spanish.xml
             DESTINATION ${ROBOTICSLAB-FOLLOW-ME_APPLICATIONS_INSTALL_DIR})
