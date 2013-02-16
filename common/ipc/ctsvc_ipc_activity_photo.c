#include "ctsvc_ipc_macros.h"

#include "ctsvc_internal.h"
#include "ctsvc_ipc_marshal.h"
#include "contacts_record.h"

static int __ctsvc_ipc_unmarshal_activity_photo(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_activity_photo(const contacts_record_h record, pims_ipc_data_h ipc_data);
static int __ctsvc_ipc_marshal_activity_photo_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_activity_photo_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_activity_photo,
	.marshal_record = __ctsvc_ipc_marshal_activity_photo,
	.get_primary_id = __ctsvc_ipc_marshal_activity_photo_get_primary_id
};


static int __ctsvc_ipc_unmarshal_activity_photo(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record)
{
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(record==NULL,CONTACTS_ERROR_NO_DATA);

	ctsvc_activity_photo_s* photo_p = (ctsvc_activity_photo_s*) record;

	do
	{
		if (ctsvc_ipc_unmarshal_int(ipc_data, &photo_p->id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &photo_p->activity_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &photo_p->photo_url) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &photo_p->sort_index) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;

	} while(0);

	CTS_ERR("_ctsvc_ipc_unmarshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_activity_photo(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_activity_photo_s* photo_p = (ctsvc_activity_photo_s*)record;
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(photo_p==NULL,CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_int((photo_p->id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((photo_p->activity_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((photo_p->photo_url),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((photo_p->sort_index),ipc_data) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while(0);

	CTS_ERR("_ctsvc_ipc_marshal fail");

	return CONTACTS_ERROR_INVALID_PARAMETER;

}

static int __ctsvc_ipc_marshal_activity_photo_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id)
{
	*property_id = CTSVC_PROPERTY_ACTIVITY_PHOTO_ID;
	return contacts_record_get_int(record, *property_id, id );
}
