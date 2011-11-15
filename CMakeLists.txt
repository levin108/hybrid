project(hybrid)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
set(TARNAME hybrid)


# program name, version etc

set(PACKAGE_VERSION "0.0.1")
set(PACKAGE_NAME "Hybird")
set(PACKAGE_TARNAME "${TARNAME}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "http://code.google.com/p/ofetion/issues/list")

find_package(PkgConfig REQUIRED)

if(NOT DEFINED BIN_INSTALL_DIR)
    set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
endif(NOT DEFINED BIN_INSTALL_DIR)

if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif(NOT DEFINED LIB_INSTALL_DIR)

if(NOT DEFINED DATA_INSTALL_DIR)
    set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/${PACKAGE_TARNAME}")
endif(NOT DEFINED DATA_INSTALL_DIR)

if(NOT DEFINED INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/${PACKAGE_TARNAME}")
endif(NOT DEFINED INCLUDE_INSTALL_DIR)

if(NOT DEFINED UI_INSTALL_DIR)
    set(UI_INSTALL_DIR "${DATA_INSTALL_DIR}/ui/")
endif(NOT DEFINED UI_INSTALL_DIR)

if(NOT DEFINED MAN_INSTALL_DIR)
    set(MAN_INSTALL_DIR "${DATA_INSTALL_DIR}/man")
endif(NOT DEFINED MAN_INSTALL_DIR)

if(NOT DEFINED CERTS_INSTALL_DIR)
	set(CERTS_INSTALL_DIR "$DATA_INSTALL_DIR/ca-certs")
endif(NOT DEFINED CERTS_INSTALL_DIR)

if(NOT DEFINED LOCALE_INSTALL_DIR)
    set(LOCALE_INSTALL_DIR "${DATA_INSTALL_DIR}/locale/")
endif(NOT DEFINED LOCALE_INSTALL_DIR)

if(NOT DEFINED MODULE_INSTALL_DIR)
	set(MODULE_INSTALL_DIR "${LIB_INSTALL_DIR}/${PACKAGE_TARNAME}/protocols")
endif(NOT DEFINED MODULE_INSTALL_DIR)

add_definitions(-DRESOURCE_DIR="${RESOURCE_INSTALL_DIR}")
add_definitions(-DDESKTOP_DIR="${DESKTOP_INSTALL_DIR}")
add_definitions(-DDATA_DIR="${DATA_INSTALL_DIR}")
add_definitions(-DPIXMAPS_DIR="${DATA_INSTALL_DIR}/pixmaps/")
add_definitions(-DSOUND_DIR="${DATA_INSTALL_DIR}/sound/")
add_definitions(-DUI_DIR="${UI_INSTALL_DIR}")
add_definitions(-DCERTS_DIR="${DATA_INSTALL_DIR}/ca-certs/")
add_definitions(-DCCERT_DIR="${DATA_INSTALL_DIR}/c-cert")

# compile option switch
# option(NLS "Native language support" ON)
option(WITH_LIBNOTIFY "Libnotify support" ON)
option(WITH_GSTREAMER "GStreamer support" ON)
option(WITH_LIBXSS "LibXScrnSaver support" ON)
option(WITH_WEBKIT "Webkit support" ON)
option(WITH_NETWORKMANAGER "NetworkManager support" ON)
option(WITH_INDICATE "Ubuntu Indicator support" ON)

# im build requirements
find_package(PkgConfig REQUIRED)
if(NLS)
	find_package(Gettext)
endif(NLS)

# im build requirements
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
pkg_check_modules(GDKPIXBUF REQUIRED gdk-pixbuf-2.0)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GOBJECT2 REQUIRED gobject-2.0)
pkg_check_modules(GMODULE2 REQUIRED gmodule-2.0)
pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

if(WITH_LIBNOTIFY)
    pkg_check_modules(LIBNOTIFY libnotify)
endif(WITH_LIBNOTIFY)

if(WITH_GSTREAMER)
    pkg_check_modules(GSTREAMER gstreamer-0.10)
endif(WITH_GSTREAMER)

if(WITH_LIBXSS)
    pkg_check_modules(LIBXSS xscrnsaver)
endif(WITH_LIBXSS)

if(WITH_WEBKIT)
	pkg_check_modules(WEBKIT webkit-1.0)
endif(WITH_WEBKIT)

if(WITH_NETWORKMANAGER)
    pkg_check_modules(NETWORKMANAGER NetworkManager)
    pkg_check_modules(DBUSGLIB dbus-glib-1)
endif(WITH_NETWORKMANAGER)

if(WITH_INDICATE)
	pkg_check_modules(INDICATE indicate)
	pkg_check_modules(INDICATE_GTK indicate-gtk)
	pkg_check_modules(INDICATE_GTK_NEW indicate-gtk-0.5)
endif(WITH_INDICATE)

if(WITH_LIBNOTIFY AND LIBNOTIFY_FOUND)
    set(USE_LIBNOTIFY 1)
    if(LIBNOTIFY_VERSION LESS 0.7)
    	set(LIBNOTIFY_OLD 1)
    	message(STATUS "Libnotify support:        OLD" )
    else(LIBNOTIFY_VERSION LESS 0.7)
    	message(STATUS "Libnotify support:        NEW" )
    endif(LIBNOTIFY_VERSION LESS 0.7)
else(WITH_LIBNOTIFY AND LIBNOTIFY_FOUND)
    message(STATUS "Libnotify support:        NO" )
endif(WITH_LIBNOTIFY AND LIBNOTIFY_FOUND)

if(WITH_GSTREAMER AND GSTREAMER_FOUND)
    set(USE_GSTREAMER 1)
    message(STATUS "GStreamer support:        YES" )
else(WITH_GSTREAMER AND GSTREAMER_FOUND)
    message(STATUS "GStreamer support:        NO" )
endif(WITH_GSTREAMER AND GSTREAMER_FOUND)

if(WITH_LIBXSS AND LIBXSS_FOUND)
    set(USE_LIBXSS 1)
    message(STATUS "Xscreensaver support:     YES" )
else(WITH_LIBXSS AND LIBXSS_FOUND)
    message(STATUS "Xscreensaver support:     NO" )
endif(WITH_LIBXSS AND LIBXSS_FOUND)

if(WITH_WEBKIT AND WEBKIT_FOUND)
	set(USE_WEBKIT 1)
	message(STATUS "WebKit support:           YES" )
else(WITH_WEBKIT AND WEBKIT_FOUND)
	message(STATUS "WebKit support:           NO" )
endif(WITH_WEBKIT AND WEBKIT_FOUND)

if(WITH_NETWORKMANAGER AND NETWORKMANAGER_FOUND AND DBUSGLIB_FOUND)
    set(USE_NETWORKMANAGER 1)
    message(STATUS "NetworkManager support:   YES" )
else(WITH_NETWORKMANAGER AND NETWORKMANAGER_FOUND AND DBUSGLIB_FOUND)
    message(STATUS "NetworkManager support:   NO" )
endif(WITH_NETWORKMANAGER AND NETWORKMANAGER_FOUND AND DBUSGLIB_FOUND)

if(WITH_INDICATE AND INDICATE_FOUND)
	if(INDICATE_GTK_FOUND OR INDICATE_GTK_NEW_FOUND)
		set(USE_INDICATE 1)
		message(STATUS "Ubuntu Indicator support: YES" )
	else(INDICATE_GTK_FOUND OR INDICATE_GTK_NEW_FOUND)
		message(STATUS "Ubuntu Indicator support: NO" )
	endif(INDICATE_GTK_FOUND OR INDICATE_GTK_NEW_FOUND)
else(WITH_INDICATE AND INDICATE_FOUND)
    message(STATUS "Ubuntu Indicator support: NO" )
endif(WITH_INDICATE AND INDICATE_FOUND)

configure_file(config-hybrid.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

##########################################################################


add_definitions(-g -Wall)

#if(NLS AND GETTEXT_FOUND)
#	add_definitions(-DGETTEXT_PACKAGE="${PACKAGE_TARNAME}")
#	add_definitions(-DENABLE_NLS="1")
#	message(STATUS "Native language support:  YES" )
#else(NLS AND GETTEXT_FOUND)
#	message(STATUS "Native language support:  NO" )
#endif(NLS AND GETTEXT_FOUND)

add_definitions(-DLOCALE_DIR="${LOCALE_INSTALL_DIR}")
add_definitions(-DMODULE_DIR="${MODULE_INSTALL_DIR}")
#add_definitions(-DMODULE_DIR=".")

##########################################################################

include_directories(
    ${GTK2_INCLUDE_DIRS}
    ${GDKPIXBUF_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${GOBJECT2_INCLUDE_DIRS}
    ${GMODULE2_INCLUDE_DIRS}
    ${OFETION_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

if(WITH_LIBNOTIFY AND LIBNOTIFY_FOUND)
    include_directories(${LIBNOTIFY_INCLUDE_DIRS})
endif(WITH_LIBNOTIFY AND LIBNOTIFY_FOUND)

if(WITH_GSTREAMER AND GSTREAMER_FOUND)
    include_directories(${GSTREAMER_INCLUDE_DIRS})
endif(WITH_GSTREAMER AND GSTREAMER_FOUND)

if(WITH_NETWORKMANAGER AND NETWORKMANAGER_FOUND)
    include_directories(${NETWORKMANAGER_INCLUDE_DIRS})
    include_directories(${DBUSGLIB_INCLUDE_DIRS})
endif(WITH_NETWORKMANAGER AND NETWORKMANAGER_FOUND)

if (WITH_WEBKIT AND WEBKIT_FOUND)
	include_directories(${WEBKIT_INCLUDE_DIRS})
endif(WITH_WEBKIT AND WEBKIT_FOUND)

if(WITH_INDICATE AND INDICATE_FOUND AND INDICATE_GTK_FOUND)
    include_directories(${INDICATE_INCLUDE_DIRS})
    include_directories(${INDICATE_GTK_INCLUDE_DIRS})
endif(WITH_INDICATE AND INDICATE_FOUND AND INDICATE_GTK_FOUND)

set(hybrid_SRC
	lib/connect.c
	lib/network.c
	lib/debug.c
	lib/eventloop.c
	lib/module.c
	lib/account.c
	lib/xmlnode.c
	lib/config.c
	lib/util.c
	lib/pref.c
	lib/logs.c
	src/gtkconn.c
	src/gtksound.c
	src/gtkutils.c
	src/gtkaccount.c
	src/blist.c
	src/conv.c
	src/info.c
	src/head.c
	src/action.c
	src/notify.c
	src/tooltip.c
	src/preference.c
	src/chat-textview.c
	src/chat-webkit.c
	src/groupadd.c
	src/buddyadd.c
	src/buddyreq.c
	src/statusicon.c
	src/searchbox.c
	src/confirm.c
	src/gtkcellrendererexpander.c
	src/core.c
)

add_executable(hybrid ${hybrid_SRC})
message(STATUS ${GLIB2_LIBRARIES})
target_link_libraries(hybrid 
	${GOBJECT2_LIBRARIES}
	${GMODULE2_LIBRARIES}
	${GTK2_LIBRARIES}
	${GLIB2_LIBRARIES}
	${GDKPIXBUF_LIBRARIES}
	${GTHREAD2_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${LIBXML2_LIBRARIES})

if(WITH_LIBNOTIFY AND LIBNOTIFY_FOUND)
    target_link_libraries(hybrid ${LIBNOTIFY_LIBRARIES})
endif(WITH_LIBNOTIFY AND LIBNOTIFY_FOUND)

if(WITH_GSTREAMER AND GSTREAMER_FOUND)
    target_link_libraries(hybrid ${GSTREAMER_LIBRARIES})
endif(WITH_GSTREAMER AND GSTREAMER_FOUND)

if(WITH_LIBXSS AND LIBXSS_FOUND)
    target_link_libraries(hybrid ${LIBXSS_LIBRARIES})
endif(WITH_LIBXSS AND LIBXSS_FOUND)

if (WITH_WEBKIT AND WEBKIT_FOUND)
	target_link_libraries(hybrid ${WEBKIT_LIBRARIES})
endif(WITH_WEBKIT AND WEBKIT_FOUND)

if(WITH_INDICATE AND INDICATE_FOUND AND INDICATE_GTK_FOUND)
    target_link_libraries(hybrid ${INDICATE_LIBRARIES})
    target_link_libraries(hybrid ${INDICATE_GTK_LIBRARIES})
endif(WITH_INDICATE AND INDICATE_FOUND AND INDICATE_GTK_FOUND)

if(WITH_NETWORKMANAGER AND NETWORKMANAGER_FOUND AND DBUSGLIB_FOUND)
    target_link_libraries(hybrid ${NETWORKMANAGER_LIBRARIES})
    target_link_libraries(hybrid ${DBUSGLIB_LIBRARIES})
endif(WITH_NETWORKMANAGER AND NETWORKMANAGER_FOUND AND DBUSGLIB_FOUND)

install(TARGETS hybrid RUNTIME DESTINATION ${BIN_INSTALL_DIR})

# install include files
file(GLOB HYBRID_INCLUDES "src/*.h" "lib/*.h")
install(FILES ${HYBRID_INCLUDES} DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

# install menus pixmaps
file(GLOB HYBRID_MENUS_PIXMAPS "share/pixmaps/menus/*.png")
install(FILES ${HYBRID_MENUS_PIXMAPS} DESTINATION "${DATA_INSTALL_DIR}/pixmaps/menus/")

# install protocols pixmaps
file(GLOB HYBRID_PROTOCOLS_PIXMAPS "share/pixmaps/protocols/*.png")
install(FILES ${HYBRID_PROTOCOLS_PIXMAPS} DESTINATION "${DATA_INSTALL_DIR}/pixmaps/protocols/")

# install status pixmaps
file(GLOB HYBRID_STATUS_PIXMAPS "share/pixmaps/status/*.png")
install(FILES ${HYBRID_STATUS_PIXMAPS} DESTINATION "${DATA_INSTALL_DIR}/pixmaps/status/")

# install icons pixmaps
file(GLOB HYBRID_ICONS_PIXMAPS "share/pixmaps/icons/*.png")
install(FILES ${HYBRID_ICONS_PIXMAPS} DESTINATION "${DATA_INSTALL_DIR}/pixmaps/icons/")

# install sound files 
file(GLOB HYBRID_ICONS_PIXMAPS "share/sound/*.wav")
install(FILES ${HYBRID_ICONS_PIXMAPS} DESTINATION "${DATA_INSTALL_DIR}/sound/")

# install ui files 
file(GLOB HYBRID_ICONS_PIXMAPS "src/ui/*")
install(FILES ${HYBRID_ICONS_PIXMAPS} DESTINATION "${UI_INSTALL_DIR}")

#install ca certs files
file(GLOB HYBRID_CA_CERTS "share/ca-certs/*")
install(FILES ${HYBRID_CA_CERTS} DESTINATION "${DATA_INSTALL_DIR}/ca-certs/")

#include client certs files
file(GLOB HYBRID_CLIENT_CERT "share/c-cert/*")
install(FILES ${HYBRID_CLIENT_CERT} DESTINATION "${DATA_INSTALL_DIR}/c-cert/")

# source list of fetion module
set(fetion_SRC
	modules/fetion/fetion.c
	modules/fetion/fx_login.c
	modules/fetion/fx_account.c
	modules/fetion/fx_sip.c
	modules/fetion/fx_group.c
	modules/fetion/fx_buddy.c
	modules/fetion/fx_trans.c
	modules/fetion/fx_util.c
	modules/fetion/fx_msg.c
	modules/fetion/fx_config.c
)

# install fetion resource xml files
file(GLOB FETION_RESOURCE_XML "share/protocols/fetion/*.xml")
install(FILES ${FETION_RESOURCE_XML} DESTINATION "${DATA_INSTALL_DIR}/protocols/fetion/")
add_definitions(-DFETION_RES_DIR="${DATA_INSTALL_DIR}/protocols/fetion/")

add_library(fetion SHARED ${fetion_SRC})

# install fetion module
install(TARGETS fetion
	LIBRARY DESTINATION ${MODULE_INSTALL_DIR}
	ARCHIVE DESTINATION ${MODULE_INSTALL_DIR}
)

# source list of xmpp module
set(xmpp_SRC
	modules/xmpp/xmpp.c
	modules/xmpp/xmpp_util.c
	modules/xmpp/xmpp_stream.c
	modules/xmpp/xmpp_parser.c
	modules/xmpp/xmpp_account.c
	modules/xmpp/xmpp_buddy.c
	modules/xmpp/xmpp_iq.c
	modules/xmpp/xmpp_message.c
)

add_library(xmpp SHARED ${xmpp_SRC})

# install fetion module
install(TARGETS xmpp
	LIBRARY DESTINATION ${MODULE_INSTALL_DIR}
	ARCHIVE DESTINATION ${MODULE_INSTALL_DIR}
)
