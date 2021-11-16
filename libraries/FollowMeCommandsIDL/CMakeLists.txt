option(ENABLE_FollowMeCommandsIDL "Enable/disable FollowMeCommandsIDL library" ON)

if(ENABLE_FollowMeCommandsIDL)

    set(ALLOW_IDL_GENERATION ON CACHE BOOL "Detect changes and rebuild IDL files")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS FollowMeCommands.thrift)

    yarp_idl_to_dir(INPUT_FILES FollowMeCommands.thrift
                    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
                    SOURCES_VAR idl_sources
                    HEADERS_VAR idl_headers
                    INCLUDE_DIRS_VAR idl_include_dirs
                    PLACEMENT MERGED
                    THRIFT_NO_NAMESPACE_PREFIX)

    add_library(FollowMeCommandsIDL SHARED ${idl_sources} ${idl_headers})

    set_target_properties(FollowMeCommandsIDL PROPERTIES PUBLIC_HEADER "${idl_headers}")

    target_link_libraries(FollowMeCommandsIDL PUBLIC YARP::YARP_os)

    target_include_directories(FollowMeCommandsIDL PUBLIC ${idl_include_dirs})

    install(TARGETS FollowMeCommandsIDL
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    add_library(ROBOTICSLAB::FollowMeCommandsIDL ALIAS FollowMeCommandsIDL)

endif()
