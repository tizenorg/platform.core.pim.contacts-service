--
-- Contacts Service
--
-- Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
--
-- Contact: Youngjae Shin <yj99.shin@samsung.com>
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

CREATE TABLE addressbooks
(
addrbook_id INTEGER PRIMARY KEY AUTOINCREMENT,
addrbook_name TEXT,
acc_id INTEGER,
acc_type INTEGER DEFAULT 0,
mode INTEGER, -- permission
last_sync_ver INTEGER
);
--CREATE TRIGGER trg_addressbook_sync AFTER UPDATE OF last_sync_ver ON addressbooks
-- BEGIN
--   DELETE FROM deleteds WHERE addrbook_id = new.addrbook_id and deleted_time <= new.last_sync_ver;
-- END;
CREATE TRIGGER trg_addressbook_del AFTER DELETE ON addressbooks
 BEGIN
   DELETE FROM groups WHERE addrbook_id = old.addrbook_id;
   DELETE FROM contacts WHERE addrbook_id = old.addrbook_id;
   DELETE FROM deleteds WHERE addrbook_id = old.addrbook_id;
 END;

CREATE TABLE contacts
(
contact_id INTEGER PRIMARY KEY AUTOINCREMENT,
addrbook_id INTEGER NOT NULL DEFAULT 0,
default_num INTEGER,
default_email INTEGER,
default_addr INTEGER,
is_favorite INTEGER DEFAULT 0,
created_ver INTEGER NOT NULL,
changed_ver INTEGER NOT NULL,
changed_time INTEGER NOT NULL,
outgoing_count INTEGER DEFAULT 0,
uid TEXT,
ringtone TEXT,
note TEXT,
image0 TEXT, -- normal image
image1 TEXT -- full image
);
CREATE INDEX contacts_ver_idx ON contacts(changed_ver);
CREATE TRIGGER trg_contacts_del AFTER DELETE ON contacts
 BEGIN
   DELETE FROM data WHERE contact_id = old.contact_id;
   DELETE FROM group_relations WHERE old.addrbook_id != -1 AND contact_id = old.contact_id;
   DELETE FROM favorites WHERE type = 0 AND related_id = old.contact_id;
 END;

CREATE TABLE deleteds
(
contact_id INTEGER PRIMARY KEY,
addrbook_id INTEGER,
deleted_ver INTEGER
);
CREATE INDEX deleteds_ver_idx ON deleteds(deleted_ver);

CREATE TABLE cts_version
(
ver INTEGER PRIMARY KEY
);
INSERT INTO cts_version VALUES(0);

CREATE TABLE sim_services
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
type INTEGER,
name TEXT,
number TEXT
);

CREATE TABLE custom_types
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
class INTEGER,
name TEXT,
UNIQUE(class, name)
);
CREATE INDEX idx_custom_type ON custom_types(class, name);
CREATE TRIGGER trg_custom_types_del AFTER DELETE ON custom_types
 BEGIN
	 UPDATE data SET data1 = 0 WHERE old.class = 1 AND number_type = old.id AND datatype = 8;
 END;

CREATE TABLE data
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
contact_id INTEGER NOT NULL,
datatype INTEGER NOT NULL,
data1 INTEGER,
data2 TEXT,
data3 TEXT,
data4 TEXT,
data5 TEXT,
data6 TEXT,
data7 TEXT,
data8 TEXT,
data9 TEXT,
data10 TEXT
);
CREATE TRIGGER trg_data_number_del AFTER DELETE ON data
 WHEN old.datatype = 8
 BEGIN
   DELETE FROM favorites WHERE  type = 1 AND related_id = old.id;
   DELETE FROM speeddials WHERE  number_id = old.id;
 END;
CREATE INDEX data_contact_idx ON data(contact_id);
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
group_id INTEGER PRIMARY KEY AUTOINCREMENT,
addrbook_id INTEGER,
group_name TEXT,
ringtone TEXT,
UNIQUE(addrbook_id, group_name)
);
CREATE TRIGGER trg_groups_del AFTER DELETE ON groups
 BEGIN
   DELETE FROM group_relations WHERE group_id = old.group_id;
 END;

CREATE TABLE group_relations
(
group_id INTEGER NOT NULL,
contact_id INTEGER NOT NULL,
UNIQUE(group_id, contact_id)
);
CREATE INDEX group_idx1 ON group_relations(contact_id);

CREATE TABLE speeddials
(
speed_num INTEGER PRIMARY KEY NOT NULL,
number_id INTEGER UNIQUE
);

CREATE TABLE favorites
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
type INTEGER NOT NULL,
related_id INTEGER NOT NULL,
favorite_prio REAL,
UNIQUE(type, related_id)
);
CREATE INDEX idx1_favorites ON favorites(favorite_prio);
CREATE INDEX idx2_favorites ON favorites(type, related_id);
CREATE TRIGGER trg_favorite_del BEFORE DELETE ON favorites
 BEGIN
   UPDATE data SET data3 = 0 WHERE old.type = 1 AND id = old.related_id AND datatype = 8;
   UPDATE contacts SET is_favorite = 0 WHERE old.type = 0 AND contact_id = old.related_id;
 END;
CREATE TRIGGER trg_favorite_insert AFTER INSERT ON favorites
 BEGIN
   UPDATE data SET data3 = 1 WHERE new.type = 1 AND id = new.related_id AND datatype = 8;
   UPDATE contacts SET is_favorite = 1 WHERE new.type = 0 AND contact_id = new.related_id;
 END;

CREATE TABLE phonelogs
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
number TEXT,
normal_num TEXT,
related_id INTEGER, --contact_id
log_type INTEGER,
log_time INTEGER,
data1 INTEGER, --duration, message_id
data2 TEXT  -- short message
);
CREATE INDEX idx1_phonelogs ON phonelogs(log_type);
CREATE INDEX idx2_phonelogs ON phonelogs(log_time);
CREATE TRIGGER trg_phonelogs_del AFTER DELETE ON phonelogs
 WHEN old.log_type = 2 OR old.log_type = 4
 BEGIN
   DELETE FROM phonelog_accumulation WHERE log_time < (old.log_time - 3456000); -- 40 days
   INSERT INTO phonelog_accumulation VALUES(NULL, 1, old.log_time, old.data1);
 END;

CREATE TABLE phonelog_accumulation
(
id INTEGER PRIMARY KEY AUTOINCREMENT,
log_cnt INTEGER,
log_time INTEGER,
duration INTEGER
);
INSERT INTO phonelog_accumulation VALUES(1, 0, NULL, 0);
INSERT INTO phonelog_accumulation VALUES(2, 0, NULL, 0); --total
