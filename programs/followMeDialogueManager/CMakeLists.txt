if(NOT TARGET ROBOTICSLAB::SpeechIDL AND (NOT DEFINED ENABLE_followMeDialogueManager OR ENABLE_followMeDialogueManager))
    message(WARNING "ROBOTICSLAB::SpeechIDL target not found, disabling followMeDialogueManager")
endif()

cmake_dependent_option(ENABLE_followMeDialogueManager "Choose if you want to compile followMeDialogueManager" ON
                       "ENABLE_FollowMeCommandsIDL;TARGET ROBOTICSLAB::SpeechIDL" OFF)

if(ENABLE_followMeDialogueManager)

    add_executable(followMeDialogueManager main.cpp
                                           FollowMeDialogueManager.hpp
                                           FollowMeDialogueManager.cpp)

    target_link_libraries(followMeDialogueManager YARP::YARP_os
                                                  YARP::YARP_init
                                                  ROBOTICSLAB::SpeechIDL
                                                  ROBOTICSLAB::FollowMeCommandsIDL)

    install(TARGETS followMeDialogueManager
            DESTINATION ${CMAKE_INSTALL_BINDIR})

else()

    set(ENABLE_followMeDialogueManager OFF CACHE BOOL "Enable/disable followMeDialogueManager program" FORCE)

endif()
