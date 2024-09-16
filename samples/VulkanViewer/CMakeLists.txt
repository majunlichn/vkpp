set(VulkanViewer_SOURCE_FILES
    main.cpp
    VulkanViewer.h
    VulkanViewer.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VulkanViewer_SOURCE_FILES})

add_executable(VulkanViewer
    ${VulkanViewer_SOURCE_FILES}
)

set_target_properties(VulkanViewer PROPERTIES FOLDER "samples")

target_link_libraries(VulkanViewer
    PRIVATE vkpp
)

add_custom_command(TARGET VulkanViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:VulkanViewer> $<TARGET_FILE_DIR:VulkanViewer>
    COMMAND_EXPAND_LISTS
)
