/* cJSON.c - Ultralightweight JSON parser in ANSI C */
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include "cJSON.h"

static void *(*cJSON_malloc)(size_t sz) = malloc;
static void (*cJSON_free)(void *ptr) = free;

static char* cJSON_strdup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char *copy = (char*)cJSON_malloc(len);
    if (copy) memcpy(copy, str, len);
    return copy;
}

static int cJSON_strcasecmp(const char *s1, const char *s2) {
    if (!s1) return (s1 == s2) ? 0 : 1;
    if (!s2) return 1;
    for (; tolower(*(const unsigned char *)s1) == tolower(*(const unsigned char *)s2); ++s1, ++s2) {
        if (*s1 == 0) return 0;
    }
    return tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
}

void cJSON_InitHooks(cJSON_Hooks* hooks)
{
    if (!hooks) {
        cJSON_malloc = malloc;
        cJSON_free = free;
        return;
    }
    cJSON_malloc = (hooks->malloc_fn) ? hooks->malloc_fn : malloc;
    cJSON_free = (hooks->free_fn) ? hooks->free_fn : free;
}

/* Internal constructor. */
static cJSON *cJSON_New_Item(void)
{
    cJSON* node = (cJSON*)cJSON_malloc(sizeof(cJSON));
    if (node) memset(node, 0, sizeof(cJSON));
    return node;
}

/* Delete a cJSON structure. */
void cJSON_Delete(cJSON *c)
{
    cJSON *next;
    while (c)
    {
        next = c->next;
        if (!(c->type & cJSON_IsReference) && c->child) cJSON_Delete(c->child);
        if (!(c->type & cJSON_IsReference) && c->valuestring) cJSON_free(c->valuestring);
        if (!(c->type & cJSON_StringIsConst) && c->string) cJSON_free(c->string);
        cJSON_free(c);
        c = next;
    }
}

/* Parse the input text to generate a number, and populate the result into item. */
static const char *parse_number(cJSON *item, const char *num)
{
    double n = 0, sign = 1, scale = 0;
    int subscale = 0, signsubscale = 1;

    if (*num == '-') sign = -1, num++;
    if (*num == '0') num++;
    if (*num >= '1' && *num <= '9') {
        do n = (n * 10.0) + (*num++ - '0');
        while (*num >= '0' && *num <= '9');
    }
    if (*num == '.' && num[1] >= '0' && num[1] <= '9') {
        num++;
        do n = (n * 10.0) + (*num++ - '0'), scale--;
        while (*num >= '0' && *num <= '9');
    }
    if (*num == 'e' || *num == 'E') {
        num++;
        if (*num == '+') num++;
        else if (*num == '-') signsubscale = -1, num++;
        while (*num >= '0' && *num <= '9') subscale = (subscale * 10) + (*num++ - '0');
    }

    n = sign * n * pow(10.0, (scale + subscale * signsubscale));
    item->valuedouble = n;
    item->valueint = (int)n;
    item->type = cJSON_Number;
    return num;
}

static int power2gt(int x) { --x; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16; return x + 1; }

typedef struct {
    char *buffer;
    int length;
    int offset;
} printbuffer;

static char* ensure(printbuffer *p, int needed)
{
    char *newbuffer;
    int newsize;
    if (!p || !p->buffer) return 0;
    needed += p->offset;
    if (needed <= p->length) return p->buffer + p->offset;

    newsize = power2gt(needed);
    newbuffer = (char*)cJSON_malloc(newsize);
    if (!newbuffer) {
        cJSON_free(p->buffer);
        p->length = 0; p->buffer = 0;
        return 0;
    }
    memcpy(newbuffer, p->buffer, p->length);
    cJSON_free(p->buffer);
    p->length = newsize;
    p->buffer = newbuffer;
    return newbuffer + p->offset;
}

static char *print_number(cJSON *item, printbuffer *p)
{
    char *str = 0;
    double d = item->valuedouble;
    if (d == 0) {
        if (p) str = ensure(p, 2);
        else str = (char*)cJSON_malloc(2);
        if (str) strcpy(str, "0");
    } else if (fabs(((double)item->valueint) - d) <= 1e-9 && d >= INT_MIN && d <= INT_MAX) {
        if (p) str = ensure(p, 21);
        else str = (char*)cJSON_malloc(21);
        if (str) sprintf(str, "%d", item->valueint);
    } else {
        if (p) str = ensure(p, 64);
        else str = (char*)cJSON_malloc(64);
        if (str) {
            if (fabs(floor(d) - d) <= 1e-9 && fabs(d) < 1.0e60) sprintf(str, "%.0f", d);
            else if (fabs(d) < 1.0e-6 || fabs(d) > 1.0e9) sprintf(str, "%e", d);
            else sprintf(str, "%f", d);
        }
    }
    return str;
}

static const char *parse_string(cJSON *item, const char *str)
{
    const char *ptr = str + 1;
    char *ptr2;
    char *out;
    int len = 0;

    if (*str != '\"') return 0;

    while (*ptr != '\"' && *ptr && ++len) {
        if (*ptr++ == '\\') ptr++;
    }

    out = (char*)cJSON_malloc(len + 1);
    if (!out) return 0;

    ptr = str + 1;
    ptr2 = out;
    while (*ptr != '\"' && *ptr) {
        if (*ptr != '\\') *ptr2++ = *ptr++;
        else {
            ptr++;
            switch (*ptr) {
                case 'b': *ptr2++ = '\b'; break;
                case 'f': *ptr2++ = '\f'; break;
                case 'n': *ptr2++ = '\n'; break;
                case 'r': *ptr2++ = '\r'; break;
                case 't': *ptr2++ = '\t'; break;
                case '\"': case '\\': case '/': *ptr2++ = *ptr; break;
                default: *ptr2++ = *ptr; break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;
    if (*ptr == '\"') ptr++;
    item->valuestring = out;
    item->type = cJSON_String;
    return ptr;
}

static char *print_string_ptr(const char *str, printbuffer *p)
{
    const char *ptr;
    char *ptr2, *out;
    int len = 0, flag = 0;
    unsigned char token;

    for (ptr = str; *ptr; ptr++) flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
    if (!flag) {
        len = (int)(ptr - str);
        if (p) out = ensure(p, len + 3);
        else out = (char*)cJSON_malloc(len + 3);
        if (!out) return 0;
        ptr2 = out;
        *ptr2++ = '\"';
        strcpy(ptr2, str);
        ptr2[len] = '\"';
        ptr2[len + 1] = 0;
        return out;
    }

    if (!str) {
        if (p) out = ensure(p, 3);
        else out = (char*)cJSON_malloc(3);
        if (!out) return 0;
        strcpy(out, "\"\"");
        return out;
    }
    ptr = str;
    while ((token = *ptr) && ++len) {
        if (strchr("\"\\\b\f\n\r\t", token)) len++;
        else if (token < 32) len += 5;
        ptr++;
    }

    if (p) out = ensure(p, len + 3);
    else out = (char*)cJSON_malloc(len + 3);
    if (!out) return 0;

    ptr2 = out;
    ptr = str;
    *ptr2++ = '\"';
    while (*ptr) {
        if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\') *ptr2++ = *ptr++;
        else {
            *ptr2++ = '\\';
            switch (token = *ptr++) {
                case '\\': *ptr2++ = '\\'; break;
                case '\"': *ptr2++ = '\"'; break;
                case '\b': *ptr2++ = 'b'; break;
                case '\f': *ptr2++ = 'f'; break;
                case '\n': *ptr2++ = 'n'; break;
                case '\r': *ptr2++ = 'r'; break;
                case '\t': *ptr2++ = 't'; break;
                default: sprintf(ptr2, "u%04x", token); ptr2 += 5; break;
            }
        }
    }
    *ptr2++ = '\"';
    *ptr2++ = 0;
    return out;
}

static const char *parse_value(cJSON *item, const char *value);
static char *print_value(cJSON *item, int depth, int fmt, printbuffer *p);

static const char *skip(const char *in) { while (in && *in && (unsigned char)*in <= 32) in++; return in; }

cJSON *cJSON_Parse(const char *value)
{
    cJSON *c = cJSON_New_Item();
    if (!c) return 0;
    if (!parse_value(c, skip(value))) { cJSON_Delete(c); return 0; }
    return c;
}

char *cJSON_Print(const cJSON *item)
{
    printbuffer p;
    p.buffer = (char*)cJSON_malloc(256);
    p.length = 256;
    p.offset = 0;
    return print_value((cJSON*)item, 0, 1, &p);
}

char *cJSON_PrintUnformatted(const cJSON *item)
{
    printbuffer p;
    p.buffer = (char*)cJSON_malloc(256);
    p.length = 256;
    p.offset = 0;
    return print_value((cJSON*)item, 0, 0, &p);
}

/* Parser core */
static const char *parse_array(cJSON *item, const char *value)
{
    cJSON *child;
    if (*value != '[') return 0;

    item->type = cJSON_Array;
    value = skip(value + 1);
    if (*value == ']') return value + 1;

    item->child = child = cJSON_New_Item();
    if (!item->child) return 0;
    value = skip(parse_value(child, skip(value)));
    if (!value) return 0;

    while (*value == ',') {
        cJSON *new_item = cJSON_New_Item();
        if (!new_item) return 0;
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip(parse_value(child, skip(value + 1)));
        if (!value) return 0;
    }

    if (*value == ']') return value + 1;
    return 0;
}

static char *print_array(cJSON *item, int depth, int fmt, printbuffer *p)
{
    char **entries;
    char *out = 0, *ptr, *ret;
    int len = 5;
    cJSON *child = item->child;
    int numentries = 0, i = 0, fail = 0;

    while (child) numentries++, child = child->next;
    if (!numentries) {
        if (p) out = ensure(p, 3);
        else out = (char*)cJSON_malloc(3);
        if (out) strcpy(out, "[]");
        return out;
    }

    entries = (char**)cJSON_malloc(numentries * sizeof(char*));
    if (!entries) return 0;
    memset(entries, 0, numentries * sizeof(char*));

    child = item->child;
    while (child && !fail) {
        ret = print_value(child, depth + 1, fmt, 0);
        entries[i++] = ret;
        if (ret) len += (int)strlen(ret) + 2 + (fmt ? 1 : 0);
        else fail = 1;
        child = child->next;
    }

    if (!fail) {
        if (p) out = ensure(p, len);
        else out = (char*)cJSON_malloc(len);
    }
    if (!out) fail = 1;

    if (fail) {
        for (i = 0; i < numentries; i++) if (entries[i]) cJSON_free(entries[i]);
        cJSON_free(entries);
        return 0;
    }

    *out = '[';
    ptr = out + 1;
    *ptr = 0;
    for (i = 0; i < numentries; i++) {
        strcpy(ptr, entries[i]);
        ptr += strlen(entries[i]);
        if (i != numentries - 1) { *ptr++ = ','; if (fmt) *ptr++ = ' '; *ptr = 0; }
        cJSON_free(entries[i]);
    }
    cJSON_free(entries);
    *ptr++ = ']';
    *ptr++ = 0;
    return out;
}

static const char *parse_object(cJSON *item, const char *value)
{
    cJSON *child;
    if (*value != '{') return 0;

    item->type = cJSON_Object;
    value = skip(value + 1);
    if (*value == '}') return value + 1;

    item->child = child = cJSON_New_Item();
    if (!item->child) return 0;
    value = skip(parse_string(child, skip(value)));
    if (!value) return 0;
    child->string = child->valuestring;
    child->valuestring = 0;
    if (*value != ':') return 0;
    value = skip(parse_value(child, skip(value + 1)));
    if (!value) return 0;

    while (*value == ',') {
        cJSON *new_item = cJSON_New_Item();
        if (!new_item) return 0;
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip(parse_string(child, skip(value + 1)));
        if (!value) return 0;
        child->string = child->valuestring;
        child->valuestring = 0;
        if (*value != ':') return 0;
        value = skip(parse_value(child, skip(value + 1)));
        if (!value) return 0;
    }

    if (*value == '}') return value + 1;
    return 0;
}

static char *print_object(cJSON *item, int depth, int fmt, printbuffer *p)
{
    char **entries = 0, **names = 0;
    char *out = 0, *ptr, *ret, *str;
    int len = 7, i = 0, numentries = 0, fail = 0;
    cJSON *child = item->child;

    while (child) numentries++, child = child->next;
    if (!numentries) {
        if (p) out = ensure(p, fmt ? depth + 4 : 3);
        else out = (char*)cJSON_malloc(fmt ? depth + 4 : 3);
        if (out) strcpy(out, "{}");
        return out;
    }

    entries = (char**)cJSON_malloc(numentries * sizeof(char*));
    if (!entries) return 0;
    names = (char**)cJSON_malloc(numentries * sizeof(char*));
    if (!names) { cJSON_free(entries); return 0; }
    memset(entries, 0, sizeof(char*) * numentries);
    memset(names, 0, sizeof(char*) * numentries);

    child = item->child;
    depth++;
    while (child) {
        names[i] = str = print_string_ptr(child->string, 0);
        entries[i++] = ret = print_value(child, depth, fmt, 0);
        if (str && ret) len += (int)strlen(ret) + (int)strlen(str) + 2 + (fmt ? 2 + depth : 0);
        else fail = 1;
        child = child->next;
    }

    if (!fail) {
        if (p) out = ensure(p, len);
        else out = (char*)cJSON_malloc(len);
    }
    if (!out) fail = 1;

    if (fail) {
        for (i = 0; i < numentries; i++) {
            if (names[i]) cJSON_free(names[i]);
            if (entries[i]) cJSON_free(entries[i]);
        }
        cJSON_free(names);
        cJSON_free(entries);
        return 0;
    }

    *out = '{';
    ptr = out + 1;
    if (fmt) *ptr++ = '\n';
    *ptr = 0;
    for (i = 0; i < numentries; i++) {
        if (fmt) for (int j = 0; j < depth; j++) *ptr++ = ' ';
        strcpy(ptr, names[i]);
        ptr += strlen(names[i]);
        *ptr++ = ':';
        if (fmt) *ptr++ = ' ';
        strcpy(ptr, entries[i]);
        ptr += strlen(entries[i]);
        if (i != numentries - 1) *ptr++ = ',';
        if (fmt) *ptr++ = '\n';
        *ptr = 0;
        cJSON_free(names[i]);
        cJSON_free(entries[i]);
    }
    cJSON_free(names);
    cJSON_free(entries);
    if (fmt) for (int j = 0; j < depth - 1; j++) *ptr++ = ' ';
    *ptr++ = '}';
    *ptr++ = 0;
    return out;
}

static const char *parse_value(cJSON *item, const char *value)
{
    if (!value) return 0;
    if (!strncmp(value, "null", 4)) { item->type = cJSON_NULL; return value + 4; }
    if (!strncmp(value, "false", 5)) { item->type = cJSON_False; return value + 5; }
    if (!strncmp(value, "true", 4)) { item->type = cJSON_True; item->valueint = 1; return value + 4; }
    if (*value == '\"') { return parse_string(item, value); }
    if (*value == '-' || (*value >= '0' && *value <= '9')) { return parse_number(item, value); }
    if (*value == '[') { return parse_array(item, value); }
    if (*value == '{') { return parse_object(item, value); }
    return 0;
}

static char *print_value(cJSON *item, int depth, int fmt, printbuffer *p)
{
    char *out = 0;
    if (!item) return 0;
    switch ((item->type) & 255) {
        case cJSON_NULL:
            if (p) out = ensure(p, 5); else out = (char*)cJSON_malloc(5);
            if (out) strcpy(out, "null"); break;
        case cJSON_False:
            if (p) out = ensure(p, 6); else out = (char*)cJSON_malloc(6);
            if (out) strcpy(out, "false"); break;
        case cJSON_True:
            if (p) out = ensure(p, 5); else out = (char*)cJSON_malloc(5);
            if (out) strcpy(out, "true"); break;
        case cJSON_Number: out = print_number(item, p); break;
        case cJSON_String: out = print_string_ptr(item->valuestring, p); break;
        case cJSON_Array: out = print_array(item, depth, fmt, p); break;
        case cJSON_Object: out = print_object(item, depth, fmt, p); break;
    }
    return out;
}

int cJSON_GetArraySize(const cJSON *array)
{
    cJSON *c = array->child;
    int i = 0;
    while (c) i++, c = c->next;
    return i;
}

cJSON *cJSON_GetArrayItem(const cJSON *array, int index)
{
    cJSON *c = array->child;
    while (c && index > 0) index--, c = c->next;
    return c;
}

cJSON *cJSON_GetObjectItem(const cJSON * const object, const char * const string)
{
    cJSON *c = object->child;
    while (c && cJSON_strcasecmp(c->string, string)) c = c->next;
    return c;
}

cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string)
{
    cJSON *c = object->child;
    while (c && strcmp(c->string, string)) c = c->next;
    return c;
}

int cJSON_HasObjectItem(const cJSON *object, const char *string)
{
    return cJSON_GetObjectItem(object, string) != NULL;
}

cJSON *cJSON_CreateNull(void) { cJSON *item = cJSON_New_Item(); if (item) item->type = cJSON_NULL; return item; }
cJSON *cJSON_CreateTrue(void) { cJSON *item = cJSON_New_Item(); if (item) item->type = cJSON_True; return item; }
cJSON *cJSON_CreateFalse(void) { cJSON *item = cJSON_New_Item(); if (item) item->type = cJSON_False; return item; }
cJSON *cJSON_CreateBool(int b) { cJSON *item = cJSON_New_Item(); if (item) item->type = b ? cJSON_True : cJSON_False; return item; }
cJSON *cJSON_CreateNumber(double num) { cJSON *item = cJSON_New_Item(); if (item) { item->type = cJSON_Number; item->valuedouble = num; item->valueint = (int)num; } return item; }
cJSON *cJSON_CreateString(const char *string) { cJSON *item = cJSON_New_Item(); if (item) { item->type = cJSON_String; item->valuestring = cJSON_strdup(string); } return item; }
cJSON *cJSON_CreateArray(void) { cJSON *item = cJSON_New_Item(); if (item) item->type = cJSON_Array; return item; }
cJSON *cJSON_CreateObject(void) { cJSON *item = cJSON_New_Item(); if (item) item->type = cJSON_Object; return item; }

void cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
    cJSON *c = array->child;
    if (!item) return;
    if (!c) { array->child = item; }
    else { while (c && c->next) c = c->next; c->next = item; item->prev = c; }
}

void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    if (!item) return;
    if (item->string) cJSON_free(item->string);
    item->string = cJSON_strdup(string);
    cJSON_AddItemToArray(object, item);
}

void cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item)
{
    if (!item) return;
    if (!(item->type & cJSON_StringIsConst) && item->string) cJSON_free(item->string);
    item->string = (char*)string;
    item->type |= cJSON_StringIsConst;
    cJSON_AddItemToArray(object, item);
}
