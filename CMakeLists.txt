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

if(NOT DEFINED MAN_INSTALL_DIR)
    set(MAN_INSTALL_DIR "${DATA_INSTALL_DIR}/man")
endif(NOT DEFINED MAN_INSTALL_DIR)

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

# im build requirements
find_package(PkgConfig REQUIRED)
if(NLS)
	find_package(Gettext)
endif(NLS)

# im build requirements
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
pkg_check_modules(GDKPIXBUF REQUIRED gdk-pixbuf-2.0)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GMODULE2 REQUIRED gmodule-2.0)
pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

##########################################################################
add_definitions(-g -Wall)
if(NLS AND GETTEXT_FOUND)
	add_definitions(-DGETTEXT_PACKAGE="${PACKAGE_TARNAME}")
	add_definitions(-DENABLE_NLS="1")
	message(STATUS "Native language support:  YES" )
else(NLS AND GETTEXT_FOUND)
	message(STATUS "Native language support:  NO" )
endif(NLS AND GETTEXT_FOUND)
add_definitions(-DLOCALE_DIR="${LOCALE_INSTALL_DIR}")
#add_definitions(-DMODULE_DIR="${MODULE_INSTALL_DIR}")
add_definitions(-DMODULE_DIR=".")
add_definitions(-DUI_DIR="../src/ui/")


##########################################################################

include_directories(
    ${GTK2_INCLUDE_DIRS}
    ${GDKPIXBUF_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${GMODULE2_INCLUDE_DIRS}
    ${OFETION_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

set(hybrid_SRC
	lib/connect.c
	lib/network.c
	lib/debug.c
	lib/eventloop.c
	lib/module.c
	lib/account.c
	lib/xmlnode.c
	lib/config.c
	src/gtkutils.c
	src/gtkaccount.c
	src/blist.c
	src/conv.c
	src/info.c
	src/notify.c
	src/gtkcellrendererexpander.c
	src/core.c
)

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

add_executable(hybrid ${hybrid_SRC})
target_link_libraries(hybrid ${GTK2_LIBRARIES} ${GDKPIXBUF_LIBRARIES} ${GTHREAD2_LIBRARIES} ${GMODULE2_LIBRARIES} ${OPENSSL_LIBRARIES} ${LIBXML2_LIBRARIES})

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
