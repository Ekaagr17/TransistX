/* cJSON.h - Ultralightweight JSON parser in ANSI C */
#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

/* cJSON Types: */
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7)

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

/* The cJSON structure: */
typedef struct cJSON
{
    struct cJSON *next;
    struct cJSON *prev;
    struct cJSON *child;

    int type;

    char *valuestring;
    int valueint;
    double valuedouble;

    char *string;
} cJSON;

/* Memory management */
typedef struct cJSON_Hooks
{
    void *(*malloc_fn)(size_t sz);
    void (*free_fn)(void *ptr);
} cJSON_Hooks;

void cJSON_InitHooks(cJSON_Hooks* hooks);

/* Supply a block of JSON, and this returns a cJSON object you can interrogate. */
cJSON *cJSON_Parse(const char *value);
/* Render a cJSON entity to text for transfer/storage. */
char *cJSON_Print(const cJSON *item);
/* Render a cJSON entity to text for transfer/storage without any formatting. */
char *cJSON_PrintUnformatted(const cJSON *item);
/* Delete a cJSON entity and all subentities. */
void cJSON_Delete(cJSON *c);

/* Returns the number of items in an array (or object). */
int cJSON_GetArraySize(const cJSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if not found. */
cJSON *cJSON_GetArrayItem(const cJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
cJSON *cJSON_GetObjectItem(const cJSON * const object, const char * const string);
cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string);
int cJSON_HasObjectItem(const cJSON *object, const char *string);

/* Helper functions for creating and adding items to an object at the same time. */
cJSON *cJSON_CreateNull(void);
cJSON *cJSON_CreateTrue(void);
cJSON *cJSON_CreateFalse(void);
cJSON *cJSON_CreateBool(int b);
cJSON *cJSON_CreateNumber(double num);
cJSON *cJSON_CreateString(const char *string);
cJSON *cJSON_CreateRaw(const char *raw);
cJSON *cJSON_CreateArray(void);
cJSON *cJSON_CreateObject(void);

/* Append item to the specified array/object. */
void cJSON_AddItemToArray(cJSON *array, cJSON *item);
void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);
void cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item);

/* Macros for creating things more easily. */
#define cJSON_AddNullToObject(object,name) cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object,name) cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object,name) cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddBoolToObject(object,name,b) cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))
#define cJSON_AddNumberToObject(object,name,n) cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object,name,s) cJSON_AddItemToObject(object, name, cJSON_CreateString(s))
#define cJSON_AddRawToObject(object,name,s) cJSON_AddItemToObject(object, name, cJSON_CreateRaw(s))

/* Type check macros */
#define cJSON_IsInvalid(item) ((item) == NULL || ((item)->type & 0xFF) == cJSON_Invalid)
#define cJSON_IsFalse(item) ((item) != NULL && ((item)->type & 0xFF) == cJSON_False)
#define cJSON_IsTrue(item) ((item) != NULL && ((item)->type & 0xFF) == cJSON_True)
#define cJSON_IsBool(item) ((item) != NULL && (((item)->type & (cJSON_True | cJSON_False)) != 0))
#define cJSON_IsNull(item) ((item) != NULL && ((item)->type & 0xFF) == cJSON_NULL)
#define cJSON_IsNumber(item) ((item) != NULL && ((item)->type & 0xFF) == cJSON_Number)
#define cJSON_IsString(item) ((item) != NULL && ((item)->type & 0xFF) == cJSON_String)
#define cJSON_IsArray(item) ((item) != NULL && ((item)->type & 0xFF) == cJSON_Array)
#define cJSON_IsObject(item) ((item) != NULL && ((item)->type & 0xFF) == cJSON_Object)
#define cJSON_IsRaw(item) ((item) != NULL && ((item)->type & 0xFF) == cJSON_Raw)

#ifdef __cplusplus
}
#endif

#endif
