qt_add_executable(docking_drag_drop_test
    docking_drag_drop_test.cpp
)

qt_add_executable(folder_maximize_test
    folder_maximize_test.cpp
)

qt_add_executable(folder_restore_state_test
    folder_restore_state_test.cpp
)

qt_add_executable(tab_action_state_test
    tab_action_state_test.cpp
)

qt_add_executable(tab_folder_features_test
    tab_folder_features_test.cpp
)

qt_add_executable(tab_folder_mouse_test
    tab_folder_mouse_test.cpp
)

qt_add_executable(tab_folder_right_widget_test
    tab_folder_right_widget_test.cpp
)

qt_add_executable(tab_list_widget_test
    tab_list_widget_test.cpp
)

foreach(target IN ITEMS
    docking_drag_drop_test
    folder_maximize_test
    folder_restore_state_test
    tab_action_state_test
    tab_folder_features_test
    tab_folder_mouse_test
    tab_folder_right_widget_test
    tab_list_widget_test
)
    target_link_libraries(${target} PRIVATE
        qcwdocking
        Qt::Widgets
    )

    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QCWDOCK_TEST_AUTOCLOSE=1"
    )
endforeach()

# The drag-and-drop test needs a real QDrag::exec() nested loop to make the
# dragged folder the source QDropEvent::source() reports. The offscreen
# plugin's QOffscreenDrag::drag() only returns IgnoreAction, while the
# minimal plugin's QSimpleDrag runs the loop.
set_tests_properties(docking_drag_drop_test PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=minimal;QCWDOCK_TEST_AUTOCLOSE=1"
)
