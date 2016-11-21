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

#include <string.h>

#include "book.h"
#include "convert.h"
#include "util.h"

#include "eb/eb/eb.h"
#include "eb/eb/error.h"
#include "eb/eb/text.h"

#include "jansson/include/jansson.h"

/*
 * Local functions
 */

static void encode_entry(Book_Entry* entry, json_t* entry_json) {
    json_object_set_new(entry_json, "heading", json_string(entry->heading));
    json_object_set_new(entry_json, "text", json_string(entry->text));
}

static void encode_subbook(Book_Subbook* subbook, json_t* subbook_json) {
    if (subbook->title != NULL) {
        json_object_set_new(subbook_json, "title", json_string(subbook->title));
    }

    if (subbook->copyright != NULL) {
        json_object_set_new(subbook_json, "copyright", json_string(subbook->copyright));
    }

    json_t* entry_json_array = json_array();
    for (int i = 0; i < subbook->entry_count; ++i) {
        json_t* entry_json = json_object();
        encode_entry(subbook->entries + i, entry_json);
        json_array_append(entry_json_array, entry_json);
        json_decref(entry_json);
    }

    json_object_set(subbook_json, "entries", entry_json_array);
    json_decref(entry_json_array);
}

static void encode_book(Book* book, json_t* book_json) {
    json_object_set_new(book_json, "characterCode", json_string(book->character_code));
    json_object_set_new(book_json, "discCode", json_string(book->disc_code));

    json_t* subbook_json_array = json_array();
    for (int i = 0; i < book->subbook_count; ++i) {
        json_t* subbook_json = json_object();
        encode_subbook(book->subbooks + i, subbook_json);
        json_array_append(subbook_json_array, subbook_json);
        json_decref(subbook_json);
    }

    json_object_set(book_json, "subbooks", subbook_json_array);
    json_decref(subbook_json_array);
}

/*
 * Exported functions
 */

char* book_read(EB_Book* book, EB_Hookset* hookset, const EB_Position* position, Book_Mode mode, Gaiji_Context* context) {
    if (eb_seek_text(book, position) != EB_SUCCESS) {
        return NULL;
    }

    char data[1024];
    ssize_t data_length = 0;
    EB_Error_Code error;

    switch (mode) {
        case BOOK_MODE_TEXT:
            error = eb_read_text(
                book,
                NULL,
                hookset,
                context,
                ARRSIZE(data) - 1,
                data,
                &data_length
            );
            break;
        case BOOK_MODE_HEADING:
            error = eb_read_heading(
                book,
                NULL,
                hookset,
                context,
                ARRSIZE(data) - 1,
                data,
                &data_length
            );
            break;
        default:
            return NULL;
    }

    if (error != EB_SUCCESS) {
        return NULL;
    }

    char * result = eucjp_to_utf8(data);
    if (result == NULL) {
        return NULL;
    }

    gaiji_stub_decode(result, strlen(result) + 1, result);
    return result;
}

void book_free(Book* book) {
    for (int i = 0; i < book->subbook_count; ++i) {
        Book_Subbook* subbook = book->subbooks + i;
        free(subbook->title);
        free(subbook->copyright);

        for (int j = 0; j < subbook->entry_count; ++j) {
            Book_Entry* entry = subbook->entries + j;
            free(entry->heading);
            free(entry->text);
        }

        free(subbook->entries);
    }
}

void book_dump(Book* book, bool pretty_print, FILE* fp) {
    json_t* book_json = json_object();
    encode_book(book, book_json);

    char* output = json_dumps(book_json, pretty_print ? JSON_INDENT(4) : JSON_COMPACT);
    if (output != NULL) {
        fputs(output, fp);
    }
    free(output);

    json_decref(book_json);
}