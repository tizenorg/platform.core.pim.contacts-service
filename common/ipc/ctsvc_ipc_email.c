#include "ctsvc_ipc_macros.h"


#include "ctsvc_internal.h"
#include "ctsvc_ipc_marshal.h"
#include "contacts_record.h"

static int __ctsvc_ipc_unmarshal_email(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_email(const contacts_record_h record, pims_ipc_data_h ipc_data);
static int __ctsvc_ipc_marshal_email_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_email_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_email,
	.marshal_record = __ctsvc_ipc_marshal_email,
	.get_primary_id = __ctsvc_ipc_marshal_email_get_primary_id
};


static int __ctsvc_ipc_unmarshal_email(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record)
{
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(record==NULL,CONTACTS_ERROR_NO_DATA);

	ctsvc_email_s* email_p = (ctsvc_email_s*) record;

	do {
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &email_p->is_default) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &email_p->id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &email_p->contact_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &email_p->type) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &email_p->label) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &email_p->email_addr) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;

	} while(0);

	CTS_ERR("_ctsvc_ipc_unmarshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_email(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_email_s* email_p = (ctsvc_email_s*)record;
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(email_p==NULL,CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_bool((email_p->is_default),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((email_p->id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((email_p->contact_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((email_p->type),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((email_p->label),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((email_p->email_addr),ipc_data) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;

	} while(0);

	CTS_ERR("_ctsvc_ipc_marshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_email_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id)
{
	*property_id = CTSVC_PROPERTY_EMAIL_ID;
	return contacts_record_get_int(record, *property_id, id );
}
