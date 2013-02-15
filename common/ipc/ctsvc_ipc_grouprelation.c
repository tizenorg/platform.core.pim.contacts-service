#include "ctsvc_ipc_macros.h"


#include "ctsvc_internal.h"
#include "ctsvc_ipc_marshal.h"
#include "contacts_record.h"

static int __ctsvc_ipc_unmarshal_group_relation(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_group_relation(const contacts_record_h record, pims_ipc_data_h ipc_data);
static int __ctsvc_ipc_marshal_group_relation_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_group_relation_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_group_relation,
	.marshal_record = __ctsvc_ipc_marshal_group_relation,
	.get_primary_id = __ctsvc_ipc_marshal_group_relation_get_primary_id
};


static int __ctsvc_ipc_unmarshal_group_relation(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record)
{
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(record==NULL,CONTACTS_ERROR_NO_DATA);

	ctsvc_group_relation_s* group_relation_p = (ctsvc_group_relation_s*) record;

	do {
		if (ctsvc_ipc_unmarshal_int(ipc_data, &group_relation_p->id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &group_relation_p->contact_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &group_relation_p->group_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &group_relation_p->group_name) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;

	} while(0);

	CTS_ERR("_ctsvc_ipc_unmarshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_group_relation(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_group_relation_s* group_relation_p = (ctsvc_group_relation_s*)record;
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(group_relation_p==NULL,CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_int((group_relation_p->id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((group_relation_p->contact_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((group_relation_p->group_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((group_relation_p->group_name),ipc_data) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;

	} while(0);

	CTS_ERR("_ctsvc_ipc_marshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_group_relation_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id)
{
	*property_id = CTSVC_PROPERTY_GROUP_RELATION_ID;
	return contacts_record_get_int(record, *property_id, id );
}
