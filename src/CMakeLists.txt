# ----- photostudio_core: Qt-free engine (layers, filters, tools, IO) -----

add_library(photostudio_core STATIC
    core/blend_modes.cpp
    core/blend_modes.hpp
    core/document.cpp
    core/document.hpp
    core/layer.cpp
    core/layer.hpp
    core/types.hpp
    filters/artistic/artistic_filters.cpp
    filters/artistic/artistic_filters.hpp
    filters/blur/blur_filters.cpp
    filters/blur/blur_filters.hpp
    filters/color/color_filters.cpp
    filters/color/color_filters.hpp
    filters/detail/detail_filters.cpp
    filters/detail/detail_filters.hpp
    filters/distortion/distortion_filters.cpp
    filters/distortion/distortion_filters.hpp
    filters/filter_base.hpp
    filters/filter_registry.cpp
    filters/filter_registry.hpp
    filters/filter_utils.hpp
    filters/register_builtin.cpp
    filters/render/render_filters.cpp
    filters/render/render_filters.hpp
    io/image_io.cpp
    io/image_io.hpp
    io/project_serializer.cpp
    io/project_serializer.hpp
    math/color.hpp
    math/interpolation.hpp
    performance/thread_pool.hpp
    tools/brush/brush_tools.cpp
    tools/brush/brush_tools.hpp
    tools/color/color_picker_tool.cpp
    tools/color/color_picker_tool.hpp
    tools/fill/fill_tools.cpp
    tools/fill/fill_tools.hpp
    tools/selection/selection_tools.cpp
    tools/selection/selection_tools.hpp
    tools/tool_base.hpp
    tools/transform/move_tool.cpp
    tools/transform/move_tool.hpp
    utils/logger.cpp
    utils/logger.hpp
)

target_include_directories(photostudio_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(photostudio_core PUBLIC
    ${OpenCV_LIBS}
    Threads::Threads
)

# ----- PhotoStudio: Qt6 application -----

qt_add_executable(PhotoStudio
    main.cpp
    ui/canvas_widget.cpp
    ui/canvas_widget.hpp
    ui/dialogs/export_dialog.cpp
    ui/dialogs/export_dialog.hpp
    ui/dialogs/filter_dialog.cpp
    ui/dialogs/filter_dialog.hpp
    ui/dialogs/new_document_dialog.cpp
    ui/dialogs/new_document_dialog.hpp
    ui/dialogs/resize_dialog.cpp
    ui/dialogs/resize_dialog.hpp
    ui/image_convert.hpp
    ui/main_window.cpp
    ui/main_window.hpp
    ui/panels/colors_panel.cpp
    ui/panels/colors_panel.hpp
    ui/panels/history_panel.cpp
    ui/panels/history_panel.hpp
    ui/panels/layers_panel.cpp
    ui/panels/layers_panel.hpp
    ui/panels/properties_panel.cpp
    ui/panels/properties_panel.hpp
    ui/theme/theme_engine.cpp
    ui/theme/theme_engine.hpp
    ${CMAKE_SOURCE_DIR}/assets/resources.qrc
)

target_link_libraries(PhotoStudio PRIVATE
    photostudio_core
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
)

set_target_properties(PhotoStudio PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

install(TARGETS PhotoStudio RUNTIME DESTINATION bin)
