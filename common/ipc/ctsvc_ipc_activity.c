#include "ctsvc_ipc_macros.h"

#include "ctsvc_internal.h"
#include "ctsvc_ipc_marshal.h"
#include "contacts_record.h"

static int __ctsvc_ipc_unmarshal_activity(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_activity(const contacts_record_h record, pims_ipc_data_h ipc_data);
static int __ctsvc_ipc_marshal_activity_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_activity_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_activity,
	.marshal_record = __ctsvc_ipc_marshal_activity,
	.get_primary_id = __ctsvc_ipc_marshal_activity_get_primary_id
};


static int __ctsvc_ipc_unmarshal_activity(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record)
{
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(record==NULL,CONTACTS_ERROR_NO_DATA);

	ctsvc_activity_s* activity_p = (ctsvc_activity_s*) record;

	do
	{
		if (ctsvc_ipc_unmarshal_int(ipc_data, &activity_p->id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &activity_p->contact_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->source_name) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->status) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &activity_p->timestamp) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->service_operation) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->uri) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->sync_data1) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->sync_data2) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->sync_data3) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->sync_data4) != CONTACTS_ERROR_NONE) break;

		if (ctsvc_ipc_unmarshal_list(ipc_data, (contacts_list_h*)&activity_p->photos) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;

	} while(0);

	CTS_ERR("_ctsvc_ipc_unmarshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_activity(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_activity_s* activity_p = (ctsvc_activity_s*)record;
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(activity_p==NULL,CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_int((activity_p->id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((activity_p->contact_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((activity_p->source_name),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((activity_p->status),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((activity_p->timestamp),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((activity_p->service_operation),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((activity_p->uri),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((activity_p->sync_data1),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((activity_p->sync_data2),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((activity_p->sync_data3),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((activity_p->sync_data4),ipc_data) != CONTACTS_ERROR_NONE) break;

		if (ctsvc_ipc_marshal_list( (contacts_list_h)activity_p->photos, ipc_data) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while(0);

	CTS_ERR("_ctsvc_ipc_marshal fail");

	return CONTACTS_ERROR_INVALID_PARAMETER;

}

static int __ctsvc_ipc_marshal_activity_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id)
{
	*property_id = CTSVC_PROPERTY_ACTIVITY_ID;
	return contacts_record_get_int(record, *property_id, id );
}