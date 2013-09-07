CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(contacts-service C)

#IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
#	SET(CMAKE_BUILD_TYPE "Release")
#ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
#MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(DEST_INCLUDE_DIR "include/contacts-svc")
SET(SRC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(INCLUDEDIR "\${prefix}/${DEST_INCLUDE_DIR}")
SET(VERSION_MAJOR 0)
SET(VERSION "${VERSION_MAJOR}.9.84.2")

EXECUTE_PROCESS(COMMAND build-util/generator.sh)

SET(PC_REQUIRED "capi-base-common")

#INCLUDE_DIRECTORIES(${SRC_INCLUDE_DIR})
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -I${SRC_INCLUDE_DIR}")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED glib-2.0 capi-base-common vconf dlog db-util icu-i18n security-server)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC")
#SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
#SET(CMAKE_C_FLAGS_RELEASE "-mabi=aapcs-linux -march=armv7-a -msoft-float -O2")

ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")

# Install notification files
FILE(GLOB NOTI_FILES ${CMAKE_SOURCE_DIR}/res/.CONTACTS_SVC_*)
INSTALL(FILES ${NOTI_FILES} DESTINATION /opt/usr/data/contacts-svc
		PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ)

# Install directory for image
INSTALL(DIRECTORY DESTINATION /opt/usr/data/contacts-svc/img/vcard)
INSTALL(DIRECTORY DESTINATION /opt/usr/data/contacts-svc/img/my)
INSTALL(DIRECTORY DESTINATION /opt/usr/data/contacts-svc/img/group)
INSTALL(DIRECTORY DESTINATION /opt/usr/data/contacts-svc/img/logo)

# Install header file
FILE(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

INSTALL(FILES ${HEADER_FILES} DESTINATION ${DEST_INCLUDE_DIR})

# Install DB file
FILE(GLOB DB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/build-util/.contacts-svc.db*)
INSTALL(FILES ${DB_FILES} DESTINATION /opt/usr/dbspace)

#Install SMACK rule file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/contacts-service2.rule DESTINATION /opt/etc/smack/accesses.d)

ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)

