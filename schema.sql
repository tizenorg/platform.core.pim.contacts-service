--
-- Contacts Service
--
-- Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
--
-- Contact: Jongwon Lee <gogosing.lee@samsung.com>
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
-- http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
--

--PRAGMA journal_mode = PERSIST;
--PRAGMA journal_mode = TRUNCATE;

CREATE TABLE persons
(
	person_id			INTEGER PRIMARY KEY AUTOINCREMENT,
	name_contact_id		INTEGER NOT NULL,
	has_phonenumber		INTEGER,
	has_email			INTEGER,
	created_ver			INTEGER NOT NULL,
	changed_ver			INTEGER NOT NULL,
	ringtone_path			TEXT,
	vibration			TEXT,
	image_thumbnail_path		TEXT,
	image_path			TEXT,
	link_count			INTEGER,
	account_id1			INTEGER,
	account_id2			INTEGER,
	account_id3			INTEGER,
	addressbook_ids			TEXT,
	dirty				INTEGER,
	status				TEXT
);

CREATE TRIGGER trg_person_del AFTER DELETE ON persons
 BEGIN
	DELETE FROM favorites WHERE person_id = old.person_id;
	SELECT _PERSON_DELETE_(old.person_id);
 END;

CREATE TABLE addressbooks
(
	addressbook_id		INTEGER PRIMARY KEY AUTOINCREMENT,
	addressbook_name	TEXT NOT NULL,
	account_id			INTEGER,
	mode						INTEGER, -- permission
	last_sync_ver	INTEGER,
	UNIQUE(addressbook_name)
);

insert into addressbooks(addressbook_id, addressbook_name, mode, account_id) values(0, 'http://tizen.org/addressbook/phone', 0, 0);

CREATE TRIGGER trg_addressbook_del AFTER DELETE ON addressbooks
 BEGIN
   DELETE FROM groups WHERE addressbook_id = old.addressbook_id;
   UPDATE contacts SET deleted = 1, person_id = 0, changed_ver = ((SELECT ver FROM cts_version) + 1) WHERE addressbook_id = old.addressbook_id;
   DELETE FROM my_profiles WHERE addressbook_id = old.addressbook_id;
   DELETE FROM contact_deleteds WHERE addressbook_id = old.addressbook_id;
   DELETE FROM group_deleteds WHERE addressbook_id = old.addressbook_id;
 END;

CREATE TABLE contacts
(
	contact_id			INTEGER PRIMARY KEY AUTOINCREMENT,
	person_id			INTEGER NOT NULL,
	addressbook_id			INTEGER NOT NULL DEFAULT 0,
	has_phonenumber		INTEGER,
	has_email			INTEGER,
	is_favorite			INTEGER DEFAULT 0,
	deleted				INTEGER DEFAULT 0,
	display_name			TEXT,
	reverse_display_name		TEXT,
	display_name_source		INTEGER,
	display_name_language		INTEGER,
	sort_name			TEXT,
	reverse_sort_name		TEXT,
	sortkey				TEXT COLLATE NOCASE,
	reverse_sortkey			TEXT COLLATE NOCASE,
	created_ver			INTEGER NOT NULL,
	changed_ver			INTEGER NOT NULL,
	changed_time			INTEGER NOT NULL,
	image_changed_ver	INTEGER NOT NULL,
	uid				TEXT,
	ringtone_path			TEXT,
	vibration			TEXT,
	image_thumbnail_path		TEXT,
	image_path			TEXT
);

CREATE INDEX contacts_idx1 ON contacts(changed_ver);
CREATE INDEX contacts_idx2 ON contacts(person_id);
CREATE INDEX contacts_idx3 ON contacts(display_name_language, sortkey);
CREATE INDEX contacts_idx4 ON contacts(display_name_language, reverse_sortkey);

-- There are three case of deleting contact logically
--   Case 1 : delete contact
--   Case 2 : delete addressbook
--   Case 3 : delete person
-- In all Case, the deleted contacts(deleted=1) are really deleted in the background.
CREATE TRIGGER trg_contacts_del AFTER DELETE ON contacts
	BEGIN
		DELETE FROM data WHERE contact_id = old.contact_id AND is_my_profile = 0;
		DELETE FROM group_relations WHERE old.addressbook_id != -1 AND contact_id = old.contact_id;
		DELETE FROM activities WHERE contact_id = old.contact_id;
		DELETE FROM persons WHERE person_id = old.person_id AND link_count = 1;
		DELETE FROM search_index WHERE contact_id = old.contact_id;
		DELETE FROM name_lookup WHERE contact_id = old.contact_id;
		DELETE FROM phone_lookup WHERE contact_id = old.contact_id;
		UPDATE persons SET dirty=1 WHERE person_id = old.person_id AND link_count > 1;
	END;

-- It is triggered during really deleting contact in the background (deleted = 1).
-- Deleted version(changed_ver) is already set when updating deleted field as 1.
CREATE TRIGGER trg_contacts_del2 AFTER DELETE ON contacts
	WHEN old.addressbook_id = (SELECT addressbook_id from addressbooks WHERE addressbook_id = old.addressbook_id) AND old.deleted = 1
	BEGIN
		INSERT INTO contact_deleteds VALUES(old.contact_id, old.addressbook_id, old.created_ver, old.changed_ver);
	END;

-- CREATE TRIGGER trg_contacts_del3 AFTER DELETE ON contacts
--	WHEN old.addressbook_id = (SELECT addressbook_id from addressbooks WHERE addressbook_id = old.addressbook_id) AND old.deleted = 0
--	BEGIN
--		INSERT INTO contact_deleteds VALUES(old.contact_id, old.addressbook_id, old.created_ver, (SELECT ver FROM cts_version) + 1);
--	END;

CREATE TRIGGER trg_contacts_update AFTER UPDATE ON contacts
	WHEN new.deleted = 1
	BEGIN
		DELETE FROM group_relations WHERE old.addressbook_id != -1 AND contact_id = old.contact_id;
		DELETE FROM persons WHERE person_id = old.person_id AND link_count = 1;
		UPDATE persons SET dirty=1 WHERE person_id = old.person_id AND link_count > 1;
	END;

CREATE TABLE contact_deleteds
(
	contact_id			INTEGER PRIMARY KEY,
	addressbook_id		INTEGER NOT NULL,
	created_ver			INTEGER NOT NULL,
	deleted_ver			INTEGER NOT NULL
);
CREATE INDEX contact_deleteds_idx1 ON contact_deleteds(deleted_ver);

CREATE TABLE cts_version
(
	ver				INTEGER PRIMARY KEY
);

INSERT INTO cts_version VALUES(0);

CREATE TABLE sdn
(
	id				INTEGER PRIMARY KEY AUTOINCREMENT,
	name				TEXT,
	number				TEXT
);

CREATE TABLE data
(
	id				INTEGER PRIMARY KEY AUTOINCREMENT,
	contact_id			INTEGER NOT NULL,
	datatype			INTEGER NOT NULL,
	is_my_profile		INTEGER,
	is_primary_default		INTEGER,
	is_default			INTEGER,
	data1				INTEGER,
	data2				TEXT,
	data3				TEXT,
	data4				TEXT,
	data5				TEXT,
	data6				TEXT,
	data7				TEXT,
	data8				TEXT,
	data9				TEXT,
	data10				TEXT,
	data11				TEXT,
	data12				TEXT
);

CREATE TRIGGER trg_data_del AFTER DELETE ON data
	BEGIN
		SELECT _DATA_DELETE_(old.id, old.datatype);
	END;

CREATE TRIGGER trg_data_image_del AFTER DELETE ON data
	WHEN old.datatype = 13
		BEGIN
			SELECT _DATA_IMAGE_DELETE_(old.data3);
		END;

CREATE TRIGGER trg_data_company_del AFTER DELETE ON data
	WHEN old.datatype = 6
		BEGIN
			SELECT _DATA_COMPANY_DELETE_(old.data8);
		END;

CREATE TRIGGER trg_data_number_del AFTER DELETE ON data
	WHEN old.datatype = 8
		BEGIN
			DELETE FROM speeddials WHERE  number_id = old.id;
		END;

CREATE INDEX data_contact_idx1 ON data(contact_id);
CREATE INDEX data_contact_idx2 ON data(datatype, contact_id);
CREATE INDEX data_idx1 ON data(data1);
CREATE INDEX data_idx2 ON data(data2);
CREATE INDEX data_idx3 ON data(data3);
CREATE INDEX data_idx4 ON data(data4);
CREATE INDEX data_idx5 ON data(data5);
CREATE INDEX data_idx6 ON data(data6);
CREATE INDEX data_idx7 ON data(data7);
CREATE INDEX data_idx8 ON data(data8);
CREATE INDEX data_idx9 ON data(data9);
CREATE INDEX data_idx10 ON data(data10);

CREATE TABLE groups
(
	group_id			INTEGER PRIMARY KEY AUTOINCREMENT,
	addressbook_id			INTEGER,
	group_name			TEXT,
	system_id			TEXT,
	is_read_only			INTEGER DEFAULT 0,
	created_ver			INTEGER NOT NULL,
	changed_ver			INTEGER NOT NULL,
	ringtone_path			TEXT,
	vibration			TEXT,
	image_thumbnail_path		TEXT,
	member_changed_ver		INTEGER,
	group_prio			REAL
);

INSERT INTO groups(addressbook_id, group_name, system_id, is_read_only, created_ver, changed_ver, group_prio)
	VALUES(0, 'coworkers', 'coworkers', 0, 0, 0, 1);
INSERT INTO groups(addressbook_id, group_name, system_id, is_read_only, created_ver, changed_ver, group_prio)
	VALUES(0, 'family', 'family', 0, 0, 0, 2);
INSERT INTO groups(addressbook_id, group_name, system_id, is_read_only, created_ver, changed_ver, group_prio)
	VALUES(0, 'friends', 'friends',0, 0, 0, 3);

CREATE TRIGGER trg_groups_del AFTER DELETE ON groups
 BEGIN
   DELETE FROM group_relations WHERE group_id = old.group_id;
 END;

CREATE TRIGGER trg_groups_del2 AFTER DELETE ON groups
	WHEN old.addressbook_id IN (SELECT addressbook_id from addressbooks WHERE addressbook_id = old.addressbook_id)
	BEGIN
		INSERT INTO group_deleteds VALUES(old.group_id, old.addressbook_id, old.created_ver, (SELECT ver FROM cts_version) + 1);
	END;

CREATE TABLE group_deleteds
(
	group_id				INTEGER PRIMARY KEY,
	addressbook_id		INTEGER NOT NULL,
	created_ver			INTEGER NOT NULL,
	deleted_ver			INTEGER NOT NULL
);

CREATE INDEX group_deleteds_idx1 ON group_deleteds(deleted_ver);

CREATE TABLE group_relations
(
	group_id			INTEGER NOT NULL,
	contact_id			INTEGER NOT NULL,
	ver INTEGER NOT NULL,
	deleted INTEGER DEFAULT 0,
	UNIQUE(group_id, contact_id)
);
CREATE INDEX groups_idx1 ON group_relations(contact_id);


CREATE TABLE speeddials
(
	speed_number			INTEGER PRIMARY KEY NOT NULL,
	number_id			INTEGER UNIQUE
);

CREATE TABLE favorites
(
	person_id			INTEGER PRIMARY KEY,
	favorite_prio			REAL
);
CREATE INDEX favorites_idx1 ON favorites(favorite_prio);
CREATE INDEX favorites_idx2 ON favorites(person_id);


--CREATE TRIGGER trg_favorites_del BEFORE DELETE ON favorites
--	BEGIN
--		UPDATE contacts SET is_favorite = 0 WHERE person_id = old.person_id;
--	END;
--CREATE TRIGGER trg_favorites_insert AFTER INSERT ON favorites
--	BEGIN
--		UPDATE contacts SET is_favorite = 1 WHERE person_id = new.person_id;
--	END;


CREATE TABLE phonelogs
(
	id				INTEGER PRIMARY KEY AUTOINCREMENT,
	number				TEXT,
	normal_num			TEXT,
	person_id			INTEGER, --person_id
	log_type			INTEGER,
	log_time			INTEGER,
	data1				INTEGER, --duration, message_id
	data2				TEXT  -- short message
);

CREATE INDEX phonelogs_idx1 ON phonelogs(log_type);
CREATE INDEX phonelogs_idx2 ON phonelogs(log_time);
CREATE TRIGGER trg_phonelogs_del AFTER DELETE ON phonelogs
	BEGIN
		SELECT _PHONE_LOG_DELETE_(old.id);
	END;

--CREATE TRIGGER trg_phonelogs_del AFTER DELETE ON phonelogs
--	WHEN old.log_type = 2 OR old.log_type = 4
--		BEGIN
--			DELETE FROM phonelog_accumulation WHERE log_time < (old.log_time - 3456000); -- 40 days
--			INSERT INTO phonelog_accumulation VALUES(NULL, 1, old.log_time, old.data1);
--		END;

--CREATE TABLE phonelog_accumulation
--(
--	id				INTEGER PRIMARY KEY AUTOINCREMENT,
--	log_cnt			INTEGER,
--	log_time			INTEGER,
--	duration			INTEGER
--);
--INSERT INTO phonelog_accumulation VALUES(1, 0, NULL, 0);
--INSERT INTO phonelog_accumulation VALUES(2, 0, NULL, 0); --total

CREATE TABLE phonelog_stat
(
	log_type			INTEGER PRIMARY KEY,
	log_count			INTEGER
);

CREATE TRIGGER trg_phonelogs_insert AFTER INSERT ON phonelogs
	BEGIN
		INSERT OR REPLACE INTO phonelog_stat values(new.log_type, coalesce((SELECT log_count+1 FROM phonelog_stat WHERE log_type=new.log_type), 1));
	END;

CREATE TABLE contact_stat
(
	id				INTEGER PRIMARY KEY AUTOINCREMENT,
	person_id			INTEGER,
	usage_type			INTEGER,
	times_used			INTEGER
);

CREATE TABLE activities
(
	id				INTEGER PRIMARY KEY AUTOINCREMENT,
	contact_id			INTEGER NOT NULL,
	source_name			TEXT,
	status				TEXT,
	timestamp			INTEGER,
	sync_data1			TEXT,
	sync_data2			TEXT,
	sync_data3			TEXT,
	sync_data4			TEXT
);

CREATE TABLE activity_photos
(
	id				INTEGER PRIMARY KEY AUTOINCREMENT,
	activity_id			INTEGER NOT NULL,
	photo_url			TEXT,
	sort_index			INTEGER
);

CREATE TRIGGER trg_activities_insert AFTER INSERT ON activities
	BEGIN
		UPDATE persons SET status=(SELECT status FROM activities WHERE contact_id IN (SELECT contact_id FROM contacts WHERE person_id = (select person_id FROM contacts WHERE contact_id = new.contact_id)) ORDER BY timestamp DESC LIMIT 1)  WHERE person_id = (SELECT person_id FROM contacts WHERE contact_id = new.contact_id);
	END;

CREATE TRIGGER trg_activities_delete AFTER DELETE ON activities
	BEGIN
		UPDATE persons SET status=(SELECT status FROM activities WHERE contact_id IN (SELECT contact_id FROM contacts WHERE person_id = (select person_id FROM contacts WHERE contact_id = old.contact_id)) ORDER BY timestamp DESC LIMIT 1)  WHERE person_id = (SELECT person_id FROM contacts WHERE contact_id = old.contact_id);
		DELETE FROM activity_photos WHERE activity_id = old.id;
	END;

CREATE VIRTUAL TABLE search_index USING FTS4
(
	contact_id INTEGER NOT NULL,
	data TEXT,
	name TEXT,
	number TEXT,
	UNIQUE(contact_id)
);

CREATE TABLE name_lookup
(
	data_id	INTEGER NOT NULL,
	contact_id INTEGER NOT NULL,
	name TEXT,
	type INTEGER
);

CREATE TABLE phone_lookup
(
	data_id	INTEGER NOT NULL,
	contact_id INTEGER NOT NULL,
	number TEXT,
	min_match TEXT
);


CREATE TABLE my_profiles
(
	my_profile_id			INTEGER PRIMARY KEY AUTOINCREMENT,
	addressbook_id			INTEGER NOT NULL DEFAULT 0,
	display_name			TEXT,
	reverse_display_name		TEXT,
	created_ver			INTEGER NOT NULL,
	changed_ver			INTEGER NOT NULL,
	changed_time			INTEGER NOT NULL,
	uid				TEXT,
	image_thumbnail_path		TEXT,
	deleted				INTEGER DEFAULT 0,
	UNIQUE(addressbook_id)
);

CREATE TRIGGER trg_my_profiles_del AFTER DELETE ON my_profiles
	BEGIN
		DELETE FROM data WHERE contact_id = old.my_profile_id AND is_my_profile = 1;
	END;

CREATE TRIGGER trg_my_profile_update AFTER UPDATE ON my_profiles
	WHEN new.deleted = 1
	BEGIN
		DELETE FROM data WHERE contact_id = old.my_profile_id AND is_my_profile = 1;
	END;

