CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(apptray-widget C)

SET(PACKAGE_NAME "org.tizen.${PROJECT_NAME}")
SET(PREFIX "/usr/apps/${PACKAGE_NAME}")
SET(BINDIR "${PREFIX}/bin")
SET(LIBEXECDIR "${PREFIX}/shared/libexec")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "/opt${PREFIX}/data")
SET(LOCALEDIR "${RESDIR}/locale")
SET(IMAGEDIR "${RESDIR}/images")
SET(EDJEDIR "${RESDIR}/edje")
SET(SHARED_RES "${RESDIR}/shared/res")


ADD_SUBDIRECTORY(apptray-widget)
ADD_SUBDIRECTORY(apptray-widget-app)

# End of a file
