/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(argv[1], "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	while ((read = getline(&line, &len, fp)) != -1) {
		if (len >= 6 && '/'==line[0]
				&& '/'==line[1]
				&& '<'==line[2]
				&& '!'==line[3]
				&& '-'==line[4]
				&& '-'==line[5])
			break;
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		if (len >= 5 && '/'==line[0]
				&& '/'==line[1]
				&& '-'==line[2]
				&& '-'==line[3]
				&& '>'==line[4])
			break;
		printf("%s", line);
	}

	free(line);
	exit(EXIT_SUCCESS);
}

