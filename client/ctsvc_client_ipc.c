

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

static __thread pims_ipc_h contacts_ipc = NULL;

static pims_ipc_h contacts_global_ipc = NULL;

int ctsvc_ipc_connect_on_thread(void)
{
    int ret = CONTACTS_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

    // ipc create
    if (contacts_ipc == NULL)
    {
        contacts_ipc = pims_ipc_create(CTSVC_IPC_SOCKET_PATH);
        if (contacts_ipc == NULL)
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
    if (pims_ipc_call(contacts_ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CONNECT, indata, &outdata) != 0)
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
            pims_ipc_destroy(contacts_ipc);
            contacts_ipc = NULL;
        }
    }

	return ret;
}

int ctsvc_ipc_disconnect_on_thread(void)
{
    int ret = CONTACTS_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

    RETVM_IF(contacts_ipc == NULL, CONTACTS_ERROR_IPC, "contacts not connected");

    // ipc call
    if (pims_ipc_call(contacts_ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_DISCONNECT, indata, &outdata) != 0)
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

    if (contacts_ipc && ret != CONTACTS_ERROR_NONE)
    {
        pims_ipc_destroy(contacts_ipc);
        contacts_ipc = NULL;
    }

    return ret;
}

pims_ipc_h ctsvc_get_ipc_handle()
{
	if(contacts_ipc == NULL)
	{
		if(contacts_global_ipc == NULL )
		{
			ASSERT_NOT_REACHED("IPC haven't been initialized yet.");
			return NULL;
		}
		CTS_DBG("fallback to global ipc channel");
		return contacts_global_ipc;
	}

	return contacts_ipc;
}

bool ctsvc_ipc_is_busy()
{
	bool ret = false;

	if(contacts_ipc != NULL)
	{
		ret = pims_ipc_is_call_in_progress(contacts_ipc);
		if(ret)
		{
			CTS_ERR("thread local ipc channel is busy.");
		}
	}
	else {
		ret = pims_ipc_is_call_in_progress(contacts_global_ipc);
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
    if (contacts_global_ipc == NULL)
    {
    	contacts_global_ipc = pims_ipc_create(CTSVC_IPC_SOCKET_PATH);
        if (contacts_global_ipc == NULL)
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
    if (pims_ipc_call(contacts_global_ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CONNECT, indata, &outdata) != 0)
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
            pims_ipc_destroy(contacts_global_ipc);
            contacts_global_ipc = NULL;
        }
    }

	return ret;
}


int ctsvc_ipc_disconnect(void)
{
    int ret = CONTACTS_ERROR_NONE;
    pims_ipc_data_h indata = NULL;
    pims_ipc_data_h outdata = NULL;

    RETVM_IF(contacts_global_ipc == NULL, CONTACTS_ERROR_IPC, "[GLOBAL_IPC_CHANNEL] contacts not connected");

    // ipc call
    if (pims_ipc_call(contacts_global_ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_DISCONNECT, indata, &outdata) != 0)
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

    if (contacts_global_ipc && ret == CONTACTS_ERROR_NONE)
    {
        pims_ipc_destroy(contacts_global_ipc);
        contacts_global_ipc = NULL;
    }
    else
    {
    	CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc didn't destroyed!!!(%d)", ret);
    }

    return ret;
}


void __ctsvc_ipc_lock()
{
    if (contacts_ipc == NULL)
    {
    	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_CALL);
    }
}

void __ctsvc_ipc_unlock(void)
{
    if (contacts_ipc == NULL)
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
