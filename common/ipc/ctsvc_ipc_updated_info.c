#include "ctsvc_ipc_macros.h"

#include "ctsvc_internal.h"
#include "ctsvc_ipc_marshal.h"
#include "contacts_record.h"

static int __ctsvc_ipc_unmarshal_updated_info(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_updated_info(const contacts_record_h record, pims_ipc_data_h ipc_data);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_updated_info_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_updated_info,
	.marshal_record = __ctsvc_ipc_marshal_updated_info,
	.get_primary_id = NULL
};


static int __ctsvc_ipc_unmarshal_updated_info(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record)
{
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(record==NULL,CONTACTS_ERROR_NO_DATA);

	ctsvc_updated_info_s*  updated_info_p = (ctsvc_updated_info_s*) record;

	do {
		if (ctsvc_ipc_unmarshal_int(ipc_data,&updated_info_p->changed_type) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data,&updated_info_p->id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data,&updated_info_p->changed_ver) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data,&updated_info_p->addressbook_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_bool(ipc_data,&updated_info_p->image_changed) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data,&updated_info_p->last_changed_type) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while(0);

	CTS_ERR("_ctsvc_ipc_unmarshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_updated_info(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_updated_info_s* updated_info_p = (ctsvc_updated_info_s*)record;
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(updated_info_p==NULL,CONTACTS_ERROR_NO_DATA);


	do {
		if (ctsvc_ipc_marshal_int((updated_info_p->changed_type),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((updated_info_p->id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((updated_info_p->changed_ver),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((updated_info_p->addressbook_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_bool((updated_info_p->image_changed),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((updated_info_p->last_changed_type),ipc_data) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while(0);

	CTS_ERR("_ctsvc_ipc_marshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}
