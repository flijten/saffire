/*
 Copyright (c) 2012, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <limits.h>
#include <pcre.h>
#include "objects/object.h"
#include "objects/regex.h"
#include "objects/boolean.h"
#include "objects/string.h"
#include "objects/null.h"
#include "objects/numerical.h"
#include "objects/base.h"
#include "objects/numerical.h"
#include "objects/method.h"
#include "general/smm.h"
#include "general/md5.h"
#include "debug.h"
#include "interpreter/errors.h"




/* ======================================================================
 *   Supporting functions
 * ======================================================================
 */

char *wctou8(const wchar_t *wstr, long len) {
    char *buf = (char *)smm_malloc(len+1);
    bzero(buf, len+1);
    long conv_len = wcstombs(buf, wstr, len);
    if (conv_len == -1) return NULL;
    return buf;
}


/* ======================================================================
 *   Object methods
 * ======================================================================
 */


/**
 * Saffire method: constructor
 */
SAFFIRE_METHOD(regex, ctor) {
    RETURN_SELF;
}


/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(regex, dtor) {
    RETURN_NULL;
}


/**
 * Saffire method: Returns string regex
 */
SAFFIRE_METHOD(regex, regex) {
    RETURN_STRING(self->regex_string);
}


/**
 * Saffire method: match a string against (compiled) regex
 */
SAFFIRE_METHOD(regex, match) {
    t_string_object *str;
    int ovector[OVECCOUNT];
    int rc;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &str)) {
        saffire_warning("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    // Convert to utf8 and execute regex
    char *s = wctou8(str->value, wcslen(str->value));
    rc = pcre_exec(self->regex, 0, s, strlen(s), 0, 0, ovector, OVECCOUNT);

    // Check result
    if (rc < 0) {
        switch (rc) {
            case PCRE_ERROR_NOMATCH:
                saffire_warning("String didn't match.\n");
                break;
            default :
                saffire_warning("Error while matching: %d\n", rc);
                break;
        }
    }

    // Display result
    for (int i=0; i<rc; i++) {
        DEBUG_PRINT("%2d: %.*s\n", i, ovector[2*i+1] - ovector[2*i], s + ovector[2*i]);
    }

    // Free utf8 string
    smm_free(s);

    // Return number of matches
    RETURN_NUMERICAL(rc);
}


/**
 *
 */
SAFFIRE_METHOD(regex, conv_boolean) {
    if (wcslen(self->regex_string) == 0) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}


/**
 *
 */
SAFFIRE_METHOD(regex, conv_null) {
    RETURN_NULL;
}


/**
 *
 */
SAFFIRE_METHOD(regex, conv_numerical) {
    RETURN_NUMERICAL(0);
}


/**
 *
 */
SAFFIRE_METHOD(regex, conv_string) {
    RETURN_STRING(self->regex_string);
}

/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */

/**
 * Initializes regex methods and properties, these are used
 */
void object_regex_init(void) {
    Object_Regex_struct.methods = ht_create();
    object_add_internal_method(&Object_Regex_struct, "ctor", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_regex_method_ctor);
    object_add_internal_method(&Object_Regex_struct, "dtor", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_regex_method_dtor);

    object_add_internal_method(&Object_Regex_struct, "boolean", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_regex_method_conv_boolean);
    object_add_internal_method(&Object_Regex_struct, "null", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_regex_method_conv_null);
    object_add_internal_method(&Object_Regex_struct, "numerical", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_regex_method_conv_numerical);
    object_add_internal_method(&Object_Regex_struct, "string", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_regex_method_conv_string);

    object_add_internal_method(&Object_Regex_struct, "match", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_regex_method_match);
    object_add_internal_method(&Object_Regex_struct, "regex", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_regex_method_regex);

    Object_Regex_struct.properties = ht_create();
}

/**
 * Frees memory for a regex object
 */
void object_regex_fini(void) {
    ht_destroy(Object_Regex_struct.methods);
    ht_destroy(Object_Regex_struct.properties);
}


/**
 * Frees memory for a regex object
 */
static void obj_free(t_object *obj) {
    t_regex_object *re_obj = (t_regex_object *)obj;

    if (re_obj->regex != NULL) {
        free(re_obj->regex);
        re_obj->regex = NULL;
    }
}

/**
 * Clones a regex object into a new object
 */
static t_object *obj_clone(t_object *obj) {
    t_regex_object *re_obj = (t_regex_object *)obj;

    // Create new object and copy all info
    t_regex_object *new_obj = smm_malloc(sizeof(t_regex_object));
    memcpy(new_obj, re_obj, sizeof(t_regex_object));

    // Newly separated object, so refcount is 1 again
    new_obj->ref_count = 1;

    // Copy / set internal data
    new_obj->regex_string = wcsdup(re_obj->regex_string);

    return (t_object *)new_obj;
}


static t_object *obj_new(t_object *obj, va_list arg_list) {
    const char *error;
    int erroffset;

    // Create new object and copy all info
    t_regex_object *new_obj = smm_malloc(sizeof(t_regex_object));
    memcpy(new_obj, Object_Regex, sizeof(t_regex_object));


    new_obj->regex_string = va_arg(arg_list, wchar_t *);
    char *buf = wctou8(new_obj->regex_string, wcslen(new_obj->regex_string));

    int pcre_options = va_arg(arg_list, int);

    new_obj->regex = pcre_compile(buf, PCRE_UTF8 | pcre_options, &error, &erroffset, 0);
    if (! new_obj->regex) {
        saffire_warning("pcre_compiled failed (offset: %d), %s\n", erroffset, error);
    }

    smm_free (buf);
    return (t_object *)new_obj;
}

#ifdef __DEBUG
char global_buf[1024];
static char *obj_debug(struct _object *obj) {
    char *buf = wctou8(((t_regex_object *)obj)->regex_string, wcslen(((t_regex_object *)obj)->regex_string));
    memcpy(global_buf, buf, 1024);
    global_buf[1023] = 0;
    smm_free(buf);
    return global_buf;
}
#endif

// Regex object management functions
t_object_funcs regex_funcs = {
        obj_new,              // Allocate a new regex object
        obj_free,             // Free a regex object
        obj_clone,            // Clone a regex object
#ifdef __DEBUG
        obj_debug
#endif
};

// Intial object
t_regex_object Object_Regex_struct = {
    OBJECT_HEAD_INIT2("regex", objectTypeRegex, NULL, NULL, OBJECT_TYPE_CLASS, &regex_funcs),
    NULL,
    L'\0',
};