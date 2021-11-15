option(ENABLE_followMeDialogueManager "Choose if you want to compile followMeDialogueManager" ON)

if(ENABLE_followMeDialogueManager)

    add_executable(followMeDialogueManager main.cpp
                                           FollowMeDialogueManager.hpp
                                           FollowMeDialogueManager.cpp
                                           StateMachine.hpp
                                           StateMachine.cpp)

    target_link_libraries(followMeDialogueManager YARP::YARP_os
                                                  YARP::YARP_init)

    install(TARGETS followMeDialogueManager
            DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()
