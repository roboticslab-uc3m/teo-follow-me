option(ENABLE_followMeArmExecution "Choose if you want to compile followMeArmExecution" ON)

if(ENABLE_followMeArmExecution)

    add_executable(followMeArmExecution main.cpp
                                        FollowMeArmExecution.hpp
                                        FollowMeArmExecution.cpp)

    target_link_libraries(followMeArmExecution YARP::YARP_os
                                               YARP::YARP_init
                                               YARP::YARP_dev)

    install(TARGETS followMeArmExecution
            DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()
