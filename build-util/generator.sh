#
# Contacts Service
#
# Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

echo "###### DB Generator #####"

cd build-util
make

# Make DB schema for generating DB file when running contacts server daemon
echo "static const char *schema_query = \"\\" > ../server/schema.h
./DB-schema-gen ../schema.sql >> ../server/schema.h
echo \"\; >> ../server/schema.h

make clean
