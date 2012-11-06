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
#include <wchar.h>
#include <wctype.h>
#include "debug.h"
#include "object/object.h"
#include "object/base.h"
#include "object/boolean.h"
#include "object/numerical.h"
#include "object/string.h"
#include "object/method.h"
#include "object/null.h"
#include "general/smm.h"
#include "interpreter/errors.h"

#define NUMERICAL_CACHED_MIN   -5       /* minimum numerical value to cache */
#define NUMERICAL_CACHED_MAX  256       /* maximum numerical value to cache */

// Offset calculations
#define NUMERICAL_CACHE_OFF     abs(NUMERICAL_CACHED_MIN)
// Max storage
#define NUMERICAL_CACHED_CNT    NUMERICAL_CACHED_MAX + NUMERICAL_CACHED_MIN + 1

t_numerical_object *numerical_cache[NUMERICAL_CACHED_CNT];


static wchar_t *itow (unsigned long int val) {
    static wchar_t buf[30];
    wchar_t *wcp = &buf[29];
    *wcp = L'\0';
    while (val != 0) {
        *--wcp = btowc ('0' + val % 10);
        val /= 10;
    }
    if (wcp == &buf[29])
        *--wcp = L'0';
    return wcp;
}


/* ======================================================================
 *   Object methods
 * ======================================================================
 */


/**
 * Saffire method: constructor
 */
SAFFIRE_METHOD(numerical, ctor) {
    RETURN_SELF;
}

/**
 * Saffire method: destructor
 */
SAFFIRE_METHOD(numerical, dtor) {
    RETURN_NULL;
}


/**
 * Saffire method: Returns value
 */
SAFFIRE_METHOD(numerical, abs) {
    t_object *obj = object_new(Object_Numerical, abs(self->value));
    RETURN_OBJECT(obj);
}


/**
 * Saffire method: Returns value
 */
SAFFIRE_METHOD(numerical, neg) {
    t_object *obj = object_new(Object_Numerical, 0 - self->value);
    RETURN_OBJECT(obj);
}


/**
 * Saffire method: output numerical value
 */
SAFFIRE_METHOD(numerical, print) {
    printf("%ld\n", self->value);
    RETURN_SELF;
}


SAFFIRE_METHOD(numerical, conv_boolean) {
    if (self->value == 0) {
        RETURN_FALSE;
    } else {
        RETURN_TRUE;
    }
}

SAFFIRE_METHOD(numerical, conv_null) {
    RETURN_NULL;
}

SAFFIRE_METHOD(numerical, conv_numerical) {
    RETURN_SELF;
}

SAFFIRE_METHOD(numerical, conv_string) {
    wchar_t *tmp = itow(self->value);
    RETURN_STRING(tmp);
}


/* ======================================================================
 *   Standard operators
 * ======================================================================
 */
SAFFIRE_OPERATOR_METHOD(numerical, add) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &other)) {
        saffire_warning("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    if (in_place) {
        DEBUG_PRINT("Add to self\n");
        self->value += other->value;
        RETURN_SELF;
    }

    t_object *obj = object_new(Object_Numerical, self->value + other->value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, sub) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &other)) {
        saffire_warning("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    if (in_place) {
        DEBUG_PRINT("Add to self\n");
        self->value -= other->value;
        RETURN_SELF;
    }

    t_object *obj = object_new(Object_Numerical, self->value - other->value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, mul) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &other)) {
        saffire_error("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    if (in_place) {
        DEBUG_PRINT("Add to self\n");
        self->value *= other->value;
        RETURN_SELF;
    }

    t_object *obj = object_new(Object_Numerical, self->value * other->value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, div) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &other)) {
        saffire_error("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    if (in_place) {
        DEBUG_PRINT("Add to self\n");
        self->value /= other->value;
        RETURN_SELF;
    }

    t_object *obj = object_new(Object_Numerical, self->value / other->value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, mod) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &other)) {
        saffire_warning("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    if (in_place) {
        DEBUG_PRINT("Add to self\n");
        self->value %= other->value;
        RETURN_SELF;
    }

    t_object *obj = object_new(Object_Numerical, self->value % other->value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, and) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &other)) {
        saffire_warning("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    if (in_place) {
        DEBUG_PRINT("Add to self\n");
        self->value &= other->value;
        RETURN_SELF;
    }

    t_object *obj = object_new(Object_Numerical, self->value & other->value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, or) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &other)) {
        saffire_warning("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    if (in_place) {
        DEBUG_PRINT("Add to self\n");
        self->value |= other->value;
        RETURN_SELF;
    }

    t_object *obj = object_new(Object_Numerical, self->value | other->value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, xor) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &other)) {
        saffire_warning("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    if (in_place) {
        DEBUG_PRINT("Add to self\n");
        self->value ^= other->value;
        RETURN_SELF;
    }

    t_object *obj = object_new(Object_Numerical, self->value ^ other->value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, sl) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &other)) {
        saffire_warning("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    if (in_place) {
        DEBUG_PRINT("Add to self\n");
        self->value <<= other->value;
        RETURN_SELF;
    }

    t_object *obj = object_new(Object_Numerical, self->value << other->value);
    RETURN_OBJECT(obj);
}

SAFFIRE_OPERATOR_METHOD(numerical, sr) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other;

    // Parse the arguments
    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &other)) {
        saffire_warning("Error while parsing argument list\n");
        RETURN_NUMERICAL(0);
    }

    if (in_place) {
        DEBUG_PRINT("Add to self\n");
        self->value >>= other->value;
        RETURN_SELF;
    }

    t_object *obj = object_new(Object_Numerical, self->value >> other->value);
    RETURN_OBJECT(obj);
}


/* ======================================================================
 *   Standard comparisons
 * ======================================================================
 */
SAFFIRE_COMPARISON_METHOD(numerical, eq) {
    DEBUG_PRINT("Numerical EQ called");

    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other = (t_numerical_object *)_other;

    return (self->value == other->value);
}
SAFFIRE_COMPARISON_METHOD(numerical, ne) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other = (t_numerical_object *)_other;

    return (self->value != other->value);
}
SAFFIRE_COMPARISON_METHOD(numerical, lt) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other = (t_numerical_object *)_other;

    return (self->value < other->value);
}
SAFFIRE_COMPARISON_METHOD(numerical, gt) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other = (t_numerical_object *)_other;

    return (self->value > other->value);
}
SAFFIRE_COMPARISON_METHOD(numerical, le) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other = (t_numerical_object *)_other;

    return (self->value <= other->value);
}
SAFFIRE_COMPARISON_METHOD(numerical, ge) {
    t_numerical_object *self = (t_numerical_object *)_self;
    t_numerical_object *other = (t_numerical_object *)_other;

    return (self->value >= other->value);
}



/* ======================================================================
 *   Global object management functions and data
 * ======================================================================
 */


/**
 * Initializes numerical methods and properties
 */
void object_numerical_init(void) {
    Object_Numerical_struct.methods = ht_create();
    object_add_internal_method(&Object_Numerical_struct, "ctor", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_numerical_method_ctor);
    object_add_internal_method(&Object_Numerical_struct, "dtor", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_numerical_method_dtor);

    object_add_internal_method(&Object_Numerical_struct, "boolean", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_numerical_method_conv_boolean);
    object_add_internal_method(&Object_Numerical_struct, "null", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_numerical_method_conv_null);
    object_add_internal_method(&Object_Numerical_struct, "numerical", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_numerical_method_conv_numerical);
    object_add_internal_method(&Object_Numerical_struct, "string", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_numerical_method_conv_string);

    object_add_internal_method(&Object_Numerical_struct, "neg", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_numerical_method_neg);
    object_add_internal_method(&Object_Numerical_struct, "abs", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_numerical_method_abs);
    object_add_internal_method(&Object_Numerical_struct, "print", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, object_numerical_method_print);

    Object_Numerical_struct.properties = ht_create();


    // Create a numerical cache
    int value = NUMERICAL_CACHED_MIN;
    for (int i=0; i!=NUMERICAL_CACHED_CNT; i++, value++) {
        numerical_cache[i] = smm_malloc(sizeof(t_numerical_object));
        memcpy(numerical_cache[i], Object_Numerical, sizeof(t_numerical_object));
        numerical_cache[i]->value = value;

        numerical_cache[i]->flags |= OBJECT_FLAG_IMMUTABLE | OBJECT_FLAG_STATIC;

        // These are instances
        numerical_cache[i]->flags &= ~OBJECT_TYPE_MASK;
        numerical_cache[i]->flags |= OBJECT_TYPE_INSTANCE;
    }
}


/**
 * Frees memory for a numerical object
 */
void object_numerical_fini(void) {
    ht_destroy(Object_Numerical_struct.methods);
    ht_destroy(Object_Numerical_struct.properties);

    for (int i=0; i!=NUMERICAL_CACHED_CNT; i++) {
        smm_free(numerical_cache[i]);
    }
}


/**
 * Clones a numerical object into a new object
 */
static t_object *obj_clone(t_object *obj) {
    t_numerical_object *num_obj = (t_numerical_object *)obj;

    // Create new object and copy all info
    t_numerical_object *new_obj = smm_malloc(sizeof(t_numerical_object));
    memcpy(new_obj, num_obj, sizeof(t_numerical_object));

    // New separated object, so refcount = 1
    new_obj->ref_count = 1;

    return (t_object *)new_obj;
}


/**
 * Creates a new numerical object by "cloning" the original one
 */
static t_object *obj_new(va_list arg_list) {
    long value = va_arg(arg_list, long);


    // Return cached object if it's already present.
    if (value >= NUMERICAL_CACHED_MIN && value <= NUMERICAL_CACHED_MAX) {
        return (t_object *)numerical_cache[value + NUMERICAL_CACHE_OFF];
    }

    t_numerical_object *new_obj = smm_malloc(sizeof(t_numerical_object));
    memcpy(new_obj, Object_Numerical, sizeof(t_numerical_object));

    new_obj->value = value;

    return (t_object *)new_obj;
}

#ifdef __DEBUG
char tmp[100];
static char *obj_debug(struct _object *obj) {
    sprintf(tmp, "%ld", ((t_numerical_object *)obj)->value);
    return tmp;
}
#endif


// String object management functions
t_object_funcs numerical_funcs = {
        obj_new,            // Allocate a new numerical object
        NULL,               // Free a numerical object
        obj_clone,          // Clone a numerical object
#ifdef __DEBUG
        obj_debug
#endif
};

t_object_operators numerical_ops = {
    object_numerical_operator_add,
    object_numerical_operator_sub,
    object_numerical_operator_mul,
    object_numerical_operator_div,
    object_numerical_operator_mod,
    object_numerical_operator_and,
    object_numerical_operator_or,
    object_numerical_operator_xor,
    object_numerical_operator_sl,
    object_numerical_operator_sr
};

t_object_comparisons numerical_cmps = {
    object_numerical_comparison_eq,
    object_numerical_comparison_ne,
    object_numerical_comparison_lt,
    object_numerical_comparison_gt,
    object_numerical_comparison_le,
    object_numerical_comparison_ge,
    NULL,
    NULL
};

// Intial object
t_numerical_object Object_Numerical_struct = {
    OBJECT_HEAD_INIT2("numerical", objectTypeNumerical, &numerical_ops, &numerical_cmps, OBJECT_TYPE_CLASS, &numerical_funcs),
    0
};