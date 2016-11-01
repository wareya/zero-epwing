/*
 * Copyright (C) 2016  Alex Yatskov <alex@foosoft.net>
 * Author: Alex Yatskov <alex@foosoft.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util.h"

void array_init(Array* arr, size_t init_size) {
    arr->ptr = (Entry *)malloc(init_size * sizeof(Entry));
    arr->used = 0;
    arr->size = init_size;
}

Entry* array_new(Array* arr) {
    if (arr->used == arr->size) {
        arr->size *= 2;
        arr->ptr = (Entry *)realloc(arr->ptr, arr->size * sizeof(Entry));
    }

    return &arr->ptr[arr->used++];
}

void array_free(Array* arr) {
    free(arr->ptr);
    arr->ptr = NULL;
    arr->used = arr->size = 0;
}