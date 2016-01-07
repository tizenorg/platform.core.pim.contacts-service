CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(contacts-service C CXX)

INCLUDE(FindPkgConfig)

EXECUTE_PROCESS(COMMAND build-util/generator.sh)

SET(EXTRA_CFLAGS "-fvisibility=hidden -fdata-sections -ffunction-sections -Wl,--gc-sections")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Werror-implicit-function-declaration ")

SET(DAEMON contacts-service-ipcd)
SET(CLIENT contacts-service2)

# for tizen open build test
IF(ENABLE_LOG_FEATURE)
	ADD_DEFINITIONS("-DENABLE_LOG_FEATURE")
ENDIF(ENABLE_LOG_FEATURE)

ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(server)
ADD_SUBDIRECTORY(test)

