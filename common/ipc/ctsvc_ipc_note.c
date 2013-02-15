#include "ctsvc_ipc_macros.h"


#include "ctsvc_internal.h"
#include "ctsvc_ipc_marshal.h"
#include "contacts_record.h"

static int __ctsvc_ipc_unmarshal_note(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_note(const contacts_record_h record, pims_ipc_data_h ipc_data);
static int __ctsvc_ipc_marshal_note_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_note_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_note,
	.marshal_record = __ctsvc_ipc_marshal_note,
	.get_primary_id = __ctsvc_ipc_marshal_note_get_primary_id
};


static int __ctsvc_ipc_unmarshal_note(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record)
{
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(record==NULL,CONTACTS_ERROR_NO_DATA);

	ctsvc_note_s*  note_p = (ctsvc_note_s*) record;

	do {
		if (ctsvc_ipc_unmarshal_int(ipc_data, &note_p->id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &note_p->contact_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &note_p->note) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;

	} while(0);

	CTS_ERR("_ctsvc_ipc_unmarshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_note(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_note_s* note_p = (ctsvc_note_s*)record;
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(note_p==NULL,CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_int((note_p->id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((note_p->contact_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((note_p->note),ipc_data) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while(0);

	CTS_ERR("_ctsvc_ipc_marshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_note_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id)
{
	*property_id = CTSVC_PROPERTY_NOTE_ID;
	return contacts_record_get_int(record, *property_id, id );
}
