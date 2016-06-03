CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(apptray-widget C)

SET(SRCS
		src/app-widget-app.c
        src/key.c
        src/util.c
        src/common/home_custom_index.c
        src/common/home_custom_scroller.c
)

SET(VENDOR "tizen")
SET(PACKAGE ${PROJECT_NAME})
SET(PKGNAME "org.${VENDOR}.${PACKAGE}")
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
#SET(INSTALL_DIR_APPS "/usr/apps/${PKGNAME}")
SET(INSTALL_DIR_PRE "/usr")

SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(IMAGEDIR "${RESDIR}/image/${PROJECT_NAME}")
SET(IMAGEDIR2 "${RESDIR}/edje/images")
SET(DATADIR "${PREFIX}/data")
SET(LOCALEDIR "${RESDIR}/locale")
SET(ICONDIR "${RESDIR}/icons")
SET(SOUNDDIR "${RESDIR}/sounds")
SET(EDJDIR "${RESDIR}/edje")
SET(SHARED_RES "${PREFIX}/shared/res")

IF("${ARCH}" STREQUAL "arm")
        ADD_DEFINITIONS("-DARM_TYPE")
ELSE()
        ADD_DEFINITIONS("-DEMULATOR_TYPE")
ENDIF()
MESSAGE("ARCH: ${ARCH}")


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

SET(GC_SECTIONS_FLAGS "-fdata-sections -ffunction-sections -Wl,--gc-sections")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GC_SECTIONS_FLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GC_SECTIONS_FLAGS}")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/inc)


INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED 
	#ail
	appcore-efl
	aul
	bundle
	capi-appfw-application
	capi-appfw-package-manager
	capi-system-system-settings
	capi-appfw-widget-application
	#csc-feature
	dlog
	ecore
	#ecore-x
	ecore-evas
	ecore-file
	ecore-imf
	ecore-input
	edje
	eet
	efl-assist
	elementary
	eina
	evas
	#widget
	widget_viewer
	widget_viewer_evas
	widget_service
	pkgmgr
	pkgmgr-info
	efl-extension
	#ui-extension
	#utilX
	vconf
	appsvc
	feedback
)
#??
FIND_LIBRARY(LIB_M m)

ADD_DEFINITIONS(${pkgs_CFLAGS})
ADD_DEFINITIONS("-fpie")
ADD_DEFINITIONS("-DRSSI_ANT_BAR=${RSSI_ANT_BAR}")
MESSAGE("RSSI_ANT_BAR: ${RSSI_ANT_BAR}")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFACTORYFS=\"$ENV{FACTORYFS}\"")
ADD_DEFINITIONS("-DSLP_DEBUG")
ADD_DEFINITIONS("-DSLP_PROF")

ADD_DEFINITIONS("-DVENDOR=\"${VENDOR}\"")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DPACKAGE_NAME=\"${PKGNAME}\"")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")

ADD_DEFINITIONS("-DINSTALL_DIR_APPS=\"${INSTALL_DIR_APPS}\"")
ADD_DEFINITIONS("-DBINDIR=\"${BINDIR}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DICON_DIR=\"${ICONDIR}\"")
ADD_DEFINITIONS("-DIMAGEDIR=\"${IMAGEDIR}\"")
ADD_DEFINITIONS("-DSOUND_DIR=\"${SOUNDDIR}\"")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")
ADD_DEFINITIONS("-DPROJECT=\"${PROJECT_NAME}\"")

ADD_EXECUTABLE(${PROJECT_NAME}-app ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME}-app ${pkgs_LDFLAGS} "-pie" ${LIB_M})

IF("${RSSI_ANT_BAR}" STREQUAL "5")
	SET(RSSI_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/groups/5level")
ELSEIF("${RSSI_ANT_BAR}" STREQUAL "6")
	SET(RSSI_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/groups/6level")
ELSE()
	SET(RSSI_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/groups/4level")
ENDIF()



# install files
INSTALL(TARGETS ${PROJECT_NAME}-app DESTINATION ${BINDIR})


# install application HOME directory
INSTALL(DIRECTORY DESTINATION ${RESDIR})

#ADD_SUBDIRECTORY(po)

# install desktop files
set(PREFIX ${CMAKE_INSTALL_PREFIX})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.tizen.apptray-widget.xml DESTINATION /usr/share/packages)


#icon
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/icon/org.tizen.apptray-widget-app.png DESTINATION /usr/share/icons)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/icon/org.tizen.apptray-widget-app-small.png DESTINATION /usr/share/icons)
ADD_SUBDIRECTORY(res/image)
ADD_SUBDIRECTORY(res/edje)
# End of a file
