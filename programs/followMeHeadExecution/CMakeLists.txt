cmake_dependent_option(ENABLE_followMeHeadExecution "Choose if you want to compile followMeHeadExecution" ON
                       ENABLE_FollowMeCommandsIDL OFF)

if(ENABLE_followMeHeadExecution)

    add_executable(followMeHeadExecution main.cpp
                                         FollowMeHeadExecution.hpp
                                         FollowMeHeadExecution.cpp)

    target_link_libraries(followMeHeadExecution YARP::YARP_os
                                                YARP::YARP_init
                                                YARP::YARP_dev
                                                ROBOTICSLAB::FollowMeCommandsIDL)

    install(TARGETS followMeHeadExecution)

endif()
