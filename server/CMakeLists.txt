INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/db)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/ipc)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR})
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/server)

SET(DAEMON contacts-service-ipcd)

SET(SRCS
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
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_url.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_extension.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_profile.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_image.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_sdn.c
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_speeddial.c

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
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_speeddial.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_sdn.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_handle.c

	db/ctsvc_db_access_control.c
	db/ctsvc_db_init.c
	db/ctsvc_db_plugin_activity.c
	db/ctsvc_db_plugin_activity_photo_helper.c
	db/ctsvc_db_plugin_activity_photo.c
	db/ctsvc_db_plugin_address_helper.c
	db/ctsvc_db_plugin_address.c
	db/ctsvc_db_plugin_addressbook_helper.c
	db/ctsvc_db_plugin_addressbook.c
	db/ctsvc_db_plugin_company_helper.c
	db/ctsvc_db_plugin_company.c
	db/ctsvc_db_plugin_contact_helper.c
	db/ctsvc_db_plugin_contact.c
	db/ctsvc_db_plugin_my_profile.c
	db/ctsvc_db_plugin_email_helper.c
	db/ctsvc_db_plugin_email.c
	db/ctsvc_db_plugin_event_helper.c
	db/ctsvc_db_plugin_event.c
	db/ctsvc_db_plugin_extension_helper.c
	db/ctsvc_db_plugin_extension.c
	db/ctsvc_db_plugin_group_helper.c
	db/ctsvc_db_plugin_group.c
	db/ctsvc_db_plugin_grouprelation.c
	db/ctsvc_db_plugin_image_helper.c
	db/ctsvc_db_plugin_image.c
	db/ctsvc_db_plugin_messenger_helper.c
	db/ctsvc_db_plugin_messenger.c
	db/ctsvc_db_plugin_name_helper.c
	db/ctsvc_db_plugin_name.c
	db/ctsvc_db_plugin_nickname_helper.c
	db/ctsvc_db_plugin_nickname.c
	db/ctsvc_db_plugin_note_helper.c
	db/ctsvc_db_plugin_note.c
	db/ctsvc_db_plugin_number_helper.c
	db/ctsvc_db_plugin_number.c
	db/ctsvc_db_plugin_person_helper.c
	db/ctsvc_db_plugin_person.c
	db/ctsvc_db_plugin_profile_helper.c
	db/ctsvc_db_plugin_profile.c
	db/ctsvc_db_plugin_relationship_helper.c
	db/ctsvc_db_plugin_relationship.c
	db/ctsvc_db_plugin_simple_contact.c
	db/ctsvc_db_plugin_url_helper.c
	db/ctsvc_db_plugin_url.c
	db/ctsvc_db_plugin_sdn.c
	db/ctsvc_db_plugin_speeddial.c
	db/ctsvc_db_query.c
	db/ctsvc_db_sqlite.c
	db/ctsvc_db_utils.c

	ctsvc_localize_jp.c
	ctsvc_localize_kor.c
	ctsvc_localize_ch.c
	ctsvc_localize.c
	ctsvc_normalize.c
	ctsvc_number_utils.c

	ctsvc_notification.c
	ctsvc_server_activity.c
	ctsvc_server_group.c
	ctsvc_server_person.c
	ctsvc_server_service.c
	ctsvc_server_setting.c

	ctsvc_ipc_server.c
	ctsvc_ipc_server2.c
	ctsvc_server_sqlite.c
	ctsvc_server_utils.c
	ctsvc_server_change_subject.c
	ctsvc_server_socket.c
	ctsvc_schema_recovery.c
	ctsvc_server_update.c
	ctsvc_server_bg.c
	ctsvc_server.c
	ctsvc_server_sim.c
)

IF(ENABLE_LOG_FEATURE)
SET(SRCS ${SRCS}
	${CMAKE_SOURCE_DIR}/common/ipc/ctsvc_ipc_phonelog.c
	${CMAKE_SOURCE_DIR}/common/ctsvc_record_phonelog.c

	db/ctsvc_db_plugin_phonelog.c
	ctsvc_server_phonelog.c
)
ENDIF(ENABLE_LOG_FEATURE)


INCLUDE(FindPkgConfig)
pkg_check_modules(ctsvc_server_pkgs REQUIRED
	sqlite3
	tapi
	capi-media-image-util
	accounts-svc
	libexif
	libsmack
	cynara-client
	cynara-session
	cynara-creds-socket
	capi-system-info
	phonenumber-utils
)

FOREACH(flag ${ctsvc_server_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -Wall -g -fPIC -std=c++0x")
SET(ctsvc_server_pkgs_LDFLAGS "${pkgs_LDFLAGS} ${ctsvc_server_pkgs_LDFLAGS}")

ADD_DEFINITIONS("-D_CONTACTS_IPC_SERVER")
ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")

#cmake_policy(SET CMP0002 OLD)
ADD_EXECUTABLE(${DAEMON} ${SRCS})
SET_TARGET_PROPERTIES(${DAEMON} PROPERTIES COMPILE_FLAGS ${EXTRA_CFLAGS})
TARGET_LINK_LIBRARIES(${DAEMON} ${ctsvc_server_pkgs_LDFLAGS})

INSTALL(TARGETS ${DAEMON} DESTINATION bin)
