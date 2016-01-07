INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/client)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/ipc)

SET(CLIENT contacts-service2)

SET(SRCS
	ctsvc_client_activity.c
	ctsvc_client_db.c
	ctsvc_client_db_notification.c
	ctsvc_client_group.c
	ctsvc_client_person.c
	ctsvc_client_service.c
	ctsvc_client_ipc.c
	ctsvc_client_noti.c
	ctsvc_client_setting.c
	ctsvc_client_handle.c
	ctsvc_client_utils.c

	ctsvc_client_service_helper.c
	ctsvc_client_db_helper.c
	ctsvc_client_activity_helper.c
	ctsvc_client_person_helper.c
	ctsvc_client_group_helper.c
	ctsvc_client_phonelog_helper.c

	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_marshal.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_addressbook.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_contact.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_my_profile.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_group.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_person.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_result.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_simple_contact.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_result.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_updated_info.c

	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_activity.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_activity_photo.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_address.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_company.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_email.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_event.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_grouprelation.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_messenger.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_name.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_nickname.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_note.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_number.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_relationship.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_image.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_url.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_extension.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_profile.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_sip.c

	${CMAKE_SOURCE_DIR}/common/ctsvc_filter.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_inotify.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_list.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_localize_utils.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_mutex.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_query.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_addressbook.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_contact.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_my_profile.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_group.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_person.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_result.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_updated_info.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_socket.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_vcard.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_view.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_handle.c

	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_sdn.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_sdn.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_sim.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_speeddial.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_speeddial.c
)

IF(ENABLE_LOG_FEATURE)
SET(SRCS ${SRCS}
	ctsvc_client_phonelog.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_phonelog.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_phonelog.c
)
ENDIF(ENABLE_LOG_FEATURE)

pkg_check_modules(client_pkgs REQUIRED glib-2.0 pims-ipc capi-base-common dlog libtzplatform-config)
INCLUDE_DIRECTORIES(${client_pkgs_INCLUDE_DIRS})
LINK_DIRECTORIES(${client_pkgs_LIBRARY_DIRS})

ADD_DEFINITIONS("-D_CONTACTS_IPC_CLIENT")

ADD_LIBRARY(${CLIENT} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${CLIENT} PROPERTIES VERSION ${FULLVER} SOVERSION ${MAJORVER})
TARGET_LINK_LIBRARIES(${CLIENT} ${client_pkgs_LIBRARIES} pthread)
INSTALL(TARGETS ${CLIENT} DESTINATION ${LIB_INSTALL_DIR})

# Install header file
SET(DEST_INCLUDE_DIR "${INCLUDE_INSTALL_DIR}/contacts-svc")
FILE(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*.h)
INSTALL(FILES ${HEADER_FILES} DESTINATION ${DEST_INCLUDE_DIR})

CONFIGURE_FILE(${CLIENT}.pc.in ${CLIENT}.pc @ONLY)
INSTALL(FILES ${CLIENT}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
