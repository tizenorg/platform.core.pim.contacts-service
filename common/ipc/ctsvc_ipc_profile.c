#include "ctsvc_ipc_macros.h"

#include "ctsvc_internal.h"
#include "ctsvc_ipc_marshal.h"
#include "contacts_record.h"

static int __ctsvc_ipc_unmarshal_profile(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_profile(const contacts_record_h record, pims_ipc_data_h ipc_data);
static int __ctsvc_ipc_marshal_profile_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_profile_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_profile,
	.marshal_record = __ctsvc_ipc_marshal_profile,
	.get_primary_id = __ctsvc_ipc_marshal_profile_get_primary_id
};


static int __ctsvc_ipc_unmarshal_profile(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record)
{
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(record==NULL,CONTACTS_ERROR_NO_DATA);

	ctsvc_profile_s* profile_p = (ctsvc_profile_s*) record;

	do {
		if (ctsvc_ipc_unmarshal_int(ipc_data, &profile_p->id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &profile_p->contact_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &profile_p->type) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &profile_p->label) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &profile_p->uid) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &profile_p->text) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &profile_p->order) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &profile_p->appsvc_operation) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &profile_p->data1) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &profile_p->data2) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &profile_p->data3) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &profile_p->data4) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while(0);

	CTS_ERR("_ctsvc_ipc_unmarshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_profile(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_profile_s* profile_p = (ctsvc_profile_s*)record;
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(profile_p==NULL,CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_int((profile_p->id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((profile_p->contact_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((profile_p->type),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((profile_p->label),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((profile_p->uid),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((profile_p->text),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((profile_p->order),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((profile_p->appsvc_operation),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((profile_p->data1),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((profile_p->data2),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((profile_p->data3),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((profile_p->data4),ipc_data) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while(0);


	CTS_ERR("_ctsvc_ipc_marshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_profile_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id)
{
	*property_id = CTSVC_PROPERTY_PROFILE_ID;
	return contacts_record_get_int(record, *property_id, id );
}
