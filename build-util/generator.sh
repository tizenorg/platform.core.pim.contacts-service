#
# Contacts Service
#
# Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
#
# Contact: Youngjae Shin <yj99.shin@samsung.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#!/bin/sh

echo "###### API Generator #####"

cd build-util
make

#contacts-svc.h
cat ../include/contacts-svc.head > ../include/contacts-svc.h
./API-generator ../src/cts-service.h >> ../include/contacts-svc.h
./API-generator ../src/cts-errors.h >> ../include/contacts-svc.h
./API-generator ../src/cts-addressbook.h >> ../include/contacts-svc.h
./API-generator ../src/cts-contact.h >> ../include/contacts-svc.h
./API-generator ../src/cts-person.h >> ../include/contacts-svc.h
./API-generator ../src/cts-normalize.h >> ../include/contacts-svc.h
./API-generator ../src/cts-list.h >> ../include/contacts-svc.h
./API-generator ../src/cts-list-filter.h >> ../include/contacts-svc.h
./API-generator ../src/cts-utils.h >> ../include/contacts-svc.h
./API-generator ../src/cts-vcard.h >> ../include/contacts-svc.h
cat ../include/contacts-svc.tail >> ../include/contacts-svc.h

# contacts-svc-struct.h
cat ../include/contacts-svc-struct.head > ../include/contacts-svc-struct.h
./API-generator ../src/cts-struct.h >> ../include/contacts-svc-struct.h
./API-generator ../src/cts-struct-ext.h >> ../include/contacts-svc-struct.h
cat ../include/contacts-svc-struct.tail >> ../include/contacts-svc-struct.h

# contacts-svc-sub.h
cat ../include/contacts-svc-sub.head > ../include/contacts-svc-sub.h
./API-generator ../src/cts-phonelog.h >> ../include/contacts-svc-sub.h
./API-generator ../src/cts-favorite.h >> ../include/contacts-svc-sub.h
./API-generator ../src/cts-group.h >> ../include/contacts-svc-sub.h
./API-generator ../src/cts-im.h >> ../include/contacts-svc-sub.h
./API-generator ../src/cts-types.h >> ../include/contacts-svc-sub.h
cat ../include/contacts-svc-sub.tail >> ../include/contacts-svc-sub.h

# Schema
echo "static const char *schema_query = \"\\" > ../helper/schema.h
./DB-schema-gen ../schema.sql >> ../helper/schema.h
echo \"\; >> ../helper/schema.h


make clean
