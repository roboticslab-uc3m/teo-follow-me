# Authors: see AUTHORS.md at project root.
# CopyPolicy: released under the terms of the LGPLv2.1, see LICENSE at project root.
# URL: https://github.com/roboticslab-uc3m/teo-follow-me

yarp_install(FILES applications/ymanager.ini
                   applications/teo-follow-me_english_micro-off_sim.xml
                   applications/teo-follow-me_english_micro-off.xml
                   applications/teo-follow-me_english_micro-on_sim.xml
                   applications/teo-follow-me_english_micro-on.xml
                   applications/teo-follow-me_spanish_micro-off_sim.xml
                   applications/teo-follow-me_spanish_micro-off.xml
                   applications/teo-follow-me_spanish_micro-on_sim.xml
                   applications/teo-follow-me_spanish_micro-on.xml
             DESTINATION ${TEO-FOLLOW-ME_APPLICATIONS_INSTALL_DIR})
