#ifndef __CTSVC_IPC_MACROS_H__
#define __CTSVC_IPC_MACROS_H__

#include <pims-ipc.h>
#include <pims-ipc-data.h>
#include <stdbool.h>

#if 0
#define CTSVC_IPC_PREPARE() \
	char __handle_str[1024] = {0, }; \
	snprintf(__handle_str, sizeof(__handle_str), \
			"/opt/CONTACT_SVC_IPC_HANDLE/handle_%d", \
			getpid());

#define CTSVC_IPC_SETUP(type) \
	pims_ipc_h __IPC_HANDLE__ = pims_ipc_create(__handle_str); \
	pims_ipc_data_h __IPC_DATA_IN_HANDLE__ = pims_ipc_data_create(type); \
	pims_ipc_data_h __IPC_DATA_OUT_HANDLE__ = NULL;

#define CTSVC_IPC_CALL(MODULE, FUNCTION) \
	pims_ipc_call(__IPC_HANDLE__, MODULE, FUNCTION, __IPC_DATA_IN_HANDLE__, &(__IPC_DATA_OUT_HANDLE__) );

#define CTSVC_IPC_ASYNC_CALL(MODULE, FUNCTION, CALLBACK, USERPARAM) \
	pims_ipc_call(__IPC_HANDLE__, MODULE, FUNCTION, __IPC_DATA_IN_HANDLE__, CALLBACK, USERPARAM );

#define CTSVC_IPC_DATA_IN_WITH_TYPE(type, in_param) \
	pims_ipc_data_put_with_type_##type(__IPC_DATA_IN_HANDLE__, in_param);

#define CTSVC_IPC_DATA_OUT_WITH_TYPE(type, out_param) \
	pims_ipc_data_get_with_type_##type(__IPC_DATA_OUT_HANDLE__, &out_param);

#define CTSVC_IPC_DATA_OUT_DUP_WITH_TYPE(type, out_param) \
	pims_ipc_data_get_with_type_##type_dup(__IPC_DATA_OUT_HANDLE__, &out_param);

#define CTSVC_IPC_CALL_INSERT(RECORD_TYPE) CTSVC_IPC_CALL("INSERT", RECORD_TYPE)
#define CTSVC_IPC_CALL_UPDATE(RECORD_TYPE) CTSVC_IPC_CALL("UPDATE", RECORD_TYPE)
#define CTSVC_IPC_CALL_DELETE(RECORD_TYPE) CTSVC_IPC_CALL("DELETE", RECORD_TYPE)
#define CTSVC_IPC_CALL_GET(RECORD_TYPE) CTSVC_IPC_CALL("GET_RECORD", RECORD_TYPE)
#define CTSVC_IPC_CALL_GET_ALL(RECORD_TYPE) CTSVC_IPC_CALL("GET_ALL_RECORDS", RECORD_TYPE)
#define CTSVC_IPC_CALL_GET_RECORDS_QUERY(RECORD_TYPE) CTSVC_IPC_CALL("GET_RECORDS_QUERY", RECORD_TYPE)
#define CTSVC_IPC_CALL_INSERT_RECORDS(RECORD_TYPE) CTSVC_IPC_CALL("INSERT_RECORDS", RECORD_TYPE)
#define CTSVC_IPC_CALL_UPDATE_RECORDS(RECORD_TYPE) CTSVC_IPC_CALL("UPDATE_RECORDS", RECORD_TYPE)
#define CTSVC_IPC_CALL_DELETE_RECORDS(RECORD_TYPE) CTSVC_IPC_CALL("DELETE_RECORDS", RECORD_TYPE)

#define CTSVC_IPC_TEARDOWN() \
	pims_ipc_data_destroy(__IPC_DATA_OUT_HANDLE__); \
	pims_ipc_data_destroy(__IPC_DATA_IN_HANDLE__); \
	pims_ipc_destroy(__IPC_HANDLE__);


#define pims_ipc_data_put_with_type_char(data, buf) \
    do { char __tmp = buf; \
        pims_ipc_data_put_with_type(data, PIMS_IPC_DATA_TYPE_CHAR, (void*)&__tmp, 0); } while(0)
#define pims_ipc_data_put_with_type_bool(data, buf) \
    do { bool __tmp = buf; \
        pims_ipc_data_put_with_type(data, PIMS_IPC_DATA_TYPE_INT, (void*)&__tmp, 0); }while(0)
#define pims_ipc_data_put_with_type_int(data, buf) \
    do { int __tmp = buf; \
        pims_ipc_data_put_with_type(data, PIMS_IPC_DATA_TYPE_INT, (void*)&__tmp, 0); } while(0)
#define pims_ipc_data_put_with_type_uint(data, buf) \
    do { unsigned int __tmp = buf; \
        pims_ipc_data_put_with_type(data, PIMS_IPC_DATA_TYPE_UINT, (void*)&__tmp, 0); } while(0)
#define pims_ipc_data_put_with_type_long(data, buf) \
    do { long __tmp = buf; \
        pims_ipc_data_put_with_type(data, PIMS_IPC_DATA_TYPE_LONG, (void*)&__tmp, 0); } while(0)
#define pims_ipc_data_put_with_type_ulong(data, buf) \
    do { unsigned long __tmp = buf; \
        pims_ipc_data_put_with_type(data, PIMS_IPC_DATA_TYPE_ULONG, (void*)&__tmp, 0); } while(0)
#define pims_ipc_data_put_with_type_double(data, buf) \
    do { double __tmp = buf; \
        pims_ipc_data_put_with_type(data, PIMS_IPC_DATA_TYPE_DOUBLE, (void*)&__tmp, 0); } while(0)
#define pims_ipc_data_put_with_type_string(data, buf) \
        pims_ipc_data_put_with_type(data, PIMS_IPC_DATA_TYPE_STRING, (void*)buf, 0)
#define pims_ipc_data_put_with_type_memory(data, buf, size) \
        pims_ipc_data_put_with_type(data, PIMS_IPC_DATA_TYPE_MEMORY, (void*)buf, size)


#define pims_ipc_data_put_bool(data, buf) \
    do { bool __tmp = buf; \
        pims_ipc_data_put(data, (void*)&__tmp, sizeof(int)); }while(0)
#define pims_ipc_data_put_int(data, buf) \
    do { int __tmp = buf; \
        pims_ipc_data_put(data, (void*)&__tmp, sizeof(int)); } while(0)
#define pims_ipc_data_put_string(data, buf) \
	do {  \
		if( buf ) \
			pims_ipc_data_put(data, (void*)buf, strlen(buf)+1); \
		else \
			pims_ipc_data_put(data, (void*)"", 0); } while(0)

static inline void pims_ipc_data_get_with_type_int(pims_ipc_data_h data, int* result)
{
    pims_ipc_data_type_e ipc_dtype = PIMS_IPC_DATA_TYPE_INVALID;
    unsigned int ipc_dsize = 0;

	*result = *((int*)pims_ipc_data_get_with_type(data, &ipc_dtype, &ipc_dsize));
}

static inline void pims_ipc_data_get_dup_with_type_int(pims_ipc_data_h data, int* result)
{
    pims_ipc_data_type_e ipc_dtype = PIMS_IPC_DATA_TYPE_INVALID;
    unsigned int ipc_dsize = 0;

	*result = *((int*) pims_ipc_data_get_dup_with_type(data, &ipc_dtype, &ipc_dsize));
}

static inline void pims_ipc_data_get_with_type_bool(pims_ipc_data_h data, bool* result)
{
    pims_ipc_data_type_e ipc_dtype = PIMS_IPC_DATA_TYPE_INVALID;
    unsigned int ipc_dsize = 0;

	*result = (bool)(*((int*)pims_ipc_data_get_with_type(data, &ipc_dtype, &ipc_dsize)));
}

static inline void pims_ipc_data_get_dup_with_type_bool(pims_ipc_data_h data, bool* result)
{
    pims_ipc_data_type_e ipc_dtype = PIMS_IPC_DATA_TYPE_INVALID;
    unsigned int ipc_dsize = 0;

	*result = (bool)(*((int*) pims_ipc_data_get_dup_with_type(data, &ipc_dtype, &ipc_dsize)));
}
static inline void pims_ipc_data_get_with_type_string(pims_ipc_data_h data, char** result)
{
    pims_ipc_data_type_e ipc_dtype = PIMS_IPC_DATA_TYPE_INVALID;
    unsigned int ipc_dsize = 0;

	*result = (char*)pims_ipc_data_get_with_type(data, &ipc_dtype, &ipc_dsize);
}

static inline void pims_ipc_data_get_dup_with_type_string(pims_ipc_data_h data, char** result)
{
    pims_ipc_data_type_e ipc_dtype = PIMS_IPC_DATA_TYPE_INVALID;
    unsigned int ipc_dsize = 0;

	*result = (char*)pims_ipc_data_get_dup_with_type(data, &ipc_dtype, &ipc_dsize);
}

static inline void pims_ipc_data_get_int(pims_ipc_data_h data, int* result)
{
    unsigned int ipc_dsize = 0;
	*result = *((int*) pims_ipc_data_get(data, &ipc_dsize));
}

static inline void pims_ipc_data_get_dup_int(pims_ipc_data_h data, int* result)
{
    unsigned int ipc_dsize = 0;
	*result = *((int*) pims_ipc_data_get_dup(data, &ipc_dsize));
}

static inline void pims_ipc_data_get_bool(pims_ipc_data_h data, bool* result)
{
    unsigned int ipc_dsize = 0;
	*result = (bool)(*((int*)pims_ipc_data_get(data, &ipc_dsize)));
}

static inline void pims_ipc_data_get_dup_bool(pims_ipc_data_h data, bool* result)
{
    unsigned int ipc_dsize = 0;
	*result = (bool)(*((int*)pims_ipc_data_get_dup(data, &ipc_dsize)));
}
static inline void pims_ipc_data_get_string(pims_ipc_data_h data, char** result)
{
    unsigned int ipc_dsize = 0;
	*result = (char*)pims_ipc_data_get(data, &ipc_dsize);
}

static inline void pims_ipc_data_get_dup_string(pims_ipc_data_h data, char** result)
{
    unsigned int ipc_dsize = 0;
	*result = (char*)pims_ipc_data_get_dup(data, &ipc_dsize);
}

#endif

#endif /*__CTSVC_IPC_MACROS_H__*/

