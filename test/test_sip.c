
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <contacts.h>

#include "test_main.h"
#include "test_debug.h"

static int _create_sip(const char *address, int type, char *label, contacts_record_h *out_record)
{
	int ret = 0;
	contacts_record_h sip = NULL;
	ret = contacts_record_create(_contacts_sip._uri, &sip);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_record_create() Fail(%d)", ret);
	ret = contacts_record_set_str(sip, _contacts_sip.address, address);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_record_set_str() Fail(%d)", ret);
	ret = contacts_record_set_int(sip, _contacts_sip.type, type);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_record_set_int() Fail(%d)", ret);
	if (CONTACTS_SIP_TYPE_CUSTOM == type) {
		ret = contacts_record_set_str(sip, _contacts_sip.label, label);
		if (CONTACTS_ERROR_NONE != ret)
			ERROR("contacts_record_set_str() Fail(%d)", ret);
	}
	*out_record = sip;

	return CONTACTS_ERROR_NONE;
}
static int _check_sip(contacts_record_h sip)
{
	int ret = 0;
	char *address = NULL;
	ret = contacts_record_get_str_p(sip, _contacts_sip.address, &address);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_record_get_str_p() Fail(%d)", ret);
	DEBUG("address[%s]", address);

	int type = 0;
	ret = contacts_record_get_int(sip, _contacts_sip.type, &type);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_record_get_int() Fail(%d)", ret);
	DEBUG("type(%d)", type);

	if (CONTACTS_SIP_TYPE_CUSTOM == type) {
		char *label = NULL;
		ret = contacts_record_get_str_p(sip, _contacts_sip.label, &label);
		if (CONTACTS_ERROR_NONE != ret)
			ERROR("contacts_record_get_str_p() Fail(%d)", ret);
		DEBUG("label[%s]", label);
	}

	return CONTACTS_ERROR_NONE;
}

int test_sip(void)
{
	int ret = 0;

	contacts_connect();

	contacts_record_h record = NULL;
	ret = contacts_record_create(_contacts_contact._uri, &record);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_record_create() Fail(%d)", ret);

	contacts_record_h sip = NULL;
	/* sip 1 */
	ret = _create_sip("address1", CONTACTS_SIP_TYPE_HOME, NULL, &sip);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("_create_sip() Fail(%d)", ret);
	ret = contacts_record_add_child_record(record, _contacts_contact.sip, sip);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_record_add_child_record() Fail(%d)", ret);

	/* sip 2 */
	ret = _create_sip("address2", CONTACTS_SIP_TYPE_CUSTOM, "zaq12wsx", &sip);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("_create_sip() Fail(%d)", ret);
	ret = contacts_record_add_child_record(record, _contacts_contact.sip, sip);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_record_add_child_record() Fail(%d)", ret);

	int id = 0;
	ret = contacts_db_insert_record(record, &id);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_db_insert_record() Fail(%d)", ret);
	DEBUG("inserted (%d)", id);

	contacts_record_destroy(record, true);
	record = NULL;
	sip = NULL;

	DEBUG("------------- check");

	ret = contacts_db_get_record(_contacts_contact._uri, id, &record);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_db_get_record() Fail(%d)", ret);

	/* sip 1 */
	ret = contacts_record_get_child_record_at_p(record, _contacts_contact.sip, 0, &sip);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_record_get_child_record() Fail(%d)", ret);
	_check_sip(sip);

	/* sip 2 */
	ret = contacts_record_get_child_record_at_p(record, _contacts_contact.sip, 1, &sip);
	if (CONTACTS_ERROR_NONE != ret)
		ERROR("contacts_record_get_child_record() Fail(%d)", ret);
	_check_sip(sip);

	contacts_disconnect();
	return CONTACTS_ERROR_NONE;
}

