cmake_minimum_required(VERSION 2.8)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include_directories(${KDE4_INCLUDES})

# We are calling our plugin "ktexteditor_neznaju", and it contains only
# one source file: neznaju.cpp.
kde4_add_plugin(ktexteditor_neznaju neznaju.cpp neznajuPlugin.cpp )

# We need to link our plugin against kdecore libs, as well as ktexteditor
target_link_libraries(ktexteditor_neznaju ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ktexteditor ${QT_QTNETWORK_LIBRARY})

# Well, we want to install our plugin on the plugin directory
install(TARGETS ktexteditor_neznaju DESTINATION ${PLUGIN_INSTALL_DIR})

# We also want to install the resource contents file on the data directory, at
# the subdirectory of our plugin name, so it does not mix up with other resource
# contents files.
install(FILES neznajuui.rc DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_neznaju)

# We want to install the desktop file that describes our plugin too. It will go
# on the services directory.
install(FILES ktexteditor_neznaju.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(QT_USE_QTNETWORK TRUE)
set(CMAKE_CXX_FLAGS "-std=c++0x")
