

#include <glib.h>
//#include <glib-oject.h>
#include "ctsvc_client_ipc.h"

#include "ctsvc_internal.h"
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_query.h"
#include "ctsvc_inotify.h"

#include "ctsvc_internal.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_view.h"
#include <pims-ipc-data.h>
#include "ctsvc_mutex.h"

static __thread pims_ipc_h __contacts_ipc = NULL;
static pims_ipc_h __contacts_global_ipc = NULL;

static __thread int __contacts_change_version = 0;
static int __contacts_global_change_version = 0;

int ctsvc_ipc_connect_on_thread(void)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	// ipc create
	if (__contacts_ipc == NULL)
	{
		__contacts_ipc = pims_ipc_create(CTSVC_IPC_SOCKET_PATH);
		if (__contacts_ipc == NULL)
		{
			CTS_ERR("pims_ipc_create() Failed(%d)", CONTACTS_ERROR_IPC_NOT_AVALIABLE);
			return CONTACTS_ERROR_IPC_NOT_AVALIABLE;
		}
	}
	else
	{
		CTS_DBG("contacts already connected");
		return CONTACTS_ERROR_NONE;
	}

	// ipc call
	if (pims_ipc_call(__contacts_ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CONNECT, indata, &outdata) != 0)
	{
		CTS_ERR("pims_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		pims_ipc_data_destroy(outdata);

		if (ret == CONTACTS_ERROR_NONE)
		{

		}
		else
		{
			pims_ipc_destroy(__contacts_ipc);
			__contacts_ipc = NULL;
		}
	}

	return ret;
}

int ctsvc_ipc_disconnect_on_thread(void)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(__contacts_ipc == NULL, CONTACTS_ERROR_IPC, "contacts not connected");

	// ipc call
	if (pims_ipc_call(__contacts_ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_DISCONNECT, indata, &outdata) != 0)
	{
		CTS_ERR("pims_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		pims_ipc_data_destroy(outdata);
	}

	if (__contacts_ipc && ret != CONTACTS_ERROR_NONE)
	{
		pims_ipc_destroy(__contacts_ipc);
		__contacts_ipc = NULL;
	}

	return ret;
}

pims_ipc_h ctsvc_get_ipc_handle()
{
	if(__contacts_ipc == NULL)
	{
		if(__contacts_global_ipc == NULL )
		{
			ASSERT_NOT_REACHED("IPC haven't been initialized yet.");
			return NULL;
		}
		CTS_DBG("fallback to global ipc channel");
		return __contacts_global_ipc;
	}

	return __contacts_ipc;
}

bool ctsvc_ipc_is_busy()
{
	bool ret = false;

	if(__contacts_ipc != NULL)
	{
		ret = pims_ipc_is_call_in_progress(__contacts_ipc);
		if(ret)
		{
			CTS_ERR("thread local ipc channel is busy.");
		}
	}
	else {
		ret = pims_ipc_is_call_in_progress(__contacts_global_ipc);
		if(ret)
		{
			CTS_ERR("global ipc channel is busy.");
		}
	}

	return ret;
}



int ctsvc_ipc_connect(void)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	// ipc create
	if (__contacts_global_ipc == NULL)
	{
		__contacts_global_ipc = pims_ipc_create(CTSVC_IPC_SOCKET_PATH);
		if (__contacts_global_ipc == NULL)
		{
			CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_create() Failed(%d)", CONTACTS_ERROR_IPC_NOT_AVALIABLE);
			return CONTACTS_ERROR_IPC_NOT_AVALIABLE;
		}
	}
	else
	{
		CTS_DBG("[GLOBAL_IPC_CHANNEL] contacts already connected");
		return CONTACTS_ERROR_NONE;
	}

	// ipc call
	if (pims_ipc_call(__contacts_global_ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CONNECT, indata, &outdata) != 0)
	{
		CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		pims_ipc_data_destroy(outdata);

		if (ret == CONTACTS_ERROR_NONE)
		{

		}
		else
		{
			pims_ipc_destroy(__contacts_global_ipc);
			__contacts_global_ipc = NULL;
		}
	}

	return ret;
}


int ctsvc_ipc_disconnect(void)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(__contacts_global_ipc == NULL, CONTACTS_ERROR_IPC, "[GLOBAL_IPC_CHANNEL] contacts not connected");

	// ipc call
	if (pims_ipc_call(__contacts_global_ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_DISCONNECT, indata, &outdata) != 0)
	{
		CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		pims_ipc_data_destroy(outdata);
	}

	if (__contacts_global_ipc && ret == CONTACTS_ERROR_NONE)
	{
		pims_ipc_destroy(__contacts_global_ipc);
		__contacts_global_ipc = NULL;
	}
	else
	{
		CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc didn't destroyed!!!(%d)", ret);
	}

	return ret;
}

static void __ctsvc_ipc_lock()
{
	if (__contacts_ipc == NULL)
	{
		ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_CALL);
	}
}

static void __ctsvc_ipc_unlock(void)
{
	if (__contacts_ipc == NULL)
	{
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_CALL);
	}
}

int ctsvc_ipc_call(char *module, char *function, pims_ipc_h data_in, pims_ipc_data_h *data_out)
{
	pims_ipc_h ipc_handle = ctsvc_get_ipc_handle();

	__ctsvc_ipc_lock();

	int ret = pims_ipc_call(ipc_handle, module, function, data_in, data_out);

	__ctsvc_ipc_unlock();

	return ret;
}

int ctsvc_ipc_call_async(char *module, char *function, pims_ipc_h data_in, pims_ipc_call_async_cb callback, void *userdata)
{
	pims_ipc_h ipc_handle = ctsvc_get_ipc_handle();

	__ctsvc_ipc_lock();

	int ret = pims_ipc_call_async(ipc_handle, module, function, data_in, callback, userdata);

	__ctsvc_ipc_unlock();

	return ret;
}

void ctsvc_client_ipc_set_change_version(int version)
{
	if (__contacts_ipc == NULL) {
		__contacts_global_change_version = version;
		CTS_DBG("change_version = %d", version);
		return;
	}
	__contacts_change_version = version;
	CTS_DBG("change_version = %d", version);
}

int ctsvc_client_ipc_get_change_version(void)
{
	if (__contacts_ipc == NULL)
		return __contacts_global_change_version;

	return __contacts_change_version;
}

