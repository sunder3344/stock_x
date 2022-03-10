cJSON.h                                                                                             0000644 0000000 0000000 00000036724 14055136534 010663  0                                                                                                    ustar   root                            root                                                                                                                                                                                                                   /*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 3 define options:

CJSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
CJSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
CJSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the CJSON_API_VISIBILITY flag to "export" the same symbols the way CJSON_EXPORT_SYMBOLS does

*/

#define CJSON_CDECL __cdecl
#define CJSON_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(CJSON_HIDE_SYMBOLS) && !defined(CJSON_IMPORT_SYMBOLS) && !defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_EXPORT_SYMBOLS
#endif

#if defined(CJSON_HIDE_SYMBOLS)
#define CJSON_PUBLIC(type)   type CJSON_STDCALL
#elif defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllexport) type CJSON_STDCALL
#elif defined(CJSON_IMPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllimport) type CJSON_STDCALL
#endif
#else /* !__WINDOWS__ */
#define CJSON_CDECL
#define CJSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(CJSON_API_VISIBILITY)
#define CJSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define CJSON_PUBLIC(type) type
#endif
#endif

/* project version */
#define CJSON_VERSION_MAJOR 1
#define CJSON_VERSION_MINOR 7
#define CJSON_VERSION_PATCH 14

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
#define cJSON_Raw    (1 << 7) /* raw json */

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

/* The cJSON structure: */
typedef struct cJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *next;
    struct cJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct cJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==cJSON_String  and type == cJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use cJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==cJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} cJSON;

typedef struct cJSON_Hooks
{
      /* malloc/free are CDECL on Windows regardless of the default calling convention of the compiler, so ensure the hooks allow passing those functions directly. */
      void *(CJSON_CDECL *malloc_fn)(size_t sz);
      void (CJSON_CDECL *free_fn)(void *ptr);
} cJSON_Hooks;

typedef int cJSON_bool;

/* Limits how deeply nested arrays/objects can be before cJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT 1000
#endif

/* returns the version of cJSON as a string */
CJSON_PUBLIC(const char*) cJSON_Version(void);

/* Supply malloc, realloc and free functions to cJSON */
CJSON_PUBLIC(void) cJSON_InitHooks(cJSON_Hooks* hooks);

/* Memory Management: the caller is always responsible to free the results from all variants of cJSON_Parse (with cJSON_Delete) and cJSON_Print (with stdlib free, cJSON_Hooks.free_fn, or cJSON_free as appropriate). The exception is cJSON_PrintPreallocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a cJSON object you can interrogate. */
CJSON_PUBLIC(cJSON *) cJSON_Parse(const char *value);
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLength(const char *value, size_t buffer_length);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match cJSON_GetErrorPtr(). */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated);
CJSON_PUBLIC(cJSON *) cJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, cJSON_bool require_null_terminated);

/* Render a cJSON entity to text for transfer/storage. */
CJSON_PUBLIC(char *) cJSON_Print(const cJSON *item);
/* Render a cJSON entity to text for transfer/storage without any formatting. */
CJSON_PUBLIC(char *) cJSON_PrintUnformatted(const cJSON *item);
/* Render a cJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
CJSON_PUBLIC(char *) cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt);
/* Render a cJSON entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
/* NOTE: cJSON is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
CJSON_PUBLIC(cJSON_bool) cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format);
/* Delete a cJSON entity and all subentities. */
CJSON_PUBLIC(void) cJSON_Delete(cJSON *item);

/* Returns the number of items in an array (or object). */
CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
CJSON_PUBLIC(cJSON *) cJSON_GetArrayItem(const cJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItem(const cJSON * const object, const char * const string);
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string);
CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when cJSON_Parse() returns 0. 0 when cJSON_Parse() succeeds. */
CJSON_PUBLIC(const char *) cJSON_GetErrorPtr(void);

/* Check item type and return its value */
CJSON_PUBLIC(char *) cJSON_GetStringValue(const cJSON * const item);
CJSON_PUBLIC(double) cJSON_GetNumberValue(const cJSON * const item);

/* These functions check the type of an item */
CJSON_PUBLIC(cJSON_bool) cJSON_IsInvalid(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsFalse(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsTrue(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsBool(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsNull(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsNumber(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsString(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsArray(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsObject(const cJSON * const item);
CJSON_PUBLIC(cJSON_bool) cJSON_IsRaw(const cJSON * const item);

/* These calls create a cJSON item of the appropriate type. */
CJSON_PUBLIC(cJSON *) cJSON_CreateNull(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateTrue(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateFalse(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateBool(cJSON_bool boolean);
CJSON_PUBLIC(cJSON *) cJSON_CreateNumber(double num);
CJSON_PUBLIC(cJSON *) cJSON_CreateString(const char *string);
/* raw json */
CJSON_PUBLIC(cJSON *) cJSON_CreateRaw(const char *raw);
CJSON_PUBLIC(cJSON *) cJSON_CreateArray(void);
CJSON_PUBLIC(cJSON *) cJSON_CreateObject(void);

/* Create a string where valuestring references a string so
 * it will not be freed by cJSON_Delete */
CJSON_PUBLIC(cJSON *) cJSON_CreateStringReference(const char *string);
/* Create an object/array that only references it's elements so
 * they will not be freed by cJSON_Delete */
CJSON_PUBLIC(cJSON *) cJSON_CreateObjectReference(const cJSON *child);
CJSON_PUBLIC(cJSON *) cJSON_CreateArrayReference(const cJSON *child);

/* These utilities create an Array of count items.
 * The parameter count cannot be greater than the number of elements in the number array, otherwise array access will be out of bounds.*/
CJSON_PUBLIC(cJSON *) cJSON_CreateIntArray(const int *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateFloatArray(const float *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateDoubleArray(const double *numbers, int count);
CJSON_PUBLIC(cJSON *) cJSON_CreateStringArray(const char *const *strings, int count);

/* Append item to the specified array/object. */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToArray(cJSON *array, cJSON *item);
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the cJSON object.
 * WARNING: When this function was used, make sure to always check that (item->type & cJSON_StringIsConst) is zero before
 * writing to `item->string` */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing cJSON to a new cJSON, but don't want to corrupt your existing cJSON. */
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item);
CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item);

/* Remove/Detach items from Arrays/Objects. */
CJSON_PUBLIC(cJSON *) cJSON_DetachItemViaPointer(cJSON *parent, cJSON * const item);
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromArray(cJSON *array, int which);
CJSON_PUBLIC(void) cJSON_DeleteItemFromArray(cJSON *array, int which);
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObject(cJSON *object, const char *string);
CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObjectCaseSensitive(cJSON *object, const char *string);
CJSON_PUBLIC(void) cJSON_DeleteItemFromObject(cJSON *object, const char *string);
CJSON_PUBLIC(void) cJSON_DeleteItemFromObjectCaseSensitive(cJSON *object, const char *string);

/* Update array items. */
CJSON_PUBLIC(cJSON_bool) cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem); /* Shifts pre-existing items to the right. */
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemViaPointer(cJSON * const parent, cJSON * const item, cJSON * replacement);
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem);
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem);
CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObjectCaseSensitive(cJSON *object,const char *string,cJSON *newitem);

/* Duplicate a cJSON item */
CJSON_PUBLIC(cJSON *) cJSON_Duplicate(const cJSON *item, cJSON_bool recurse);
/* Duplicate will create a new, identical cJSON item to the one you pass, in new memory that will
 * need to be released. With recurse!=0, it will duplicate any children connected to the item.
 * The item->next and ->prev pointers are always zero on return from Duplicate. */
/* Recursively compare two cJSON items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
CJSON_PUBLIC(cJSON_bool) cJSON_Compare(const cJSON * const a, const cJSON * const b, const cJSON_bool case_sensitive);

/* Minify a strings, remove blank characters(such as ' ', '\t', '\r', '\n') from strings.
 * The input pointer json cannot point to a read-only address area, such as a string constant, 
 * but should point to a readable and writable adress area. */
CJSON_PUBLIC(void) cJSON_Minify(char *json);

/* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
CJSON_PUBLIC(cJSON*) cJSON_AddNullToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddTrueToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddFalseToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean);
CJSON_PUBLIC(cJSON*) cJSON_AddNumberToObject(cJSON * const object, const char * const name, const double number);
CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string);
CJSON_PUBLIC(cJSON*) cJSON_AddRawToObject(cJSON * const object, const char * const name, const char * const raw);
CJSON_PUBLIC(cJSON*) cJSON_AddObjectToObject(cJSON * const object, const char * const name);
CJSON_PUBLIC(cJSON*) cJSON_AddArrayToObject(cJSON * const object, const char * const name);

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define cJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
/* helper for the cJSON_SetNumberValue macro */
CJSON_PUBLIC(double) cJSON_SetNumberHelper(cJSON *object, double number);
#define cJSON_SetNumberValue(object, number) ((object != NULL) ? cJSON_SetNumberHelper(object, (double)number) : (number))
/* Change the valuestring of a cJSON_String object, only takes effect when type of object is cJSON_String */
CJSON_PUBLIC(char*) cJSON_SetValuestring(cJSON *object, const char *valuestring);

/* Macro for iterating over an array or object */
#define cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* malloc/free objects using the malloc/free functions that have been set with cJSON_InitHooks */
CJSON_PUBLIC(void *) cJSON_malloc(size_t size);
CJSON_PUBLIC(void) cJSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif
                                            config_x.ini                                                                                        0000644 0000000 0000000 00000000473 14212320543 012051  0                                                                                                    ustar   root                            root                                                                                                                                                                                                                   SH000001
SZ399001
SZ399006
SH000688
SZ000650
SZ300568
SH600703
SZ002812
SH601615
SH688126
SH688396
SH600031
SZ002797
SZ002332
SZ300253
SH600789
SZ300803
SZ002456
SZ300115
SZ002436
SH600584
SH600460
SZ002065
SZ002129
SH605179
SH688011
SZ300113
SZ300339
SH600036
SZ300821
SH688189
SH688508
SH688037
SH688521
SH688012
                                                                                                                                                                                                     config_x.ini.bk                                                                                     0000644 0000000 0000000 00000000473 14212073163 012447  0                                                                                                    ustar   root                            root                                                                                                                                                                                                                   SH000001
SZ399001
SZ399006
SH000688
SZ000650
SZ300568
SZ600703
SZ002812
SH601615
SH688126
SH688396
SH600031
SZ002797
SZ002332
SZ300253
SZ600789
SZ300803
SZ002456
SZ300115
SZ002436
SH600584
SH600460
SZ002065
SZ002129
SH605179
SH688011
SZ300113
SZ300339
SH600036
SZ300821
SH688189
SH688508
SH688037
SH688521
SH688012
                                                                                                                                                                                                     stock_x                                                                                             0000755 0000000 0000000 00000303600 14212320226 011150  0                                                                                                    ustar   root                            root                                                                                                                                                                                                                   ELF          >    @     @       �~         @ 8 	 @ $ #       @       @ @     @ @     �      �                   8      8@     8@                                          @       @     ��      ��                    ��      ��`     ��`     X      �                    ��      ��`     ��`     �      �                   T      T@     T@     D       D              P�td   `r      `r@     `r@     �      �             Q�td                                                  R�td   ��      ��`     ��`                          /lib64/ld-linux-x86-64.so.2          GNU                        GNU j��v҇�y���AO8             �@   (    #       BE���|��=��|<�9�qX                                                 �                                           y                      �                      �                      Y                                          /                     `                     |                     G                     g                                          �                      S                                           n                     �                      
                     e                      �                     �                      �                      ?                     N                     S                                            �                     *                       9                       �    8�`             �    x�`             7      @             �     p@             u     �@             �    8�`              libcurl.so.4 _ITM_deregisterTMCloneTable __gmon_start__ _ITM_registerTMCloneTable curl_easy_cleanup curl_slist_free_all curl_easy_init curl_easy_setopt curl_easy_perform curl_slist_append libiconv.so.2 libiconv_open libiconv libiconv_close libc.so.6 strcpy sprintf fopen strncmp perror __isoc99_sscanf strncpy realloc tolower strtod feof fgets strlen memset memcpy fclose malloc strncat strcmp __libc_start_main free _edata __bss_start _end GLIBC_2.7 GLIBC_2.14 GLIBC_2.2.5                                                       �          ii   �     ���   �     ui	   �      ��`                   �`                   ��`                   ��`                   �`                    �`                   (�`                   0�`                   8�`                   @�`                   H�`                   P�`                   X�`        	           `�`        
           h�`                   p�`                   x�`                   ��`                   ��`                   ��`                   ��`                   ��`                   ��`                   ��`                   ��`        #           ��`                   Ȑ`        $           А`                   ؐ`                   ��`                   �`                   �`                   ��`                    �`        "           ��H��H��  H��t��H���     �5�  �%�  @ �%�  h    ������%ڃ  h   ������%҃  h   ������%ʃ  h   �����%  h   �����%��  h   �����%��  h   �����%��  h   �p����%��  h   �`����%��  h	   �P����%��  h
   �@����%��  h   �0����%��  h   � ����%z�  h   �����%r�  h   � ����%j�  h   ������%b�  h   ������%Z�  h   ������%R�  h   ������%J�  h   �����%B�  h   �����%:�  h   �����%2�  h   �����%*�  h   �p����%"�  h   �`����%�  h   �P����%�  h   �@����%
�  h   �0����%�  h   � ����%��  h   ������1�I��^H��H���PTI�� i@ H���h@ H�Ǉ@ ���  �����f.�     �H�=�  H�ڂ  H9�tH�v�  H��t	���    ��    H�=��  H�5��  H)�H��H��H��?H�H��tH�M�  H��t��fD  ��    ���=u�   uUH���z����c�  ]Ð�D  f.�     ���UH��H��PH�}�H�u�H�U�H�M�L�E�L�M�H�E�H�E�H�E�H�E�H�U�H�E�H��H������H�E�H�}� u������RH�U�H�E��    H���`���H�}�H�M�H�U�H�u�H�E�I��H�������H���u������H�E�H�������    ��UH��H�� H�}��u�H�U�M��E�Hc��E�Hc�H�M�H�E�I��I��H��H�¾Pi@ �Wi@ ������UH��H�� H�}�H�u�H�U�H�M�H�u�H�M�H�U�H�E�I��I��H��H�¾Wi@ �Pi@ �������UH��H��@H�}؉�H�UȈE�H�E�H���D����E��E�    �E�    H�E�H�ƿ]i@ �    �����E�    �  �E�Hc�H�E�H�� 8E���   �E�+E��ƿpi@ �    �����E�+E�H�H�������H�E�E�+E�HcЋE�Hc�H�E�H�H�E�H��H�������H�E�H�ƿzi@ �    �2����E�H�H��    H�E�H�H�E�H��E�ƿ�i@ �    �����E�H�H��    H�E�H�H� H�ƿ]i@ �    ������E�H�H��    H�E�H�H� H�ƿ�i@ �    �����E����E��E��E��E�;E�������E�;E�}$�E�HcȋE�H�H��    H�E�H�H�U�H�H����UH��H��@H�}؉�H�UȈE�H�E�H�������E��E�    �E�    �E�    �   �E�Hc�H�E�H�� 8E�u�E�Hc�H�E�H��  �E�+E���H�H���O���H�E�E�+E���HcЋE�Hc�H�E�H�H�E�H��H���C���H�E�H�E��E�H�H��    H�E�H�H�E�H��E����E��E��E��E�;E��\����E�;E�}$�E�HcȋE�H�H��    H�E�H�H�U�H�H����UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��]�UH��SH��(H�}�H�u�H�U�H�M�H��}  H��uAH�E�H������H�PH��}  H��H������H�t}  H�m}  H�U�H��H��������cH�U}  H���m���H��H�E�H���^���H�H�PH�0}  H��H������H�}  H�E�H���2���H��H�}  H�M�H��H���Y���H�E�H�E�H��([]�UH��SH��  ������H��������  �����H��|  H��|      ��i@ ��i@ ����H�E�H�}� u
��i@ � ���h0j@ hEj@ hTj@ haj@ A��i@ A��i@ ��i@ ��i@ ��i@ ��i@ �    ����H�� �
   �d���H�E�H�E�    H�E�nj@ �%  H������
   �    H���b���H�U�H������
   H���z���H����   H�����H�������E�H�}� u@�Ẽ�Hc�H�E�H��H���P���H�E�Ẽ�Hc�H�����H�E�H��H��������   H�E�H�������EȋEȃ�Hc�H�E�H��H������H�E�H�M�H�E�   H��H������H�E�H���i����EȋUȋE�Ѓ�Hc�H�E�H��H������H�E�Ẽ�Hc�H�����H�E�H��H���f���H�E�H���j����������H�E�H�����������H�E�H�E�    H�E�pj@ H�E�H�������H��H�������H�E�H�E�H�������H��H�M�H�E�H��H�������H�E�H�������E��E�Hc�H�E�H������H�H�PH�E�H��H�������H�E�H�E�H���h���H��H�M�H�E�H��H������H�E�    H�E���j@ H���)���H�E���j@ H������H�E���j@ H������H�E���j@ H�������H�E��k@ H��������E�'  �M�H�U�H�E���H�Ǹ    ������E�   �M�H�E��   ��H�Ǹ    ������E�P   �M�H�E��   ��H�Ǹ    �����E�T   �M�H�E��   ��H�Ǹ    �����E�v'  �M�H�E���k@ ��H�Ǹ    �l����E�+N  �M�H�E���@ ��H�Ǹ    �J����E�''  �M�H�U�H�E���H�Ǹ    �)����E�@   �M�H�E��    ��H�Ǹ    ����ǅ|���Q   ��|���H�E��    ��H�Ǹ    �����H�E�H���c�����x���H�E�H������H�E�H���%���H�nx  H���:  H��p���Hǅh���    H�E�    Hǅ`���    HǅX���    H��p�����k@ H���^1  H��`���H��`���H���K  ����  H��`����@0f��f.���  f��f.���  H��p�����k@ H���1  H��h���H��h��� tH��h���H�@��    H�E��  H�Eྟk@ H����0  H��P���H�Eྦk@ H���0  H��H���H�Eྮk@ H���0  H��@���H�Eྶk@ H���0  H��8���H�E��k@ H���g0  H��0���H�E��k@ H���O0  H��(���H�E��k@ H���70  H�� ���H�E��k@ H���0  H�����H�E��k@ H���0  H�����H������H(��Mb������������)�H������H(���Q����������)�H�� ����h0H��(����`0H��0����X0H��8����P0H��@����H0H��H����@0H��P���H�@ ��H�ƿ�k@ �   �����H�E�H� H�E�H�}� �o���H��p���H����  H�E�H�������H��u  H�������H�E�H��������    H�]���UH��H��u  H��u  H�]�UH��H��H�}�H�E�H���H  ��u�    �H�E�H�@ ��UH��H��H�}�H�E�H���cH  ��u
�V  �	H�E��@0��UH��A�   �   �   �0l@ �`�` �    �����`�` ]�UH��SH��H�}�H�u�H�}� tH�}� u�   �}H�E�H;E�u#�    �lH�E�� ��u�    �ZH�E�H�E�H�E�� �����"�����H�E�� ��������9�t�H�E�� �����������H�E�� ���������)É�H��[]�UH��H�� H�}�H�u�H�E�    H�E�    H�}� u�    �QH�E�H���b���H��H�E�H�E�H� H�U�H����H�E�H�}� u�    �H�U�H�M�H�E�H��H������H�E���UH��H�}�H�}� u&H��s  �@ H��s  p@ H��s   @ �   H�ks  �@ H�E�H� H��tH�E�H� H�Qs  H�Ns  p@ H�E�H�@H��tH�E�H�@H�2s  H�/s      H�s  H=�@ uH�s  H=p@ uH�s   @ ]�UH��H�� H�}�H�E�H� �@   ��H�E�H�}� tH�E��@   �    H���Y���H�E���UH��H�� H�}�H�E�    �   H�E�H� H�E�H�E�@%   ��uH�E�H�@H��tH�E�H�@H������H�E�@%   ��u!H�E�H�@ H��tH�Fr  H�U�H�R H����H�E�@%   ��u!H�E�H�@8H��tH�r  H�U�H�R8H����H�r  H�U�H����H�E�H�E�H�}� �C������UH��.   ]�UH��H��pH�}�H�u�f���E�H�E�    ������E�H�E�    H�}� tH�E�H� H��u
�    �{  H�E�    �uH�E�H�H�E�H�HH�E�H�H�� ����+��:w~��H��@l@ ��H�E�H�H�E�H�HH�E�H�H�� H�M�H�U�Hʈ�H�U�H�E�H��E��H�E�H�}�>w,H�}� t%H�E�H�PH�E�H�H�E�H�@H9��]�������H�U�H�E�H��  H�U�H�E�H��H���m���fH~�H�E�H�U�H�E�H9�u
�    �   H�E��E��@0�E�f/�Q  rH�E��@(����,�sQ  f/E�rH�E��@(   ���E��,�H�E��P(H�E��@   H�E�H�@H�M�H�U�H)�H��H�H�E�H�P�   ��UH��H�}��E��E�f/�P  rH�E��@(����,��P  f/E�rH�E��@(   ���E��,�H�E��P(H�E��E��@0H�E��@0]�UH��SH��(H�}�H�u�H�E�    H�E؋@����tH�E؋@%   ��t
�    �   H�E�H������H��H�E�H�@ H������H9�w!H�E�H�@ H�U�H��H���
���H�E�H�@ �PH�Eо �` H�������H�E�H�}� u�    �-H�E�H�@ H��tH�E�H�@ H���@F  H�E�H�U�H�P H�E�H��([]�UH��H�� H�}�H�u�H�E�    H�E�    H�}� tH�E�H� H��u
�    ��  H�E�H�@H��tH�E�H�PH�E�H�@H9�r
�    �  H�}����v
�    �  H�E�H�PH�E�H�H��H�E�H�E�H�@H9E�wH�E�H�H�E�H�@H��^  H�E�@ ��t
�    �I  H�}����?vH�}����w
H�E������    �!  H�E�H�H�E�H�E�H�@8H��t_H�E�H�@8H�U�H�H�M�H��H����H�E�H�}� ��   H�E�H�@0H�U�H�H����H�E�H�@    H�E�H�     �    �   H�E�H�@(H�U�H����H�E�H�}� u2H�E�H�@0H�U�H�H����H�E�H�@    H�E�H�     �    �\H�E�H�@H�PH�E�H�H�E�H��H������H�E�H�@0H�U�H�H����H�E�H�U�H�PH�E�H�U�H�H�E�H�PH�E�H���UH��SH��(H�}�H�E�    H�}� tDH�E�H� H��t8H�E�H�H�E�H�@H�H�E�H�E�H�XH�E�H������H�H�E�H�P��H��([]�UH���E��M��E��~=M  fT��M��~,M  fT�f/�v�E��~M  fT���E��~M  fT��E��E��\E��~�L  fT��U���L  �Y�f/�����]�UH��H��`H�}�H�u�H�E�    H�E��@0�E��E�    H�E�    H�E�    H�E�    H�E�    f�E�  �t����E�f���E�H�}� u
�    �~  �E�f.E�z#�E��~8L  fT�f.DL  ������tH�E��n@ H�Ǹ    �����E��n�E�H�E��n@ H�Ǹ   ������E�H�U�H�E��$n@ H�Ǹ    ������u�E��M��c�����u�E�H�E��(n@ H�Ǹ   �����E��}� x�}�~
�    �   �E�H�H�PH�E�H��H������H�E�H�}� u
�    �   H�E�    �CH�U�H�E�H�� 8E�uH�U�H�E�H�� .�H�U�H�E�H�H�M�H�E�H�� �H�E��E�H�H9E�r�H�U�H�E�H��  H�E�H�P�E�H�H�H�E�H�P�   ��UH��H�}��E�    H�E�    H�E�    ��   H�U�H�E�H�� </v3H�U�H�E�H�� <9w!H�U�H�E�H�� �ЋE�Ѓ�0�E��   H�U�H�E�H�� <@v0H�U�H�E�H�� <FwH�U�H�E�H�� �ЋE�Ѓ�7�E��IH�U�H�E�H�� <`v0H�U�H�E�H�� <fwH�U�H�E�H�� �ЋE�Ѓ�W�E���    �H�}�w�e�H�E�H�}������E�]�UH��H��HH�}�H�u�H�U�H�E�    �E�    H�E�H�E��E� �E� �E� �E� H�E�H+E�H����  H�E�H��H�������E��}���  v�}���  ��  �}���  ��   �}���  ��   H�E�H��H�E��E�    �E�H�E�H+E�H���U  H�E�� <\�I  H�E�H��� <u�6  H�E�H��H�������E܁}���  �  �}���  �  �E���
% � �E�%�  	�   ��H�E���E��E�H�E�H�}�w�E��>H�}��  w
�E��E���*H�}���  w
�E��E���H�}��� ��   �E��E���E����E��-H�E���?��H�E�H��E�HЃɀ�ʈH�m��E����E��}� ù}�vH�E���H�E�H� 
U��H�E���H�E�H� ���H�E�H��E�H�H�E�H��E������
��������    ��UH��H��PH�}�H�u�H�E�H�H�E�H�@H��H�H�E�H�E�H�H�E�H�@H��H�H�E�H�E�    H�E�    H�E�H�H�E�H�@H�� <"�h  H�E�    H�E�    �CH�E�� <\u3H�E�H�PH�E�H� H)�H��H��H�E�H�@H9��%  H�E�H�E�H�E�H�E�H� H�U�H)�H��H��H�E�H�@H9�sH�E�� <"u�H�E�H� H�U�H)�H��H��H�E�H�@H9���  H�E�� <"��  H�E�H�H�E�H�@H�H�E�H)�H+E�H�E�H�E�H�@ H�U�H��H����H�E�H�}� �w  H�E�H�E���   H�E�� <\t"H�U�H�BH�E�H�E�H�HH�M�����   �E�H�E�H+E�H���+  H�E�H��� ����"��S�  ��H��0n@ ��H�E�H�PH�U�� �}H�E�H�PH�U�� �lH�E�H�PH�U�� 
�[H�E�H�PH�U�� �JH�E�H�PH�U�� 	�9H�E�H�PH�U�H�U��R��!H�U�H�M�H�E�H��H���}����E߀}� t~��E�HE�H�E�H;E������H�E��  H�E��@   H�E�H�U�H�P H�E�H� H�U�H)�H��H��H�E�H�PH�E�H�@H�PH�E�H�P�   �S������
�������H�}� tH�E�H�@(H�U�H����H�}� tH�E�H� H�U�H)�H��H��H�E�H�P�    ��UH��H��@H�}�H�u�H�E�    H�E�    H�E�    H�E�    H�E�    H�}� u
�    �v  H�}� u=H�E��   H���6���H�E�H�}� u
�    �I  H�E�f� ""�@ �   �2  H�E�H�E��EH�E�� ������}����w�
��"t��\uH�E��H�E�� <wH�E��H�E�H�E�� ��u�H�E�H+E�H��H�E�H�H�E�H�E�H�PH�E�H��H������H�E�H�}� u
�    �  H�}� uPH�E�� "H�E�H�HH�U�H�E�H��H�������H�E�H�PH�E�H�� "H�E�H�PH�E�H��  �   �?  H�E�� "H�E�H��H�E�H�E�H�E���   H�E�� <v(H�E�� <"tH�E�� <\tH�E��H�E���   H�E�H�PH�U�� \H�E�� ����tB����	tS��
t<��t%�P��"t��\t��t/�?H�E�� \�\H�E�� "�SH�E�� b�JH�E�� f�AH�E�� n�8H�E�� r�/H�E�� t�&H�E�� ��H�E��p@ H�Ǹ    �	���H�E��H�E�H�E�H�E�� ���
���H�E�H�PH�E�H�� "H�E�H�PH�E�H��  �   ��UH��H��H�}�H�u�H�E�H�@ H�U�H��H��������UH��H�}�H�}� tH�E�H� H��u
�    �   H�}� tH�E�H�PH�E�H�@H9�rH�E��vH�E�H�@H�PH�E�H�PH�}� t.H�E�H�PH�E�H�@H9�sH�E�H�H�E�H�@H�� < v�H�E�H�PH�E�H�@H9�uH�E�H�@H�P�H�E�H�PH�E�]�UH��H��H�}�H�}� tH�E�H� H��tH�E�H�@H��t�    �`H�}� tUH�E�H�@H�PH�E�H�@H9�s<H�E�H�H�E�H�@Hк   ��p@ H��������uH�E�H�@H�PH�E�H�PH�E���UH��H��0H�}�H�u��U�H�}� u�    �+H�E�H���U���H��H�E��M�H�U�H�u�H�E�H���   ��UH��H��pH�}�H�u�H�U��M�H�E�    H�E�    H�E�    H�E�    H�E�    H�E�    H�E�    H�E�    H��^      H��^      H�}� ��   H�}� ��   H�E�H�E�H�E�H�E�H�E�    H�"^  H�#^  H�E�H�U�H�^  H�E� �` �
���H�E�H�}� ��   H�E�H���<���H���u���H��H�E�H��H����  ��tY�}� t+H�E�H���M���H�U�H�E�H9�s=H�U�H�E�H�� ��u+H�}� tH�U�H�E�H�H�E�H�H�E��   �������H�}� tH�E�H������H�}� tkH�E�H�E�H�E�    H�U�H�E�H9�s
H�E�H�E��H�E�H��tH�E�H��H�E�H�}� tH�U�H�E�H�H�E�H�H�E�H�U�H�]  H�]  �    ��UH��H��H�}�H�E��    �    H��������UH��H��H�}�H�u�H�u�H�E��    �    H���������UH��H��pH�}��u�H�U�H�E�    H�E��@   �    H�������H�E�H� H�M=  H����H�E�H�==  H�E��E��E�H�M�H�H�QH�E�H�U�H�AH�E�H�E�H����   H�U�H�E�H��H����  ����   H�E�H���D���H�E�H�@H��t1H�E�H�@H�U�H�JH�U�H��H����H�E�H�}� tzH�E�    �dH�E�H� H�U�H��H����H�E�H�}� tTH�E�H�PH�E�H9�HG�H�M�H�E�H��H���1���H�U�H�E�H��  H�E�H�@H�U�H����H�E��A�������H�E�H��tH�E�H�@H�U�H����H�}� tH�E�H�@H�U�H���и    ��UH��H��H�}�H�E�� �` �   H���K�����UH��H��H�}�H�E�� �` �    H���'�����UH��H��PH�}��u��U�H�E�    H�E�    H�E�    H�E�    H�E�    H�E�    H�E�    H�E�    �}� y
�    �   H�HZ  �U�Hc�H����H�E�H�E�H��u�    �q�E�H�H�E�H�E�    �E�    �E��E�H�Z  H�Z  H�E�H�U�H��Y  H�E�H�U�H�E�H��H����  ��uH��Y  H�U�H���и    �H�E���UH��H��`H�}�H�u��U��M�H�E�    H�E�    H�E�    H�E�    H�E�    H�E�    H�E�    H�E�    �}� xH�}� u�    �ZH�E�H�E��E�H�H�E�H�E�    �E�   �E��E�H�Y  H� Y  H�E�H�U�H�Y  H�E�H�U�H�E�H��H����  ��UH��H��H�}�H�u�H�}� tH�E�H� H��u
�    ��  H�}� tjH�E�H�@H�PH�E�H�@H9�wQH�E�H�H�E�H�@Hк   �n@ H���������u)H�E��@   H�E�H�@H�PH�E�H�P�   �R  H�}� tjH�E�H�@H�PH�E�H�@H9�wQH�E�H�H�E�H�@Hк   ��p@ H��������u)H�E��@   H�E�H�@H�PH�E�H�P�   ��  H�}� tuH�E�H�@H�PH�E�H�@H9�w\H�E�H�H�E�H�@Hк   ��p@ H��������u4H�E��@   H�E��@(   H�E�H�@H�PH�E�H�P�   �e  H�}� tFH�E�H�PH�E�H�@H9�s1H�E�H�H�E�H�@H�� <"uH�U�H�E�H��H�������  H�}� txH�E�H�PH�E�H�@H9�scH�E�H�H�E�H�@H�� <-t2H�E�H�H�E�H�@H�� </v1H�E�H�H�E�H�@H�� <9wH�U�H�E�H��H��������   H�}� tCH�E�H�PH�E�H�@H9�s.H�E�H�H�E�H�@H�� <[uH�U�H�E�H��H���  �OH�}� tCH�E�H�PH�E�H�@H9�s.H�E�H�H�E�H�@H�� <{uH�U�H�E�H��H���  ��    ��UH��H�� H�}�H�u�H�E�    H�}� tH�}� u
�    ��  H�E�@�����  ������   ��t?��tx��  �� ��  �� ���p  �  ��@��  =�   ��   �  H�E�   H������H�E�H�}� u
�    �p  H�E�� null�@ �   �X  H�E�   H���a���H�E�H�}� u
�    �2  H�E�� falsf�@e �   �  H�E�   H���!���H�E�H�}� u
�    ��   H�E�� true�@ �   ��   H�U�H�E�H��H���������   H�E�    H�E�H�@ H��u
�    �   H�E�H�@ H�������H��H�E�H�U�H�E�H��H������H�E�H�}� u�    �fH�E�H�H H�U�H�E�H��H��������   �DH�U�H�E�H��H���u����/H�U�H�E�H��H���  �H�U�H�E�H��H����  ��    ��UH��H��0H�}�H�u�H�E�    H�E�    H�E�H�@H=�  v
�    �a  H�E�H�@H�PH�E�H�PH�E�H�H�E�H�@H�� <[�  H�E�H�@H�PH�E�H�PH�E�H�������H�}� t2H�E�H�PH�E�H�@H9�sH�E�H�H�E�H�@H�� <]�[  H�}� tH�E�H�PH�E�H�@H9�rH�E�H�@H�P�H�E�H�P�  H�E�H�@H�P�H�E�H�PH�E�H�� H�������H�E�H�}� �P  H�}� uH�E�H�E�H�E�H�E��H�E�H�U�H�H�E�H�U�H�PH�E�H�E�H�E�H�@H�PH�E�H�PH�E�H�������H�U�H�E�H��H���J�������   H�E�H�������H�}� t2H�E�H�PH�E�H�@H9�sH�E�H�H�E�H�@H�� <,�)���H�}� ��   H�E�H�PH�E�H�@H9�s~H�E�H�H�E�H�@H�� <]ue��H�E�H�@H�P�H�E�H�PH�}� tH�E�H�U�H�PH�E��@    H�E�H�U�H�PH�E�H�@H�PH�E�H�P�   �"�������H�}� tH�E�H�������    ��UH��H��0H�}�H�u�H�E�    H�E�    H�E�H�@H�E�H�}� u
�    �  H�Eо   H���-���H�E�H�}� u
�    �g  H�E�� [H�E�H�@H�PH�E�H�PH�E�H�@H�PH�E�H�P��   H�U�H�E�H��H��������u
�    �  H�E�H�������H�E�H� H����   H�EЋ@$��t�   ��   H�E�H�E�H�PH�E�H��H���u���H�E�H�}� u
�    �   H�E�H�PH�U�� ,H�EЋ@$��tH�E�H�PH�U��  H�E��  H�E�H�PH�E�H�H�E�H�PH�E�H� H�E�H�}� ����H�Eо   H�������H�E�H�}� u�    �/H�E�H�PH�U�� ]H�E��  H�E�H�@H�P�H�E�H�P�   ��UH��H��0H�}�H�u�H�E�    H�E�    H�E�H�@H=�  v
�    �3  H�E�H�@H�PH�E�H�PH�}� ��  H�E�H�PH�E�H�@H9���  H�E�H�H�E�H�@H�� <{��  H�E�H�@H�PH�E�H�PH�E�H���S���H�}� t2H�E�H�PH�E�H�@H9�sH�E�H�H�E�H�@H�� <}�  H�}� tH�E�H�PH�E�H�@H9�rH�E�H�@H�P�H�E�H�P�8  H�E�H�@H�P�H�E�H�PH�E�H�� H���3���H�E�H�}� ��  H�}� uH�E�H�E�H�E�H�E��H�E�H�U�H�H�E�H�U�H�PH�E�H�E�H�E�H�@H�PH�E�H�PH�E�H���Z���H�U�H�E�H��H���/�������  H�E�H���3���H�E�H�P H�E�H�P8H�E�H�@     H�}� �X  H�E�H�PH�E�H�@H9��?  H�E�H�H�E�H�@H�� <:�"  H�E�H�@H�PH�E�H�PH�E�H������H�U�H�E�H��H����������   H�E�H������H�}� t2H�E�H�PH�E�H�@H9�sH�E�H�H�E�H�@H�� <,�����H�}� ��   H�E�H�PH�E�H�@H9���   H�E�H�H�E�H�@H�� <}uk��H�E�H�@H�P�H�E�H�PH�}� tH�E�H�U�H�PH�E��@@   H�E�H�U�H�PH�E�H�@H�PH�E�H�P�   �(����
�������H�}� tH�E�H���M����    ��UH��H��@H�}�H�u�H�E�    H�E�    H�E�H�@H�E�H�}� u
�    �s  H�E��@$��t�   ��   H�E�H�E�H�PH�E�H��H�������H�E�H�}� u
�    �,  H�E�H�PH�U�� {H�E�H�@H�PH�E�H�PH�E��@$��tH�E�H�PH�U�� 
H�E�H�PH�E�H�H�E�H�P�  H�E��@$��tsH�E�H�PH�E�H��H���A���H�E�H�}� u
�    �  H�E�    �H�E�H�PH�U�� 	H�E�H�E�H�@H9E�r�H�E�H�PH�E�H�@H�H�E�H�PH�E�H�@8H�U�H��H���7�����u
�    �0  H�E�H�������H�E��@$��t�   ��   H�E�H�U�H�E�H��H������H�E�H�}� u
�    ��  H�E�H�PH�U�� :H�E��@$��tH�E�H�PH�U�� 	H�E�H�PH�E�H�H�E�H�PH�U�H�E�H��H���������u
�    �  H�E�H������H�E��@$��t�   ��    H�E�H� H��t�   ��    H�H�E�H�E�H�PH�E�H��H������H�E�H�}� u
�    �  H�E�H� H��tH�E�H�PH�U�� ,H�E��@$��tH�E�H�PH�U�� 
H�E��  H�E�H�PH�E�H�H�E�H�PH�E�H� H�E�H�}� �����H�E��@$��tH�E�H�@H�P��   H�E�H��H������H�E�H�}� u�    �jH�E��@$��t0H�E�    �H�E�H�PH�U�� 	H�E�H�E�H�@H��H9E�r�H�E�H�PH�U�� }H�E��  H�E�H�@H�P�H�E�H�P�   ��UH��H�}�H�E�    H�E�    H�}� u�    �)H�E�H�@H�E��H�E�H�E�H� H�E�H�}� u�H�E�]�UH��H�}�H�u�H�E�    H�}� u�    �0H�E�H�@H�E��H�m�H�E�H� H�E�H�}� tH�}� u�H�E�]�UH��H��H�}��u�}� y�    ��E�Hc�H�E�H��H���{�����UH��H��0H�}�H�u��U�H�E�    H�}� tH�}� u
�    �   H�E�H�@H�E��}� tI�H�E�H� H�E�H�}� tWH�E�H�@8H��tJH�E�H�P8H�E�H��H���r�����u��-H�E�H� H�E�H�}� tH�E�H�P8H�E�H��H���������u�H�}� tH�E�H�@8H��u�    �H�E���UH��H��H�}�H�u�H�M�H�E��    H��H��������UH��H��H�}�H�u�H�M�H�E��   H��H���������UH��H��H�}�H�u�H�U�H�E�H��H������H��������UH��H�}�H�u�H�E�H�U�H�H�E�H�U�H�P�]�UH��H�� H�}�H�u�H�E�    H�}� u�    �tH�E�H�������H�E�H�}� u�    �VH�M�H�E��@   H��H���j���H�E�H�@8    H�E��@����H�E��PH�E�H�@    H�E�H�PH�E�H�H�E���UH��H�� H�}�H�u�H�E�    H�}� tH�}� t
H�E�H;E�u�    �qH�E�H�@H�E�H�}� u%H�E�H�U�H�PH�E�H�U�H�PH�E�H�     �4H�E�H�@H��t'H�E�H�@H�U�H��H������H�E�H�@H�U�H�P�   ��UH��H��H�}�H�u�H�U�H�E�H��H���3�����UH��H�}�H�E�]�UH��H��@H�}�H�u�H�U�H�M�D�E�H�E�    �E�    H�}� tH�}� tH�}� t
H�E�H;E�u
�    �   �}� tH�E�H������H�E�H�E؋@���E��2H�U�H�E�H��H�������H�E�H�}� u�    �hH�E؋@����E�H�E؋@%   ��u"H�E�H�@8H��tH�E�H�@H�U�H�R8H����H�E�H�U�H�P8H�E؋U�PH�U�H�E�H��H��������UH��H�� H�}�H�u�H�U�H�U�H�u�H�E�A�    � �` H���������UH��H�� H�}�H�u�H�U�H�U�H�u�H�E�A�   � �` H��������UH��H��H�}�H�u�H�}� u�    �#H�E� �` H�������H��H�E�H��H���p�����UH��H�� H�}�H�u�H�U�H�}� tH�}� u�    �/H�E� �` H������H��H�u�H�E�A�    � �` H���������UH��H�� H�}�H�u��S  H�E�H�U�H�u�H�E�A�    � �` H��������tH�E��H�E�H���m����    ��UH��H�� H�}�H�u��,  H�E�H�U�H�u�H�E�A�    � �` H���d�����tH�E��H�E�H�������    ��UH��H�� H�}�H�u��  H�E�H�U�H�u�H�E�A�    � �` H��������tH�E��H�E�H��������    ��UH��H��0H�}�H�u��U܋E܉���  H�E�H�U�H�u�H�E�A�    � �` H��������tH�E��H�E�H���f����    ��UH��H��0H�}�H�u��E��E��  H�E�H�U�H�u�H�E�A�    � �` H���S�����tH�E��H�E�H�������    ��UH��H��0H�}�H�u�H�U�H�E�H����  H�E�H�U�H�u�H�E�A�    � �` H���������tH�E��H�E�H�������    ��UH��H��0H�}�H�u�H�U�H�E�H����  H�E�H�U�H�u�H�E�A�    � �` H��������tH�E��H�E�H���G����    ��UH��H�� H�}�H�u��	  H�E�H�U�H�u�H�E�A�    � �` H���>�����tH�E��H�E�H��������    ��UH��H�� H�}�H�u��  H�E�H�U�H�u�H�E�A�    � �` H���������tH�E��H�E�H�������    ��UH��H�}�H�u�H�}� tH�}� u
�    �   H�E�H�@H9E�tH�E�H�@H�U�H�H�H�E�H� H��tH�E�H� H�U�H�RH�PH�E�H�@H9E�uH�E�H�H�E�H�P� H�E�H� H��uH�E�H�@H�U�H�RH�PH�E�H�@    H�E�H�     H�E�]�UH��H��H�}��u�}� y�    �'�E�Hc�H�E�H��H���c���H��H�E�H��H���������UH��H��H�}��u�U�H�E���H������H���j������UH��H�� H�}�H�u�H�U�H�E�H��H���Y���H�E�H�U�H�E�H��H��������UH��H�� H�}�H�u�H�U�H�E�H��H���G���H�E�H�U�H�E�H��H���[�����UH��H��H�}�H�u�H�U�H�E�H��H���e���H����������UH��H��H�}�H�u�H�U�H�E�H��H���s���H���������UH��H��(H�}�u�H�U�H�E�    �}� y
�    �   �E�Hc�H�E�H��H������H�E�H�}� uH�U�H�E�H��H�������WH�E�H�U�H�H�E�H�PH�E�H�PH�E�H�U�H�PH�E�H�@H9E�uH�E�H�U�H�P�H�E�H�@H�U�H��   ��UH��H�� H�}�H�u�H�U�H�}� tH�}� tH�}� u
�    ��   H�E�H;E�u
�   ��   H�E�H�H�E�H�H�E�H�PH�E�H�PH�E�H� H��tH�E�H� H�U�H�PH�E�H�@H9E�u3H�E�H�@H�PH�E�H�@H9�uH�E�H�U�H�PH�E�H�U�H�P�8H�E�H�@H��tH�E�H�@H�U�H�H�E�H� H��uH�E�H�@H�U�H�PH�E�H�     H�E�H�@    H�E�H�������   ��UH��H�� H�}��u�H�U�}� y�    �+�E�Hc�H�E�H��H���?���H��H�U�H�E�H��H��������UH��H�� H�}�H�u�H�U�M�H�}� tH�}� u
�    �   H�E�@%   ��uH�E�H�@8H��tH�E�H�@8H���1  H�E� �` H������H��H�E�H�P8H�E�@�����H�E�P�U�H�M�H�E�H��H������H��H�U�H�E�H��H���������UH��H�� H�}�H�u�H�U�H�U�H�u�H�E��    H��������UH��H�� H�}�H�u�H�U�H�U�H�u�H�E��   H���������UH��H��� �` � ���H�E�H�}� tH�E��@   H�E���UH��H��� �` �����H�E�H�}� tH�E��@   H�E���UH��H��� �` ����H�E�H�}� tH�E��@   H�E���UH��H�� �}� �` �s���H�E�H�}� t�}� t�   ��   H�E��PH�E���UH��H�� �E� �` �2���H�E�H�}� taH�E��@   H�E��E��@0�E�f/  rH�E��@(����,��  f/E�rH�E��@(   ���E��,�H�E��P(H�E���UH��H�� H�}� �` ����H�E�H�}� tGH�E��@   H�E� �` H���K���H��H�E�H�P H�E�H�@ H��uH�E�H�������    �H�E���UH��H�� H�}� �` �<���H�E�H�}� t"H�E��@  H�E�H������H��H�E�H�P H�E���UH��H�� H�}� �` �����H�E�H�}� t"H�E��@@  H�E�H���:���H��H�E�H�PH�E���UH��H�� H�}� �` ����H�E�H�}� t"H�E��@   H�E�H�������H��H�E�H�PH�E���UH��H�� H�}� �` �a���H�E�H�}� tGH�E��@�   H�E� �` H������H��H�E�H�P H�E�H�@ H��uH�E�H���W����    �H�E���UH��H��� �` �����H�E�H�}� tH�E��@    H�E���UH��H��� �` �����H�E�H�}� tH�E��@@   H�E���UH��H��0H�}؉u�H�E�    H�E�    H�E�    H�E�    �}� xH�}� u
�    ��   �Y���H�E�H�E�    �qH�E�H��    H�E�HЋ �*������H�E�H�}� uH�E�H���`����    �sH�}� uH�E�H�U�H�P�H�U�H�E�H��H�������H�E�H�E�H�E�H�}� t�E�H�H9E��y���H�}� tH�E�H�@H��tH�E�H�@H�U�H�PH�E���UH��H��0H�}؉u�H�E�    H�E�    H�E�    H�E�    �}� xH�}� u
�    ��   �O���H�E�H�E�    �sH�E�H��    H�E�H�� �Z������H�E�H�}� uH�E�H���T����    �sH�}� uH�E�H�U�H�P�H�U�H�E�H��H������H�E�H�E�H�E�H�}� t�E�H�H9E��w���H�}� tH�E�H�@H��tH�E�H�@H�U�H�PH�E���UH��H��0H�}؉u�H�E�    H�E�    H�E�    H�E�    �}� xH�}� u
�    ��   �C���H�E�H�E�    �oH�E�H��    H�E�H�� �����H�E�H�}� uH�E�H���L����    �sH�}� uH�E�H�U�H�P�H�U�H�E�H��H������H�E�H�E�H�E�H�}� t�E�H�H9E��{���H�}� tH�E�H�@H��tH�E�H�@H�U�H�PH�E���UH��H��0H�}؉u�H�E�    H�E�    H�E�    H�E�    �}� xH�}� u
�    ��   �;���H�E�H�E�    �qH�E�H��    H�E�H�H� H���X���H�E�H�}� uH�E�H���B����    �sH�}� uH�E�H�U�H�P�H�U�H�E�H��H������H�E�H�E�H�E�H�}� t�E�H�H9E��y���H�}� tH�E�H�@H��tH�E�H�@H�U�H�PH�E���UH��H��0H�}؉u�H�E�    H�E�    H�E�    H�E�    H�}� ��  � �` �A���H�E�H�}� �~  H�E؋@�����H�E��PH�E؋P(H�E��P(H�E��@0H�E��@0H�E�H�@ H��t1H�E�H�@ � �` H��襽��H��H�E�H�P H�E�H�@ H���  H�E�H�@8H��tHH�E؋@%   ��t
H�E�H�@8�H�E�H�@8� �` H���M���H�U�H�B8H�E�H�@8H����   �}� u	H�E���   H�E�H�@H�E��gH�E�   H������H�E�H�}� ��   H�}� t!H�E�H�U�H�H�E�H�U�H�PH�E�H�E��H�E�H�U�H�PH�E�H�E�H�E�H� H�E�H�}� u�H�}� tH�E�H�@H��tH�E�H�@H�U�H�PH�E��%��
�������H�}� tH�E�H���ؽ���    ��UH��H�}�H�E�H� H�PH�E�H��4H�E�H� � <
uH�E�H� H�PH�E�H�� H�E�H� H�PH�E�H�H�E�H� � ��u�]�UH��H�}�H�E�H� H�PH�E�H��FH�E�H� � <*u&H�E�H� H��� </uH�E�H� H�PH�E�H�� H�E�H� H�PH�E�H�H�E�H� � ��u�]�UH��H�}�H�u�H�E�H�H�E�H� ��H�E�H� H�PH�E�H�H�E�H� H�PH�E�H���   H�E�H�H�E�H� ��H�E�H� � <"u3H�E�H� � "H�E�H� H�PH�E�H�H�E�H� H�PH�E�H��   H�E�H� � <\uNH�E�H� H��� <"u<H�E�H� H�U�H�H���@�H�E�H� H�PH�E�H�H�E�H� H�PH�E�H�H�E�H� H�PH�E�H�H�E�H� H�PH�E�H�H�E�H� � ������]�UH��H��H�}�H�E�H�E�H�E�H����   �   H�E�� ����	��&wz��H���p@ ��H�E�H��H�E��   H�E�H��� </uH�E�H�������eH�E�H��� <*uH�E�H��������HH�E�H��H�E��:H�U�H�E�H��H��� ����%H�U�H�E���H�E�H��H�E�H�E�H��H�E�H�E�� ���@���H�E��  ����UH��H�}�H�}� u�    �H�E��@��������]�UH��H�}�H�}� u�    �H�E��@��������]�UH��H�}�H�}� u�    �H�E��@��������]�UH��H�}�H�}� u�    �H�E��@��������]�UH��H�}�H�}� u�    �H�E��@��������]�UH��H�}�H�}� u�    �H�E��@��������]�UH��H�}�H�}� u�    �H�E��@��������]�UH��H�}�H�}� u�    �H�E��@���� ����]�UH��H�}�H�}� u�    �H�E��@����@����]�UH��H�}�H�}� u�    �H�E��@��=�   ����]�UH��H��@H�}�H�uЉU�H�}� t.H�}� t'H�E؋PH�EЋ@1�����uH�E�H��������t
�    ��  H�E؋@����t<����~)��~.��t)��� t"�� ��t���@t=�   t
�    �  �H�E�H;E�u
�   �  H�E؋@����tS�����_  ��~7��t2�P  �� ��   �� 
��tS�8  ��@�  =�   t>�#  �   �  H�E��H0H�E��@0������t
�   ��  �    ��  H�E�H�@ H��tH�E�H�@ H��u
�    ��  H�E�H�P H�E�H�@ H��H��������u
�   �  �    �  H�E�H�@H�E�H�E�H�@H�E��:�U�H�M�H�E�H��H���6�����u
�    �Z  H�E�H� H�E�H�E�H� H�E�H�}� tH�}� u�H�E�H;E�t
�    �"  �   �  H�E�    H�E�    H�}� t
H�E�H�@��    H�E��^H�E�H�H8�U�H�E�H��H������H�E�H�}� u
�    �   �U�H�M�H�E�H��H���u�����u
�    �   H�E�H� H�E�H�}� u�H�}� t
H�E�H�@��    H�E��XH�E�H�H8�U�H�E�H��H������H�E�H�}� u�    �?�U�H�M�H�E�H��H���������u�    �H�E�H� H�E�H�}� u��   ��    ��UH��H��H�}�H��(  H�U�H������UH��H��H�}�H��(  H�U�H���А�Ð��AWI��AVI��AUA��ATL�%%  UH�-%  SL)�H������H��t1��    L��L��D��A��H��H9�u�H��[]A\A]A^A_Ðf.�     ���   ��H��H���                         gb2312 utf-8 data pointer = %p
 aaa = %d
 temp = %s
 count = %d
 data['count'] = %s
 r config_x.ini Open config file failed! 今开 昨收 涨幅(%) 当前 名称      %-15s|%-15s|%-15s|%-15s|%-15s|%-20s|%-20s|%-20s|%-20s|
 成交金额(万元) 成交数(手) 当日最低 当日最高 , https://stock.xueqiu.com/v5/stock/realtime/quotec.json?symbol= authority: stock.xueqiu.com method: GET scheme: https    accept-encoding: gzip, deflate, br      User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/99.0.4844.51 Safari/537.36  error_code data symbol current percent last_close open high low volume amount    %-13s|%-13.2f|%-13.2f|%-13.2f|%-13.2f|%-16.2f|%-16.2f|%-16d|%-14d|
             %i.%i.%i        �@     N @     �@      @     N @     �@     �@     �@     �@     �@     �@     �@     �@     �@     �@     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     �@     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     N @     �@     null %1.15g %lg %1.17g  �,@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �,@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �,@     �-@     �-@     �-@     �-@     �-@     �,@     �-@     �-@     �-@     �,@     �-@     �-@     �-@     �-@     �-@     �-@     �-@     �,@     �-@     �-@     �-@     �,@     �-@     �,@     �,@     u%04x ﻿ false true    �b@     �b@     Pc@     Pc@     �b@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     �b@     Pc@     ;c@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     Pc@     �b@                  �  �����A      ���������              �<�������;�  �   ����   �����  ����  ����H  ?���h  �����  ˝���  k����  |����  ����  ����(  ����H  ����h  �����  �����  �����  �����  ����   ��(  ɠ��H  Р��h  נ���  ޠ���  ����  ����  ���  ����(  ���H  ���h  ����  ����  ����  $����  +���	  2���(	  9���H	  @���h	  G����	  N����	  '����	  F����	  ]���
  ����0
  ����P
  ���p
  �����
  ����
  ̫���
  ����
  ���  ���4  ����T  *���t  ����   ����  �����  ����  ���  #���<  B���\  p���|  1����  Z����  ����  �����  ����  ����<  ���\  4���|  �����  �����  ����  
����  ����  ����<  ����\  ����|  X����  �����  y����  �����  ���  Q���<  "���\  L���|  v����  �����  �����  f����  ���  5���<  C���\  K���|  �����  �����  �����  R����  ����  ����<  Q���\  ����|  ����  m����  �����  "����  w���  6���<  {���\  ����|  �����  ����  L����  z����  3���  V���<  ����\  \���|  �����  �����  �����  ����  D���  ����<  ���\  z���|  �����  ����  U����  �����  ����  ���<  )���\  5���|  =����  G����  <����  �����  ���  >���<  0���\  Z���|  �����  �����  �����  ����  0���  [���<  ����\  ����|  �����  ����  0����  P���   ����H             zR x�        ����/    D   0   ̖��       $   D   �����   FJw� ?;*3$"       l   F����    A�C�     �   ϗ��G    A�CB     �   ����E    A�C@     �   ����   A�C�    �   ����   A�C      ����    A�CB      ,  s���    A�CB      L  Z���    A�CB      l  A���    A�CB      �  (���    A�CB      �  ���    A�CB      �  ����    A�CB      �  ݙ��    A�CB        ę��    A�CB      ,  ����    A�CB      L  ����    A�CB      l  y���    A�CB      �  `���    A�CB      �  G���    A�CB      �  .���    A�CB      �  ���    A�CB        ����    A�CB      ,  ���    A�CB      L  ʘ��    A�CB      l  ����    A�CB      �  ����    A�CB      �  ���    A�CB      �  f���    A�CB      �  M���    A�CB        4���    A�CB      ,  ���    A�CB      L  ���    A�CB      l  ���    A�CB      �  З��    A�CB      �  ����    A�CB       �  �����    A�CE��       �  S���   A�CH�       N���    A�CR      4  E���-    A�Ch      T  R���1    A�Cl      t  c���/    A�Cj       �  r����    A�CE��      �  �����    A�C|     �  Y����    A�C�     �  ���A    A�C|        ����    A�C�     8  ʡ��    A�CF      X  �����   A�C�    x  `���n    A�Ci      �  �����    A�CE��      �  g���   A�C     �  `���h    A�CE�^         �����    A�C�        ����   A�C�    @  ���   A�C    `  ߩ��   A�C    �  ޫ��.   A�C)    �  ����   A�C�    �  ����)    A�Cd      �  �����    A�C�        5����    A�C�        ����N    A�CI     @  ز���   A�C�    `  ����$    A�C_      �  ����,    A�Cg      �  �����   A�C�    �  #���$    A�C_      �  '���$    A�C_       	  +����    A�C�      	  ����    A�C�     @	  �����   A�C�    `	  ~���-   A�C(    �	  �����   A�C�    �	  ����   A�C�    �	  ����o   A�Cj    �	  ����   A�C�     
  ����Q    A�CL      
  ����T    A�CO     @
  ����3    A�Cn      `
  ����    A�C�     �
  ����*    A�Ce      �
  ����*    A�Ce      �
  ����.    A�Ci      �
  ����&    A�Ca         �����    A�C�        b����    A�C�     @  ����%    A�C`      `  ����    A�CI      �  ����   A�C    �  ����5    A�Cp      �  ����5    A�Cp      �  ����C    A�C~         ���Z    A�CU        N���U    A�CP     @  ����U    A�CP     `  ����U    A�CP     �  ����]    A�CX     �  *���_    A�CZ     �  i���`    A�C[     �  ����`    A�C[        ����U    A�CP        ���U    A�CP     @  S����    A�C�     `  ����E    A�C@     �  ���+    A�Cf      �  "���<    A�Cw      �  >���<    A�Cw      �  Z���.    A�Ci         h���.    A�Ci         v����    A�C�     @  ���#   A�C    `  ���M    A�CH     �  ?����    A�C�     �  ����/    A�Cj      �  ����/    A�Cj      �  ����.    A�Ci         ���.    A�Ci         ���.    A�Ci      @   ���?    A�Cz      `  ?����    A�C�     �  ����n    A�Ci     �  ����I    A�CD     �  ���I    A�CD     �  H���I    A�CD        q���n    A�Ci        ����.    A�Ci      @  ����.    A�Ci      `  ����
   A�C    �  ����   A�C    �  ����   A�C    �  ����
   A�C    �  �����   A�C�       X���`    A�C[        ����r    A�Cm     @  ����0   A�C+    `  �����    A�C�     �  ����*    A�Ce      �  ����+    A�Cf      �  ����+    A�Cf      �  ����*    A�Ce         ����+    A�Cf         ���+    A�Cf      @  ���+    A�Cf      `  ���+    A�Cf      �  "���+    A�Cf      �  -���-    A�Ch      �  :���4   A�C/    �  N���    A�CY          L���    A�CZ       D   $  H���e    F�E�E �E(�H0�H8�G@n8A0A(B BBB    l  p���                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           �@     �@                          �              �               @            (i@            ��`                          �`                   ���o    �@            h@            �@     
       �                                           �`            �                           0	@            �@            `       	              ���o    �@     ���o           ���o    D@                                                                                                                                     ��`                     6@     F@     V@     f@     v@     �@     �@     �@     �@     �@     �@     �@     �@     @     @     &@     6@     F@     V@     f@     v@     �@     �@     �@     �@     �@     �@     �@     �@     @                             �@     p@      @     GCC: (GNU) 8.3.1 20190507 (Red Hat 8.3.1-4) 
         GA$3h878   @     @     
         GA$3c878   @     @     
         GA$3s878   @     @     
         GA$3e878   @     @     
         GA$3h878   @     @     
         GA$3c878   @     @     
         GA$3s878   @     @     
         GA$3e878   @     @              GA$3a1 @     ?@              GA$3a1 ?@     ?@     
         GA$3p878   ?@     ?@               GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*              GA+stack_clash            GA*cf_protection             GA*FORTIFY �              GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign              GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*              GA+stack_clash            GA*cf_protection             GA*FORTIFY �              GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign              GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*              GA+stack_clash            GA*cf_protection             GA*FORTIFY �              GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign              GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*              GA+stack_clash            GA*cf_protection             GA*FORTIFY �              GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign              GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*              GA+stack_clash            GA*cf_protection             GA*FORTIFY �              GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign    
         GA$3p878   @@     E@               GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*              GA+stack_clash            GA*cf_protection             GA*FORTIFY �              GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign             GA*FORTIFY �             GA+GLIBCXX_ASSERTIONS             GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*              GA+stack_clash            GA*cf_protection             GA*FORTIFY �              GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign              GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*              GA+stack_clash            GA*cf_protection             GA*FORTIFY �              GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign              GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*              GA+stack_clash            GA*cf_protection             GA*FORTIFY �              GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign              GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*              GA+stack_clash            GA*cf_protection             GA*FORTIFY �              GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign             GA$3a1 E@     E@              GA$3a1  @     @              GA$3a1 (i@     0i@              GA$3a1 P@     �@     
         GA$3p878   �h@     %i@               GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*             GA+stack_clash            GA*cf_protection             GA*FORTIFY               GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign             GA*FORTIFY              GA+GLIBCXX_ASSERTIONS            GA*FORTIFY              GA+GLIBCXX_ASSERTIONS   
         GA$3h878   @     @               GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*             GA+stack_clash            GA*cf_protection             GA*FORTIFY               GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign    
         GA$3c878   @     @               GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*             GA+stack_clash            GA*cf_protection             GA*FORTIFY               GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign    
         GA$3s878   @     @               GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*             GA+stack_clash            GA*cf_protection             GA*FORTIFY               GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign    
         GA$3e878   @     @               GA$gcc 8.3.1 20190507  
          GA*GOW *E             GA*             GA+stack_clash            GA*cf_protection             GA*FORTIFY               GA+GLIBCXX_ASSERTIONS             GA*             GA!              GA+omit_frame_pointer             GA*               GA!stack_realign             GA$3a1 %i@     %i@              GA$3a1 %i@     %i@              GA$3a1 @     @              GA$3a1 0i@     5i@     ,             �@     �                      ,    )       �@     	M                      %       �  �  �  �@     �          �  �9   �   �   �	  �  �	  �&  int R$  �  �l   |  �l   �   �	  �   �  �1$  	*  3e    	�"  6	�   	o  7	�   	�  8	�   	  9	�    	�  :	�   (	I  ;	�   0	$  <	�   8	�  =	�   @	�  @	�   H	A"  A	�   P	o  B	�   X	�  D=  `	L	  FC  h	#   He   p	�$  Ie   t	�#  Js   x	�   MP   �	I'  NW   �	t  OI  �	  QY  �	�#  Y   �	�  [d  �	#  \o  �	#  ]C  �	�   ^	G   �	U  _
-   �	�  `e   �	   bu  � ?  �   

  +�  8  �   �   Y  9     0  }  _     j  �   �  9      ��  $  �  ��  V  ��  %  	e   �  �   �  �   �  �  	�  M$  �   �  �  
  �   D  $�   $  2e   V  7e     ;e     e   r  .G   �   b  9    �  �R  8  �e     �l   �  �R  :  �e   �  �l   �  4�  	�'  6	e    	R  7	e    �  �  
�  fZ  G	  N&  H		�    M  I	   �  �  �  @   �m  B   �  5$  �  �    �  '  �  R  	�  
�&  �  �  �  �'  :      �  A    �    ]  �  �  �  �"  n  �!  �     Y  !$  "'  #N  $  %-'  &�  'x  (�&  )
  *�  +�  ,K  -�  .x   /�  0  1  2�'  3d  4)  5�
  6�  73  8�  9�  :/  ;�  <�  =	  >]  ?   @�  A1  B�  C�  D�!  E)  Fb  G$  H�  I�#  J�  Kr$  L�  M�&  Nq  OT  P�  Q.  R�%  S�$  T  Uh  V�"  WI#  X  Y$  ZQ  [�  \D  ].  ^ w   I#  @   �A  0  'S  '�  �
  '�  '  'l  '6  '  '9  +NL   ,NF      '#   '�!  !'�  "'�  �  �
  �  &'b  ''�  ('1  )']  *'
  |&  ,'|  -'2  /'�     !�  "\  4'�  5'�  7'g  8'U  )  *:  +�   ,�  -\#  .�  /�  05  2/  3i  4  5�!  6�  XN�  I'
  :�  ;{  <�
  =m  N'�  O'�  @�  Q'�  D�   E�  V'�  G�  H�  J  K�&  \'  ]'[$  N{  oN�  P�  Q�  b'�%  c'�  T  Ux  f'a  g'�"  h'�  i'�  Z
  [�  \Q  m'"  ~N@  o'�
  `|  q'v'  bS&  cM%  t'  e\  v'�  w'd  x'�%  i�  j  kO  �N�  }'�  n�!  oD  p�  q�  r�	  �u  �up  �uC  �'_  wz  �u�  ye  �M  �N�   �'8   �'�  �'n  ��  �H  ��  �  �A  ��  �N�  �N  �N�&  �u  �uQ  �'%  �N@  �'�  ��  �<
  �'0	  �'N!  �.  ��#  ��  �V  �1  �!  ��  �z  �'�  �N�  �'&  �'z  �%  �N�  �'�  �'[%  �'k#  �t  ��'  �'�	  �'j	  �'$  �'�
  �'u  �!  �'{!  �  �  ��  �'�  �N�  �'�  �'   �'�  �>   �d&  �'  �'a   �'W
  ��  �   �'�$  �N}  ��%  �N}  �N_  �Nj  �'�  �'�  �'r  �'�  �'<  �'p
  ��'  �N�$  �'�  ��#  �'4%  ��  �$  �=  �a  ��  �'�  �&  �N�  �'�  �'�  �'�  �'8  �'S	  ��  ��  �=  �'k  ��  �'�  �'0  ��  �;!  ��  �'u  �'�  �c  �'m  �2#   (b!  (�#  �_  (�!  ��  ��  (�  (�	  �]  �<  ��  (
  (�   (�  (M  (�  (�  (�  (%  (C  (�#  R  (�  (�  (�  	h  (#  v  �  (�  >v�  �  0O�  !(�  #"  �   $(�  %(�   n%   @   n{  	   �	  �	  I    S   �'   @   ��  �     8  N  Q"  i"    �"  G   G$  @g7  	M  j7   	D'  k7  	�(  m7  	�,  p	e   	u  s�    	�  u	e   (	%  w  0	,  z�   8 �  G$  {�  =  res �   	p�`     �%  e   �@           ��  �  e   ��}�    ��}fp �  �H�  $�  ��}2  %	�   �X�$  (�   �@)  =	  ��{   >m  ��~url ?	�   ��.  @	�   ���  De   ��e  I  ��  `�  ��~N&  a�  ��~4  b�  �P)  c�  ��~�  d�  ��~@     �       �  len ,e   ���@     �       �$  2	e   ��  �@     !       �  W  Qe   �� �@     "       %  W  Re   �� �@     "       K  W  Se   ��  @     "       q  W  Te   �� "@     "       �  W  Ue   ��~ D@     "       �  W  Ve   ��~ f@     !       �  W  We   ��~ �@     "       	  W  Xe   ��~ �@     (       /  W  Ye   ��~ �@     {      �  k�  ��~    l�  ��~�  m�  ��~�  n�  ��~�  o�  ��~  p�  ��~low q�  ��~�  r�  ��~�  s�  ��~  �   �  9   	 =  I    
-   �@     �       �M  ptr 
�   �XK1  
%-   �PK  
2-   �H�  
?G   �@  �   ��@            � 2  ��@            � �  ��@            � �  ��@            � C  ��@            � �  ��@            � �%  �}@            � �  �v@            � �  �o@            � )  �h@            � E  �a@            � �  �Z@            � �  �S@            � �  �L@            � 
  �E@            � 5&  �>@            � X'  �7@            �   �0@            � �  �)@            �    �"@            � J  �@            � �   �@            � �"  �@            � �  �@            � �$  ��@            � u  ��@            � �!  ��@            � �  ��@            � ,  ��@            � �  ��@            �!~  <�@           �  str <�   �H�  <�   �DN&  <0  ��len =e   �`i ?e   �l�$  @e   �hC  Ae   �d@            �  H�   �Xp P�   �P  !  !+@     �      ��  str !�   �H�  ! �   �DN&  !1  ��len "e   �`i #e   �l�$  $e   �hC  %e   �d�@     �       �  *
�   �X  "g2u e   �@     E       �  �  �   �h%  -   �`  (�   �X  6-   �P "u2g e   �@     G       �t  �  �   �h%  e   �d  %�   �X  0e   �` #
  
e   �@     �       �+   
�   �H  
,�   �@�  
>�   ��%  
L-   ��  
Y�   ��  
h-   ��cd F  �X$rc 	e   pin   �h�"    �`  x+   (  �  �*  �  �@     	M      �  �  �>   -   �   �  Y   int R   p   ^   �	  i   �   �	  ~   �	  �&  R$  �  ��   |  ��   i   �  �1D  	*  3R    	�"  6	�   	o  7	�   	�  8	�   	  9	�    	�  :	�   (	I  ;	�   0	$  <	�   8	�  =	�   @	�  @	�   H	A"  A	�   P	o  B	�   X	�  D]  `	L	  Fc  h	#   HR   p	�$  IR   t	�#  J�   x	�   ME   �	I'  N�   �	t  Oi  �	  Qy  �	�#  Y�   �	�  [�  �	#  \�  �	#  ]c  �	�   ^	|   �	U  _
-   �	�  `R   �	   b�  � ?  �   

  +�  X  �   i   y  >     P  }       �  i   �  >      ��  D  �  ��  V  ��  %  R   d   �   �  �  �  s*  �  �    �(  R   M$  �   0  �  G$  @	g�  	M  	j�   	D'  	k�  	�(  	m�  	�,  	p	R   	u  	s�    	�  	u	R   (	%  	w  0	,  	z�   8 8  G$  	{8  �  m3  	}�  	6(  	��   	F3  	�   |   �  -    �    |      m3  	��  �*  	�R     X	T  	Q0  YT   	�+  Z-    �   T  A*  [0  :*  \_  	P�`     �-  ��  	,  ��   	,  �  	�/  ��   |   �  |   -    �  �-  ��  �  4  ��  	 �`     8	H  2  !T   �1  "-   �#  #-   �-  $-   !4  %�    �3  &�  @�	�  �3  ��   �1  �-   �#  �-   �-  �-   0  �   �(  �  $!4  ��  ( ~   h1  �U  �(  #�h@            �  <+  #%|   �h .  |   rh@            �9  K1  *-   �h �1  �  >e@     4      ��  a �<�  �Hb �S�  �@',  �g+  ���f@     �       �  1)  ��  �h-  ��  �`  Xg@           1)  ��  �X-  ��  �P  �  �  �  �  !�0  �  e@     -       �<  4  �:�  �h !�.  �  �d@     +       �p  4  �=�  �h !�/  �  �d@     +       ��  4  �<�  �h !^/  y  �d@     +       ��  4  y=�  �h !�+  o  ed@     +       �  4  o=�  �h !�-  e  :d@     +       �@  4  e;�  �h !�,  \  d@     *       �t  4  \;�  �h !,  Q  �c@     +       ��  4  Q;�  �h !3  G  �c@     +       ��  4  G<�  �h !t)  =  �c@     *       �	  4  =>�  �h �0  �b@     �       �P	  Q0  '�   �Xz-  �   �h "�3  �
na@     0      ��	  Y(  �
"�	  �h�+  �
0�	  �` �   "6,  �
�`@     r       ��	  Y(  �
+�	  �h "!2  �
�`@     `       ��	  Y(  �
)�	  �h ;)  �
�  �^@     �      ��
  4  �
4�  �HQ(  �
E  �D�0  �
�  �h�(  �
�  �`M  �
�  �X�(  �
�  �P#�/  �
�`@      P1  `
�  �]@     
      �  )  `
B  �HC  `
OR   �D$i b
-   �h$n c
�  �`$p d
�  �X$a e
�  �P d   �*  8
�  �\@           ��  &*  8
=�  �HC  8
JR   �D$i :
-   �h$n ;
�  �`$p <
�  �X$a =
�  �P 
  ]4  
�  �[@           �  &*  
;  �HC  
HR   �D$i 
-   �h$n 
�  �`$p 
�  �X$a 
�  �P �  _(  �	�  Z@     
      ��  &*  �	7L   �HC  �	DR   �D$i �	-   �h$n �	�  �`$p �	�  �X$a �	�  �P �2  �	�  QZ@     .       ��  4  �	�  �h �1  �	�  #Z@     .       ��  4  �	�  �h �(  �	�  �Y@     n       �9  raw �	3^   �X4  �	�  �h �)  �	�  lY@     I       �}  �(  �	?�  �X4  �	�  �h K)  �	�  #Y@     I       ��  �(  �	@�  �X4  �	�  �h �3  �	�  �X@     I       �  ,  �	?^   �X4  �	�  �h A4  �	�  lX@     n       �I  ,  �	6^   �X4  �	�  �h �,  t	�  �W@     �       ��  num t	1  �X4  v	�  �h �.  i	�  �W@     ?       ��  h-  i	3  �\4  k	�  �h �(  ^	�  vW@     .       �  4  `	�  �h �-  S	�  HW@     .       �9  4  U	�  �h �/  H	�  W@     .       �m  4  J	�  �h �)  B	  �V@     /       ��  <+  B	H�  �h,  B	\^   �`�0  B	k�  �X '4  =	  �V@     /       �  <+  =	;�  �h,  =	O^   �`�0  =	^�  �X %w.  +	  V@     �       �y  <+  +	1�  �h,  +	E^   �`L-  +	T�  �X',  +	l  �T �)  !	  �U@     M       ��  -  !	:�  �hS/  !	ER   �d�0  !	S�  �X �,  �  �T@     #      �!  �/  �D  �h4  �Z  �`L-  �h�  �X �0  �  �S@     �       ��  -  �9�  �XS/  �DR   �T�0  �R�  �HK2  ��  �h �3  ��S@     .       ��  <+  �C�  �h,  �W^   �` �.  �~S@     .       �  <+  �6�  �h,  �J^   �` {0  ��  BS@     <       �Y  <+  �F�  �X,  �Z^   �P3  ��  �h !0  ��  S@     <       ��  <+  �9�  �X,  �M^   �P3  ��  �h 1.  ��R@     +       ��  -  �5�  �hS/  �@R   �d K.  ��  �R@     E       �1  -  �8�  �hS/  �CR   �d !�2  ��  �Q@     �       �u  �/  �9�  �h4  �O  �` G*  {�  �Q@     U       ��  <+  {;  �X�  {Vd   �P-  }�  �h V0  o�  -Q@     U       �  <+  o<  �X�  oWd   �P\-  q�  �h �1  c�  �P@     `       ��  <+  c9  �X�  cTd   �Praw cmd   �H�*  e�  �h .  W�  mP@     `       ��  <+  W<  �X�  WWd   �P,  Wpd   �H*  Y�  �h #/  K�  P@     _       �I  <+  K<  �X�  KWd   �P�/  Kj
  �H�2  M�  �h 2  ?�  �O@     ]       ��  <+  ?:  �X�  ?Ud   �Ph-  ?l+  �L�2  A�  �h +  3�  \O@     U       �  <+  3;  �X�  3Vd   �P4  5�  �h t(  '�  O@     U       �U  <+  ':  �X�  'Ud   �P�+  )�  �h _+  �  �N@     U       ��  <+  :  �X�  Ud   �PY/  �  �h --    XN@     Z       ��  <+  @�  �h,  T^   �`4  c�  �X �3    N@     C       �A  -  ?�  �h4  M�  �` m/    �M@     5       ��  <+  9�  �h,  M^   �`4  \�  �X M,  �  �M@     5       ��  <+  �7�  �h,  �K^   �`4  �Z�  �X %�.  �  �L@           �~  <+  �4  �X,  �Od   �P4  �e  �H!4  ���  �@g)  ��+  ��0  ��   �h�,  �	R   �d �  ~  &A0  �|   �L@            ��  ,  �**  �h -  �  pL@     %       �  -  �6�  �h4  �D�  �` %e.  �  �K@     �       �U  -  �,�  �X4  �:�  �P�(  ��  �h %�2  ��  *K@     �       ��  4  �-�  �X!4  �P�  �P�2  ��  �h "C+  �K@     &       ��  D'  �"�  �h4  �/�  �` +  ~  �J@     .       �-  <+  ~;�  �h,  ~O^   �` �,  y�  �J@     *       �q  <+  yL�  �h,  ygd   �` (  t�  �J@     *       ��  <+  t?�  �h,  tZd   �` %X-  T�  �I@     �       �  <+  T3�  �X�  TNd   �P',  Te+  �L�.  V�  �h y,  J�  ~I@     3       �]  -  J7�  �h�(  JBR   �d &�+  7�  *I@     T       ��  -  7+�  �X�(  79-   �P�(  9�  �h !�-   R   �H@     Q       �  -   3�  �X�(  "�  �hK1  #-   �` %�1  �  'E@     �      ��  4  �4�  ���2  �N�  ��N3  ��  �h�1  �-   �Ht1  ��  �`F@     s       �  $i �-   �X  xH@     0       $i -   �P  �  �  %6+  =  �A@     o      �f  4  =.  �Hn0  =Il  �@e  ?�  �ht1  @�  �`#�/  �E@     #�0  ��D@     '�   T4  a�  �X  H  f  %�1  �  �?@     �      ��  4  �3�  �H�2  �M�  �@N3  �  �h�1  -   �X�.  �  �` %-  �  O=@     �      ��  4  �-  �Hn0  �Hl  �@e  ��  �ht1  ��  �`#�/  ��?@     #�0  �o?@     '�   T4  ��  �X  %�)  S  ";@     -      ��  4  S3�  �X�2  SM�  �P�+  U�  �h �<@     ~       /  ~-   �`  %.*    08@     �      �;  4  -  �hn0  Hl  �` $1    j7@     �       ��  4  9�  ���3  E�   ���1  WY   ���(  p+  ��$p 
�  �� #3  ��   o6@     �       �   4  �7�  ��p-  �AR   ��fmt �W  ��$p ��  �� �+  ��   K6@     $       �L   4  �:�  �h 0  ��   '6@     $       ��   4  �/�  �h %�/  ��  �4@     �      � !  4  �1�  ���(  �B  ��!4  �g�  ��<1  �9   	 r@     �3  � !  ���-  ��  �h#�/  ��5@      �  0!  >     �+  ��  h4@     ,       �t!  4*  �9^   �hy3  �G-   �` 1  ��  D4@     $       ��!  4*  �/^   �h �*  C�  \2@     �      �e"  4*  C=^   ��y3  CK-   ��@(  Cge"  �� 1  C�  ���3  EH  ��4  F�  �h#�/  r�3@      �3@     k       -  z_  ��  ^   �-  3�  2@     N       ��"  4*  37^   �X@(  3Ke"  �P 1  3h  �Ly3  5-   �h %�1  $f  y1@     �       �#  �3  $9l  �h &�2  
f  �0@     �       �7#  �3  
Bl  �h %$)  �  �0@     )       �y#  4  �4�  �hp �N�  �` %�(  �  �-@     �      �$  Y(  �@Z  ���2  �[�  ���0  �T  �h�+  ��  �PN3  ��  �`62  �-   �H )  �-   �X %�*    �*@     .      ��$  4  .  ��n0  Il  ���0  T  �h�)  T  �`N3  �  ���+  �  �X#�/  s�-@     (0   �$  �*  -   �@�.  -   �P '`   �-  :~   �O  %c,  �~   �(@           ��%  �0  �HZ  ���)  �sZ  ��N3  ���%  ��/  �>   �h�3  �u   �`�/  �T  �X�1  �~   �g�+  �~   �f�-  �~   �e]3  �~   �d#�/  ��*@     '    �,  �T  �P�(  �u   �L  �  &�)  cu   o'@           �;&  Y(  c8Z  �X$h eu   �l$i f-   �` %�.  !  z%@     �      ��&  4  !4�  ���2  !N�  ��N3  #�  �X$d $  �P�1  %	R   �l$i &-   �`Q+  '�&  ��k2  (~   �O�1  )  �� ~   �&  >    &'(    �$@     �       �O'  a )  �Xb 3  �P�-    �h )�0  �$@     h       ��'  �3  /�  �H73  T  �X %/  ��  g"@           ��'  p �2�  �XD2  �<-   �P�*  ��  �h.  �-   �` �1  ��   �!@     �       �E(  <+  �1�  �Hu  �E^   �@�)  ��   �X !�/  ~  !@     n       ��(  <+  ~3�  �h�/  ~B  �` %�/  1  Q@     �      �/)  4  1.  ��n0  1Il  ���/  3  �`)  4�  �P�+  5/)  ��k2  6~   �_$i 7-   �h#3  ^R @      ~   ?)  >   ? *g2  ~   F@            �+Z2  �m@     �       ��)  ,4  �(�  �XM  ��  �h -y2  ��  ,@     A       ��)  ,!4  �;�  �X<0  ��  �h .�+  �u@     �       �
*  ,!4  �1
*  �h   -y*  ��  �@     �       �o*  ,,  �9T  �X,!4  �^�  �P�1  �-   �h�)  ��  �` -;/  �R   J@     �       ��*  ,u+  �9T  �X,}+  �WT  �P /�*  |^   @     /       ��*    ~�*  	`�`      i   �*  >    /^*  m  �@     1       �++  ,4  m?�  �h /�)  c�   �@     -       �]+  ,4  c?�  �h 0�/  ^^   �@            � %   :;9I  $ >     $ >   I  & I  :;9  	 :;9I8  
 :;9   <  I  ! I/  4 :;9I?<  !   4 :;9I?<  7 I  :;9   :;9I8  >I:;9  (    :;9I  (   4 :;9I?  .?:;9'I@�B   :;9I  4 :;9I  4 :;9I       :;9I   . :;9'@�B  !.?:;9'@�B  ".?:;9'I@�B  #.?:;9'I@�B  $4 :;9I   %   :;9I  & I  $ >   I  $ >     :;9  	 :;9I8  
 :;9   <  I  ! I/  4 :;9I?<  !   4 :;9I?<  &   'I   I  '  :;9  4 :;9I  :;9   :;9I8   :;9I  .?:;9'@�B   :;9I  .?:;9'I@�B   :;9I    4 :;9I     !.?:;9'I@�B  ".:;9'@�B  #
 :;9  $4 :;9I  %.:;9'I@�B  &.:;9'I@�B  'U  (U  ).:;9'@�B  *. :;9'I@�B  +.?:;9'@�B  , :;9I  -.:;9'I@�B  ..?:;9'@�B  /.?:;9'I@�B  0. ?:;9'I@�B   �   t  �      /usr/include/curl /usr/lib/gcc/x86_64-redhat-linux/8/include /usr/include/bits /usr/include/bits/types /usr/include /usr/local/include /usr/include/sys  util.c    typecheck-gcc.h   stock_x.c    stddef.h   types.h   struct_FILE.h   FILE.h   stdio.h   sys_errlist.h   unistd.h   getopt_core.h   iconv.h   time.h   time.h   curl.h   cJSON.h     p 	�@     	�
�g tu	K�7 fu�Y80Y/>1u+90K�>uvYtY�<�,gf$�f�t�Y<u&=<'K<K	� tJ J��f<t>8>K�?uxt
Y�Y�,>7f<$�/f<�t��<u	� qJ J��f<t>� <J>J>J@J>J@J>J>J>J@J>J>J>J>J>J?J?J>J>J?J?J>J?J>J>J>J>J?J?J>JI�~<�t1Y<�J
 u4vB @�N<J 	u%�"v��	u�/u�B���Y�t��>@uf"Yf�	�C=fYu�C=I�<"Yf
r����	��'�6�J�"�4=6f4�H<J/"��"""""�"	(!��Y����	��' � ��	� � � X � u u! u u u u u u� v �� J �	 uz �����	�Y y   �   �      /usr/lib/gcc/x86_64-redhat-linux/8/include /usr/include/bits /usr/include/bits/types /usr/include  cJSON.c    stddef.h   types.h   struct_FILE.h   FILE.h   stdio.h   sys_errlist.h   math.h   cJSON.h      	�@     � 'K;tt=1
��Lw�1
��L��6L$Y2 tvw�ytL3pKX X t( �  t �Rt(� t�K#t���vw�!�t�vvhK1�w!��	�[�t&Ztw��(Z!�x�t; �+ t!��2 �t�	v[K2��Z�tX7 J/ �Z��tX7 J/ �Z)t�ZtX9 J1 �Z)t�Z	t�p��@OZ.��#���0 t  tZ��0/�	�&<D�$��$1_ i  X5 t8 t�+?!��J��������w�0��� Y2������ wg1�t<2 J9 t( XL	�-� ��	Zg�Kvv�Z	���Kt��� t tZ�� X+ � �[������ht�
�tK�������-[:�%t��"�tY��-�%��v"�tY��'w	�J	t��	tZ���u2��$ t t\-t�v��	�%x�1 f .  Y �# �  �2��	�u�#���v� � "�3��C X? �M� fh,�Xgv��0�Z�=1*��( x. X X
f�>�X� Y2�u��[�( J �&L�<��- J$ �+L��- J$ �+L�ww iJ X�=3=�u�KKKL��1! ��  ���uL��t= �* ��0$ �6�L�=�.Xdf .Kzwh�Kh�Kh�K	J�O 1
 � t, t* X /\ T �m h
f t< N
f t<1L	�f	<<B	<	*< Y2*�I�J&uE�Ju�	�'�f���1t(L:�,tfT<��YZ2sX$t�K<�Y XU t/J!t�I<�W �S t3�1 t5�"���Zt/L�!�<yK�"��$w'�=$0'�=$0'�=$0'�=$0'�=$2'��'2�k%M�8�J>�>�0t�<���	�t<	<D<%<7 v	��Ev7t �<� Y2������v�vKv	����0	�"
�Y0t'MZ> l ! X t,J�<��v�vu	�/#�=#�>�u��Zt' J# tC J? tMt��=�%,u%0u%0u%0u%0u%0u%0u=1t$�Y9 Z ^ X! X t*��=�>Y21���.�$ t tZ�	 tLk�� t/ JG �+ fO!��Z��K2�$ t tA X7 �Zw	 tA �,  (  L?K11vx��g2g8��� �������
�&Q	h�%JJ1 XJ �. fO*v�w	^�<B	< 	v�w��#JJ*Z"JhJ*Z2J"JM9vLJ?J<w[ Y2�Y1��31�,['t�K�g�J
�.���)ZL�$J<J$J�w)�:t$J�)x	�t	J�J!t?!�	J[	_fB	<	< JZ!�	J[v	�� Y2�Y1�Y1!@h,� t�KJZwXK�ug
�.L"t	JYw K1Y@ fvw�XK�ug� /2�0 t  tZ�	 t< �'  #  L�=�	 t< �'  #  L�=�	 t< �'  #  L��=�	 t2 JP �. fLw	 t3 JQ �. fb J� �] f� J� �� fLw	 t2 JP �. fLM	 t2 JP �. fLMY2�� tv�t<[Kv���Kv���Kv�סx��Z%����gv v�/xMMMZ2������	�'�f����	 t2 JP �. f�	 tL�	�\�*����!w� �����	��.	�� t5 JS �1 f�	 �4 JR �1 fO	L.5 ��u�����	�t<<	D 	v�Y2����v�Kv�u����Z.L	��t-�t J t X K �v��=t L#�>#uhi��Kvv�=u��Y2�������	 �5 �S �1 f����	 t2 JP �. f�	 tL�	�\�*����!w� �����	��.	�,��#�� �9 �W �5 f��	��.	�� t5 JS �1 f�	 �5 �S �1 fO	�.�  ��u�����	�t<<
<D	D 	v�Y2����v%�t J t X K �v��=��tL�>hZtM�v��  0# �4 :* X �#j3�#�;���L	�)�t J t X K gv��=tL�>i.L	�)�t J tR X= t X t X u �v�tZ�?tL�>uhA�9� �tR J � f X /vvtM	� 0 �6 :' X/ �	 Jk�=u��Y2���vw
�0Y
�|K1��vw�0Y�$ tzK1�hwK1/� tv��h0�= t* �o XS �O �	f�� t. �* �P6 t# �YwK1�u1�u1�4.�2���@��vw�vwu�'/ ��uK1�� t+ t�w�w���� Z�� ��Y2�/
.�K5�	�v t. t@ t��h�t�gvwtitX5 J- �Z�	�[��/1=�2=�1�vw#1= tvw/1�	��Li�Y1�	��Li�Y1�	��Li�Y1/	��Li�Y1K	��Li�Y1=	��Li�Y1=	��Li�Y1�	��Li�Y1�	��Li�Y1� tv��i �t>t[ t�M�it�t[#��N��K1�hw'1��?�h/1�h/1��?��@/�h��vM#��� ��h��vY1= t3 tv���tu��tZ!t��h�*J�Z���Z%�vtZ!�����Y1/hw+1g tv�tXC J4 �Z	�"�<�0,1=�1=�2��v�K1��v�K1��v�K1��+v f t XwK1��v�������xK1��v$�<��Z�xK1��v$���K1��u���KF0��u���K1��v$�<��Z�xK1��v�K1��v�K2僃�� fv	���'0J�v�vv� 0+ p� X t X� t �
Y��K1僃�� fv	���/0f�v�vv� 0+ p� X t X� t �
Y��K1僃�� fv	���'0.�v�vv� 0+ p� X t X� t �
Y��K1僃�� fv	���'0.�v�vv� 0, p� X t X� t �
Y��K2僃����� tXut u���HZ'������ Z&tKX Ju �T � � � ��h��0K�w�����l�t t �Z��	BfB<D< 	v�Y1�"0t<K!$*!t � t <Q1�"0t<' J. t" tL!$)!t � t <R80�tt</!#Y
t t<0t<Kt=!!Yt</ J6 t* tKttJg!*u !t6�3t � t <�1��J�Z	�B�[�L��L��1/1JJ</�a�t$�J	YX( 1�vwt <�1�vwt <�1�vwt <�2�vwt6<�0�vwt <�1�vwt <�1�vwt <�1�vwt <�1�vwt <�1�vwt <�1/ t* t> t9 t$ XQ JN �L�t<7�'��t<N�fL���/ X* �Z)����L����0XL��x�( tt����� t � X9i�Yv�XL u� �t t � X9h�YvwXL vt �txZ1�t�1�t� current _unused2 CURLOPT_MAIL_RCPT _fileno from_charset CURLOPT_FTP_ACCOUNT CURLOPT_READFUNCTION CURLOPT_RTSP_TRANSPORT CURLE_TOO_MANY_REDIRECTS CURLOPT_FILETIME CURLOPT_NOBODY CURLOPT_DISALLOW_USERNAME_IN_URL CURLOPT_TLS13_CIPHERS _curl_easy_getinfo_err_curl_off_t CURLOPT_AUTOREFERER CURL_SSLVERSION_TLSv1_2 CURLOPT_READDATA CURL_SSLVERSION_LAST CURLOPT_ACCEPT_ENCODING _shortbuf explode _curl_easy_setopt_err_FILE CURLOPT_MAXFILESIZE __environ CURLOPT_SSLENGINE_DEFAULT CURLOPT_SSH_KNOWNHOSTS CURLE_CONV_FAILED CURLE_WRITE_ERROR CURLOPT_TIMECONDITION _flags CURLE_SSL_ENGINE_INITFAILED next CURLE_REMOTE_ACCESS_DENIED CURLOPT_STREAM_WEIGHT CURLE_SSL_CACERT_BADFILE __off_t CURLOPT_PROXY_SSLKEYTYPE CURLOPT_FAILONERROR CURLOPT_FNMATCH_DATA CURLE_AGAIN CURLOPT_SSLENGINE CURLE_FTP_COULDNT_SET_TYPE _lock CURLOPT_SEEKFUNCTION CURLOPT_NOPROGRESS CURLOPT_PROXY_SSLCERTTYPE CURLOPT_OBSOLETE40 CURLOPT_PROXY_TRANSFER_MODE CURLE_SSL_CERTPROBLEM CURLOPT_FTP_SKIP_PASV_IP CURLOPT_SSL_VERIFYHOST CURLOPT_RESOLVE CURLOPT_IPRESOLVE CURLE_SEND_ERROR outbuf CURLOPT_RTSP_STREAM_URI CURL_LAST CURL_SSLVERSION_SSLv2 CURL_SSLVERSION_SSLv3 CURLOPT_HTTP200ALIASES CURLOPT_HEADERFUNCTION CURLOPT_PROXY_CAPATH CURLOPT_ABSTRACT_UNIX_SOCKET CURLOPT_DIRLISTONLY CURLE_URL_MALFORMAT CURLOPT_HTTP_TRANSFER_DECODING _libiconv_version CURLOPT_INFILESIZE CURLOPT_SSL_VERIFYSTATUS _IO_write_end _curl_opt CURLOPT_SSL_OPTIONS CURLOPT_SERVICE_NAME CURLOPT_OBSOLETE72 CURLOPT_MAXREDIRS _curl_easy_getinfo_err_double CURL CURLOPT_SUPPRESS_CONNECT_HEADERS desc __tzname CURLOPT_EGDSOCKET CURLOPT_HTTPAUTH CURLE_TFTP_UNKNOWNID CURLOPT_WRITEFUNCTION CURLOPT_SSL_CTX_FUNCTION CURLOPT_REQUEST_TARGET CURLE_COULDNT_RESOLVE_PROXY CURLOPT_PROXY_SSLCERT CURLOPT_POSTREDIR CURLE_FTP_WEIRD_227_FORMAT CURLOPT_SASL_IR CURLE_TFTP_NOTFOUND CURL_HTTP_VERSION_2TLS CURLOPT_FTP_USE_EPSV CURLE_SSL_CRL_BADFILE optarg CURLE_INTERFACE_FAILED CURLE_TFTP_ILLEGAL _curl_easy_setopt_err_resolver_start_callback CURLE_NOT_BUILT_IN sys_errlist CURLOPT_SSL_FALSESTART CURLOPT_FTP_CREATE_MISSING_DIRS CURLE_READ_ERROR CURLOPT_PROXY_TLSAUTH_PASSWORD curl CURLOPT_TIMEOUT_MS CURLOPT_CONNECT_ONLY CURLOPT_HTTP_CONTENT_DECODING CURLOPT_CERTINFO temp CURLE_BAD_FUNCTION_ARGUMENT inbuf CURLE_BAD_CONTENT_ENCODING _curl_easy_setopt_err_ssl_ctx_cb CURLE_HTTP_RETURNED_ERROR CURL_HTTP_VERSION_NONE CURLE_LDAP_INVALID_URL CURLOPT_SSH_PRIVATE_KEYFILE _chain CURLOPT_SSL_ENABLE_NPN CURLOPT_PROXYUSERNAME unsigned char CURLOPT_INFILESIZE_LARGE CURLOPT_PROXY_SSL_VERIFYPEER CURL_HTTP_VERSION_1_0 CURL_HTTP_VERSION_1_1 CURLOPT_PASSWORD _IO_lock_t CURLOPT_CRLF _curl_easy_setopt_err_error_buffer CURLOPT_SSH_PUBLIC_KEYFILE CURLOPT_RTSP_CLIENT_CSEQ CURLOPT_TRANSFER_ENCODING CURLOPT_NOPROXY CURLOPT_RESUME_FROM CURLE_SSL_ENGINE_SETFAILED CURLOPT_COOKIESESSION CURLOPT_PROXY CURLOPT_HTTPPROXYTUNNEL high CURLOPT_DNS_USE_GLOBAL_CACHE CURLE_COULDNT_CONNECT CURLOPT_TCP_KEEPINTVL CURLOPT_URL CURLOPT_CONNECT_TO libiconv_t CURLOPT_WILDCARDMATCH open _IO_write_ptr CURLE_SEND_FAIL_REWIND CURLOPT_OPENSOCKETFUNCTION CURLOPT_PROGRESSDATA CURLE_LOGIN_DENIED CURLE_USE_SSL_FAILED CURLOPT_ERRORBUFFER _curl_easy_setopt_err_curl_slist _curl_easy_setopt_err_progress_cb CURLOPT_RANGE CURLOPT_SSH_HOST_PUBLIC_KEY_MD5 CURLOPT_CONV_FROM_NETWORK_FUNCTION line CURLOPT_FRESH_CONNECT CURLE_UPLOAD_FAILED CURLOPT_PORT CURLOPT_GSSAPI_DELEGATION CURLOPT_PROXYTYPE CURLOPT_PROXY_SSL_CIPHER_LIST count CURL_HTTP_VERSION_2_0 CURLOPT_FNMATCH_FUNCTION CURLOPT_SSLCERTTYPE CURLOPT_UNIX_SOCKET_PATH CURLOPT_POST size_t CURLOPT_MIMEPOST CURL_SSLVERSION_DEFAULT CURLOPT_CAINFO CURLE_COULDNT_RESOLVE_HOST explode2 CURLOPT_RESUME_FROM_LARGE CURLE_SSL_CIPHER CURLOPT_DEBUGDATA CURLOPT_PRE_PROXY CURLE_GOT_NOTHING CURLOPT_SSH_COMPRESSION _IO_save_base CURLOPT_DNS_INTERFACE CURLOPT_COOKIE CURLOPT_PROXY_TLSAUTH_USERNAME CURLOPT_PRIVATE CURLOPT_FTP_USE_EPRT CURLE_OBSOLETE50 CURLE_PARTIAL_FILE CURLE_REMOTE_DISK_FULL CURLOPT_SOCKOPTDATA CURLE_SSL_SHUTDOWN_FAILED CURLOPT_IGNORE_CONTENT_LENGTH CURLOPT_KRBLEVEL CURLOPT_SSLVERSION CURLE_UNKNOWN_OPTION CURLOPT_DNS_CACHE_TIMEOUT CURLOPT_SSL_ENABLE_ALPN CURLE_HTTP2_STREAM code_convert outlen _curl_easy_setopt_err_conv_cb CURLOPT_TLSAUTH_TYPE CURLOPT_FTP_ALTERNATIVE_TO_USER CURLE_SSH CURLOPT_POSTFIELDSIZE _curl_easy_setopt_err_CURLSH CURLOPT_PROXY_TLS13_CIPHERS last_close CURLOPT_FTP_USE_PRET CURLOPT_MAXCONNECTS /home/sunzhidong/bin/stock_x CURLOPT_TRANSFERTEXT CURLOPT_WRITEDATA CURLE_OK nmemb CURLE_SSL_INVALIDCERTSTATUS CURLOPT_INTERFACE __timezone CURLOPT_HAPROXYPROTOCOL CURLOPT_PROXYPORT userdata CURLOPT_MAIL_FROM CURLOPT_RESOLVER_START_DATA _curl_easy_setopt_err_curl_httpost CURLE_RTSP_CSEQ_ERROR CURLOPT_PROXYPASSWORD CURLE_HTTP2 CURLOPT_TIMEOUT stderr CURLOPT_KEYPASSWD _IO_save_end CURLOPT_HEADERDATA CURLE_SSL_CACERT _curl_easy_getinfo_err_string CURLOPT_TCP_NODELAY CURLOPT_SSH_KEYFUNCTION stdout CURLOPT_PROXY_SSLKEY optopt CURL_SSLVERSION_TLSv1 CURLOPT_REFERER CURLE_RECV_ERROR CURLOPT_FTP_RESPONSE_TIMEOUT CURLOPT_SSLKEY CURLOPT_MAXFILESIZE_LARGE short unsigned int CURLE_FTP_ACCEPT_TIMEOUT CURLOPT_XOAUTH2_BEARER valueint CURLOPT_PROXY_SERVICE_NAME CURLOPT_HTTPGET CURLOPT_REDIR_PROTOCOLS CURLOPT_POSTFIELDS _curl_easy_setopt_err_curl_off_t CURLOPT_IOCTLFUNCTION CURLOPT_DEFAULT_PROTOCOL __off64_t CURLE_LDAP_SEARCH_FAILED CURLE_OUT_OF_MEMORY percent CURLOPT_TLSAUTH_PASSWORD _IO_read_base CURLOPT_LOW_SPEED_TIME _IO_buf_end CURLOPT_HEADER CURLOPT_FORBID_REUSE CURLE_SSL_ENGINE_NOTFOUND CURLOPT_NETRC_FILE opterr CURLOPT_PROXY_SSL_VERIFYHOST CURLOPT_POSTFIELDSIZE_LARGE CURLOPT_SSL_CTX_DATA CURLOPT_SSH_KEYDATA CURLOPT_KEEP_SENDING_ON_ERROR _mode CURLOPT_CONV_TO_NETWORK_FUNCTION _IO_write_base CURLE_NO_CONNECTION_AVAILABLE CURLOPT_SSLCERT CURLE_OBSOLETE20 tz_dsttime CURLE_OBSOLETE24 CURLE_OBSOLETE29 CURLOPT_PROTOCOLS CURLE_FTP_COULDNT_RETR_FILE urllen CURLOPT_PROXY_KEYPASSWD CURLOPT_LOCALPORT CURLOPT_LOW_SPEED_LIMIT CURLE_FTP_WEIRD_PASV_REPLY CURLE_OBSOLETE32 CURLOPT_DEBUGFUNCTION CURLOPT_LOGIN_OPTIONS CURLE_BAD_DOWNLOAD_RESUME CURLE_RTSP_SESSION_ERROR CURLOPT_PINNEDPUBLICKEY CURLOPT_EXPECT_100_TIMEOUT_MS CURLOPT_MAIL_AUTH CURLOPT_STDERR CURLOPT_SSH_AUTH_TYPES _IO_marker _curl_easy_setopt_err_long CURLOPT_MAX_RECV_SPEED_LARGE stockcode CURLOPT_PROXY_SSLVERSION CURLOPT_VERBOSE CURLOPT_FTPSSLAUTH CURLE_OBSOLETE40 CURLE_OBSOLETE44 CURLE_OBSOLETE46 CURLOPT_USERPWD CURLOPT_SEEKDATA CURLOPT_SSL_SESSIONID_CACHE CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS CURLE_ABORTED_BY_CALLBACK CURLE_SSL_PINNEDPUBKEYNOTMATCH CURLOPT_PROXY_CRLFILE CURLE_RANGE_ERROR CURLOPT_HEADEROPT _IO_codecvt CURLE_OBSOLETE57 CURLOPT_RTSP_SERVER_CSEQ volume long double CURLOPT_TIMEVALUE_LARGE CURLOPT_HTTP_VERSION CURLOPT_TIMEVALUE CURLE_QUOTE_ERROR parse CURLOPT_PROXYUSERPWD CURLOPT_COOKIEFILE _curl_easy_setopt_err_curl_mimepost CURLOPT_FOLLOWLOCATION CURLE_REMOTE_FILE_EXISTS CURLOPT_PIPEWAIT CURLOPT_RESOLVER_START_FUNCTION CURLOPT_COOKIELIST stock_x.c _curl_easy_setopt_err_postfields write_data stdin to_charset _IO_buf_base CURLOPT_NEW_FILE_PERMS CURLOPT_FTP_FILEMETHOD CURLOPT_USE_SSL _IO_read_end CURLOPT_CAPATH _curl_easy_getinfo_err_curl_tlssesssioninfo CURLOPT_USERAGENT CURLOPT_DNS_LOCAL_IP6 _IO_FILE _curl_easy_setopt_err_string CURLE_TELNET_OPTION_SYNTAX _IO_wide_data sector CURLOPT_APPEND CURLE_RECURSIVE_API_CALL CURLE_FILESIZE_EXCEEDED valuestring CURLOPT_DNS_LOCAL_IP4 CURLOPT_PROXY_PINNEDPUBLICKEY amount CURLOPT_COOKIEJAR CURLE_FTP_COULDNT_USE_REST GNU C17 8.3.1 20190507 (Red Hat 8.3.1-4) -mtune=generic -march=x86-64 -g _curl_easy_getinfo_err_curl_socket __pad5 CURLOPT_CUSTOMREQUEST CURLOPT_TLSAUTH_USERNAME CURLOPT_CRLFILE CURLOPT_TCP_KEEPALIVE _markers CURLOPT_OPENSOCKETDATA CURLOPT_TELNETOPTIONS _curl_easy_getinfo_err_curl_certinfo CURLE_FILE_COULDNT_READ_FILE codes CURLOPT_NETRC CURLOPT_PROXYHEADER CURLOPT_PREQUOTE CURLOPT_HTTPHEADER CURLOPT_TFTP_BLKSIZE CURLOPT_SSL_VERIFYPEER argc seperator CURLOPT_PROXY_CAINFO CURLE_FTP_ACCEPT_FAILED argv _curl_easy_setopt_err_ioctl_cb CURLOPT_LOCALPORTRANGE CURLOPT_CONV_FROM_UTF8_FUNCTION __daylight _curl_easy_getinfo_err_curl_slist head CURLOPT_CHUNK_DATA CURLOPT_CHUNK_END_FUNCTION CURLOPT_HTTPPOST CURLE_WEIRD_SERVER_REPLY CURLE_UNSUPPORTED_PROTOCOL CURLOPT_PROGRESSFUNCTION CURLOPT_POSTQUOTE CURLOPT_INTERLEAVEDATA _curl_easy_setopt_err_debug_cb CURLOPT_RTSP_REQUEST CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE CURLcode _freeres_buf CURLOPT_IOCTLDATA _curl_easy_setopt_err_opensocket_cb CURLOPT_PROXY_TLSAUTH_TYPE long long unsigned int _cur_column CURLOPT_SOCKS5_GSSAPI_SERVICE CURLOPT_NEW_DIRECTORY_PERMS CURLOPT_PATH_AS_IS CURLOPT_FTP_SSL_CCC CURLOPT_STREAM_DEPENDS_E CURLOPT_SOCKS5_GSSAPI_NEC CURLOPT_PROXYAUTH CURLE_TFTP_PERM CURLE_FTP_PORT_FAILED CURLOPT_PUT CURLOPT_TCP_FASTOPEN CURLOPT_FTPPORT _curl_easy_setopt_err_write_callback CURLOPT_DNS_SHUFFLE_ADDRESSES _IO_backup_base CURL_SSLVERSION_TLSv1_0 CURL_SSLVERSION_TLSv1_1 _IO_read_ptr CURL_SSLVERSION_TLSv1_3 CURLOPT_SSLKEYTYPE CURLE_OPERATION_TIMEDOUT pout _curl_easy_setopt_err_sockopt_cb CURLE_FTP_BAD_FILE_LIST _freeres_list CURLOPT_SOCKS5_AUTH CURLOPT_STREAM_DEPENDS CURLE_CHUNK_FAILED CURLOPT_UPLOAD CURLOPT_ADDRESS_SCOPE CURLOPT_TFTP_NO_OPTIONS CURLE_TFTP_NOSUCHUSER _old_offset CURLOPT_DNS_SERVERS CURLOPT_PROXY_SSL_OPTIONS CURLOPT_CONNECTTIMEOUT_MS CURLOPT_TCP_KEEPIDLE optind CURLE_HTTP_POST_ERROR CURLE_FAILED_INIT cJSON long long int CURLOPT_CONNECTTIMEOUT CURLE_CONV_REQD _flags2 CURLOPT_CLOSESOCKETDATA _curl_easy_setopt_err_read_cb CURLOPT_INTERLEAVEFUNCTION CURLE_FTP_PRET_FAILED pre_flag bigLen valuedouble sys_nerr inlen CURLOPT_SOCKOPTFUNCTION CURLOPT_ACCEPTTIMEOUT_MS CURLOPT_SHARE CURLOPT_ISSUERCERT CURLOPT_LASTENTRY CURLOPT_SSL_CIPHER_LIST _curl_easy_getinfo_err_long CURLE_SSL_ISSUER_ERROR CURLOPT_CHUNK_BGN_FUNCTION main CURLOPT_UNRESTRICTED_AUTH CURLOPT_XFERINFOFUNCTION CURLOPT_COPYPOSTFIELDS _curl_easy_setopt_err_cb_data CURLOPT_NOSIGNAL CURLOPT_RTSP_SESSION_ID CURLOPT_QUOTE CURLOPT_MAX_SEND_SPEED_LARGE CURLE_FTP_WEIRD_PASS_REPLY CURLOPT_RANDOM_FILE short int CURLE_REMOTE_FILE_NOT_FOUND CURLE_FUNCTION_NOT_FOUND CURLE_SSL_CONNECT_ERROR CURLE_LDAP_CANNOT_BIND prev _vtable_offset _curl_easy_setopt_err_seek_cb CURLOPT_BUFFERSIZE tz_minuteswest CURLOPT_USERNAME CURLOPT_CLOSESOCKETFUNCTION CURL_HTTP_VERSION_LAST CURLE_FTP_CANT_GET_HOST CURLE_PEER_FAILED_VERIFICATION cJSON_GetObjectItem compare_double malloc_fn return_parse_end recurse input cJSON_CreateIntArray cJSON_AddTrueToObject format print_string_ptr cJSON_CreateFalse current_child cJSON_free cJSON_CreateRaw signgam second_code index newchild escape_characters strings after_end print_string a_element cJSON_Duplicate cJSON_CreateObjectReference constant_key cJSON_IsInvalid cJSON_CreateArrayReference parse_hex4 input_end copy print_value cJSON_GetStringValue cJSON_ReplaceItemInArray cJSON_ReplaceItemInObjectCaseSensitive string_item numbers parse_value global_error cJSON_AddArrayToObject cJSON_GetNumberValue float cJSON_strdup newbuffer cJSON_Version cJSON_bool allocation_length cJSON.c cJSON_CreateDoubleArray cJSON_ParseWithLengthOpts raw_item parse_string cJSON_HasObjectItem cJSON_AddFalseToObject parse_object suffix_object number_buffer cJSON_AddNullToObject string1 string2 cJSON_PrintUnformatted cJSON_ParseWithLength cJSON_InitHooks true_item cJSON_IsNumber utf8_position output get_array_item number_c_string cJSON_IsTrue deallocate case_sensitive skip_multiline_comment cJSON_AddItemToObject utf16_literal_to_utf8 cJSON_GetArrayItem cJSON_ReplaceItemViaPointer cJSON_IsBool new_type second_sequence cJSON_CreateNumber cJSON_GetObjectItemCaseSensitive parse_array b_element cJSON_AddItemToArray cJSON_AddItemReferenceToObject replacement get_object_item boolean prebuffer into local_error cJSON_IsNull internal_hooks depth maxVal cJSON_CreateTrue cJSON_GetArraySize sequence_length printed cJSON_ParseWithOpts cJSON_malloc newsize cJSON_AddStringToObject cJSON_DeleteItemFromArray cJSON_DetachItemFromArray add_item_to_array replace_item_in_object cJSON_IsObject cJSON_CreateBool current_element cJSON_DeleteItemFromObject skipped_bytes print_number add_item_to_object codepoint ensure raw_length cJSON_AddNumberToObject case_insensitive_strcmp which null cJSON_IsString cJSON_AddItemToObjectCS reallocate fail first_sequence cJSON_CreateNull print parent parse_number cJSON_GetErrorPtr cJSON_SetNumberHelper cJSON_IsArray new_key noalloc cJSON_Print cJSON_DetachItemFromObject node cast_away_const json cJSON_AddObjectToObject input_buffer cJSON_DetachItemFromObjectCaseSensitive cJSON_InsertItemInArray cJSON_IsRaw success cJSON_Minify newitem input_pointer update_offset require_null_terminated cJSON_Parse cJSON_PrintPreallocated default_buffer_size cJSON_CreateStringArray printbuffer current_item print_array cJSON_Compare skip_utf8_bom cJSON_CreateArray print_object cJSON_AddRawToObject test cJSON_SetValuestring utf8_length content cJSON_AddBoolToObject skip_oneline_comment output_length needed after_inserted cJSON_Delete get_decimal_point cJSON_New_Item buffer_skip_whitespace output_buffer bool_item cJSON_DetachItemViaPointer create_reference number_item cJSON_CreateObject cJSON_IsFalse loop_end to_detach cJSON_PrintBuffered buffer_pointer free_fn output_pointer first_byte_mark cJSON_Hooks buffer_length first_code cJSON_CreateStringReference minify_string cJSON_DeleteItemFromObjectCaseSensitive parse_buffer cJSON_AddItemReferenceToArray false_item global_hooks cJSON_ReplaceItemInObject cJSON_CreateString new_item cJSON_CreateFloatArray m            �      �                      m      g      �      �                      �      r      �      �                      �"      T#      %$      +$                      C'      �(      Z)      f)                                                      8@                   T@                   t@                   �@                   �@                   h@                   D@                   �@                  	 �@                  
 0	@                    @                    @                   @                   (i@                   @i@                   `r@                   @w@                   ��`                   �`                   ��`                   ��`                    �`                   �`                   @�`                                         x��                                                                                                                                                      ��                =     ?@             M     ?@             a     @             u     @             �     @             �     @             �     @             �     @             �     @                 @             %    @@             =    E@             Y    @             u    @             �    @             �    @             �    @             �    @                 @             <    @             ]    @@             �    E@             �   ��                �    �h@             �    %i@             �    @             �    @                 @             2    @             S    @             o    @             �    @             �    @             �    �h@             �    i@                 i@                  %i@             =   ��                H    P@             J    �@             ]    �@             s    @�`            �    �`             �    �@             �    ��`             �   ��                �    �@            �    �@                �@            7    �@            \    �@            �    �@            �    @            �    @            �    @                @            .    "@            M    )@            n    0@            �    7@            �    >@            �    E@            �    L@                S@            '    Z@            J    a@            n    h@            �    o@            �    v@            �    }@            �    �@                �@            &    �@            R    �@            w    �@            �    �@            �   ��                �    P�`            �    `�`            �    J@     �       �     �`            	    �@     �       	    ,@     A       	    F@            1	    Q@     �      >	    g"@           E	    �$@     h       S	    �$@     �       b	    z%@     �      o	    o'@           z	    �(@           �	    �*@     .      �	    �-@     �      �	    �0@     )       �	    �0@     �       �	    y1@     �       �	    08@     �      �	    �4@     �      �	     r@            
    ";@     -      
    O=@     �      #
    �A@     o      0
    �?@     �      <
    'E@     �      I
    *I@     T       X
    �I@     �       h
    K@     &       v
    *K@     �       �
    �K@     �       �
    �L@            �
    �L@           �
    V@     �       �
    �`@     `       �
    �`@     r       �
    na@     0      =   ��                    ��@                  ��                     `r@             .    ��`             7     �`             H     ��`             [     �`             q    �Y@     n       �                     �                     �    S@     <       �    �c@     +       �    �X@     I       �                     �    �P@     `           ~I@     3           d@     *       !    O@     U       7    �b@     �       D    8�`             K                     9     �`             Z    @i@            i    �M@     5       �    �@           �    �@     -       �                     �                     �    rh@            �                     �    mP@     `       �    m@     �       �    QZ@     .                            '    p�`            +    �@           0    �d@     +       ?    �]@     
      W                     l    �J@     *       �                     �    �@            �   Hi@             �    �W@     ?       �    �J@     .       �    �T@     #      �    BS@     <                            1    �R@     E       K    �d@     +       �   (i@             Y    �S@     .       �   @@            �    P@     _       �                     �    lY@     I       �                     �    o6@     �                                �c@     +       )    �\@           A    �@     �       L    !@     n       b    �V@     /       �                     �    XN@     Z       �                     �                     =    @     /       �                     �    #Y@     I           �@     E           e@     -       )    �@     G       -    Z@     
      B    j7@     �       Z                     [    @             i    2@     N       }    �h@            �   8�`             �    �R@     +       �    �S@     �       �                     �    h4@     ,       �     p@                 \O@     U                            ,    +@     �      5    �^@     �      E    �Q@     U       \    �W@     �       o    HW@     .       �    '6@     $       �    �Q@     �       �    �[@           �    W@     .       �    lX@     n       �     �@             �    :d@     +           �U@     M           ~S@     .       7    �`             D    D4@     $       T    x�`             P    \2@     �      j    -Q@     U       �                     �                     �    �d@     +       �                     �                     �    vW@     .       �    pL@     %            �M@     5           8�`             "    �c@     *       2    �@     �       ?                     Q    �h@     e       a                     t    �O@     ]       �    �J@     *       �    @     /       �      @             �    �H@     Q       �    N@     C       �    �!@     �           K6@     $       +    >e@     4      9    �@     1       N                      j    #Z@     .       |    u@     �       �     i@            �    ed@     +       �                     �                      �                      �    �V@     /           �N@     U        /usr/lib/gcc/x86_64-redhat-linux/8/../../../../lib64/crt1.o .annobin_init.c .annobin_init.c_end .annobin_init.c.hot .annobin_init.c_end.hot .annobin_init.c.unlikely .annobin_init.c_end.unlikely .annobin_init.c.startup .annobin_init.c_end.startup .annobin_init.c.exit .annobin_init.c_end.exit .annobin_static_reloc.c .annobin_static_reloc.c_end .annobin_static_reloc.c.hot .annobin_static_reloc.c_end.hot .annobin_static_reloc.c.unlikely .annobin_static_reloc.c_end.unlikely .annobin_static_reloc.c.startup .annobin_static_reloc.c_end.startup .annobin_static_reloc.c.exit .annobin_static_reloc.c_end.exit .annobin__dl_relocate_static_pie.start .annobin__dl_relocate_static_pie.end elf-init.oS .annobin_elf_init.c .annobin_elf_init.c_end .annobin_elf_init.c.hot .annobin_elf_init.c_end.hot .annobin_elf_init.c.unlikely .annobin_elf_init.c_end.unlikely .annobin_elf_init.c.startup .annobin_elf_init.c_end.startup .annobin_elf_init.c.exit .annobin_elf_init.c_end.exit .annobin___libc_csu_init.start .annobin___libc_csu_init.end .annobin___libc_csu_fini.start .annobin___libc_csu_fini.end crtstuff.c deregister_tm_clones __do_global_dtors_aux completed.7294 __do_global_dtors_aux_fini_array_entry frame_dummy __frame_dummy_init_array_entry stock_x.c _curl_easy_setopt_err_long _curl_easy_setopt_err_curl_off_t _curl_easy_setopt_err_string _curl_easy_setopt_err_write_callback _curl_easy_setopt_err_resolver_start_callback _curl_easy_setopt_err_read_cb _curl_easy_setopt_err_ioctl_cb _curl_easy_setopt_err_sockopt_cb _curl_easy_setopt_err_opensocket_cb _curl_easy_setopt_err_progress_cb _curl_easy_setopt_err_debug_cb _curl_easy_setopt_err_ssl_ctx_cb _curl_easy_setopt_err_conv_cb _curl_easy_setopt_err_seek_cb _curl_easy_setopt_err_cb_data _curl_easy_setopt_err_error_buffer _curl_easy_setopt_err_FILE _curl_easy_setopt_err_postfields _curl_easy_setopt_err_curl_httpost _curl_easy_setopt_err_curl_mimepost _curl_easy_setopt_err_curl_slist _curl_easy_setopt_err_CURLSH _curl_easy_getinfo_err_string _curl_easy_getinfo_err_long _curl_easy_getinfo_err_double _curl_easy_getinfo_err_curl_slist _curl_easy_getinfo_err_curl_tlssesssioninfo _curl_easy_getinfo_err_curl_certinfo _curl_easy_getinfo_err_curl_socket _curl_easy_getinfo_err_curl_off_t cJSON.c global_error version.4456 case_insensitive_strcmp global_hooks cJSON_strdup cJSON_New_Item get_decimal_point parse_number ensure update_offset compare_double print_number parse_hex4 utf16_literal_to_utf8 parse_string print_string_ptr print_string buffer_skip_whitespace skip_utf8_bom parse_value print default_buffer_size.4729 print_value parse_array parse_object print_array print_object get_array_item get_object_item suffix_object create_reference add_item_to_array cast_away_const add_item_to_object replace_item_in_object skip_oneline_comment skip_multiline_comment minify_string __FRAME_END__ __GNU_EH_FRAME_HDR _DYNAMIC __init_array_end __init_array_start _GLOBAL_OFFSET_TABLE_ cJSON_CreateRaw curl_easy_perform cJSON_DetachItemFromObject cJSON_IsTrue cJSON_CreateStringReference sprintf@@GLIBC_2.2.5 cJSON_AddRawToObject cJSON_GetArrayItem cJSON_IsBool cJSON_AddTrueToObject cJSON_Minify _edata curl_easy_init _IO_stdin_used cJSON_AddItemToObjectCS explode cJSON_GetStringValue libiconv curl_slist_append cJSON_malloc strlen@@GLIBC_2.2.5 cJSON_AddStringToObject cJSON_Delete cJSON_CreateObject strncmp@@GLIBC_2.2.5 res main cJSON_IsObject cJSON_CreateStringArray strncpy@@GLIBC_2.2.5 cJSON_GetObjectItem memset@@GLIBC_2.2.5 cJSON_GetErrorPtr __dso_handle cJSON_CreateBool cJSON_HasObjectItem cJSON_ReplaceItemViaPointer cJSON_DetachItemFromObjectCaseSensitive strncat@@GLIBC_2.2.5 cJSON_DetachItemFromArray cJSON_IsArray cJSON_DeleteItemFromObjectCaseSensitive _dl_relocate_static_pie cJSON_AddNumberToObject strtod@@GLIBC_2.2.5 cJSON_CreateArrayReference memcpy@@GLIBC_2.14 cJSON_PrintBuffered perror@@GLIBC_2.2.5 cJSON_IsFalse cJSON_CreateDoubleArray write_data cJSON_SetNumberHelper cJSON_ReplaceItemInObjectCaseSensitive strcpy@@GLIBC_2.2.5 cJSON_AddItemReferenceToObject curl_easy_cleanup __isoc99_sscanf@@GLIBC_2.7 fclose@@GLIBC_2.2.5 cJSON_CreateObjectReference g2u cJSON_IsRaw u2g cJSON_CreateIntArray cJSON_PrintPreallocated libiconv_close cJSON_ParseWithOpts cJSON_free __TMC_END__ cJSON_DeleteItemFromArray cJSON_InsertItemInArray fopen@@GLIBC_2.2.5 cJSON_ParseWithLength free@@GLIBC_2.2.5 cJSON_AddFalseToObject curl_slist_free_all explode2 cJSON_Duplicate cJSON_AddArrayToObject cJSON_CreateNumber cJSON_CreateTrue cJSON_Print cJSON_DetachItemViaPointer cJSON_CreateFloatArray cJSON_CreateNull cJSON_CreateString malloc@@GLIBC_2.2.5 cJSON_IsNull cJSON_ReplaceItemInArray cJSON_DeleteItemFromObject __data_start cJSON_Parse cJSON_ParseWithLengthOpts cJSON_AddObjectToObject strcmp@@GLIBC_2.2.5 curl_easy_setopt cJSON_IsString libiconv_open tolower@@GLIBC_2.2.5 cJSON_CreateFalse cJSON_AddItemToArray cJSON_AddItemToObject __bss_start cJSON_IsInvalid code_convert feof@@GLIBC_2.2.5 __libc_csu_init fgets@@GLIBC_2.2.5 cJSON_AddBoolToObject cJSON_GetObjectItemCaseSensitive cJSON_Version realloc@@GLIBC_2.2.5 cJSON_GetArraySize cJSON_AddItemReferenceToArray cJSON_SetValuestring cJSON_PrintUnformatted cJSON_Compare cJSON_GetNumberValue _ITM_deregisterTMCloneTable cJSON_CreateArray cJSON_InitHooks __libc_csu_fini cJSON_IsNumber __libc_start_main@@GLIBC_2.2.5 __gmon_start__ _ITM_registerTMCloneTable cJSON_ReplaceItemInObject cJSON_AddNullToObject  .symtab .strtab .shstrtab .interp .note.ABI-tag .note.gnu.build-id .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt .init .text .fini .rodata .eh_frame_hdr .eh_frame .init_array .fini_array .dynamic .got .got.plt .data .bss .comment .gnu.build.attributes .debug_aranges .debug_info .debug_abbrev .debug_line .debug_str .debug_ranges                                                                                    8@     8                                    #             T@     T                                     1             t@     t      $                              D   ���o       �@     �      <                             N             �@     �      �                          V             h@     h      �                             ^   ���o       D@     D      L                            k   ���o       �@     �      @                            z             �@     �      `                            �      B       0	@     0	      �                          �              @                                          �              @            �                            �             @           [                             �             (i@     (i                                    �             @i@     @i       	                             �             `r@     `r      �                             �             @w@     @w      �                             �             ��`     ��                                   �             �`     �                                   �             ��`     ��      �                           �             ��`     ��                                    �              �`      �                                  �             �`     �      (                              �             @�`     8�      8                              �      0               8�      ,                                          x��     d�      0                                                  ��      `                              (                     ��      �C                             4                     ��      P                             B                     ��      Y$                             N     0               B     t4                            Y                     �K     �                                                    �L     H      "   �                 	                      �g     #                                                   }     g                                                                                                                                                             stock_x.c                                                                                           0000644 0000000 0000000 00000011571 14212320276 011376  0                                                                                                    ustar   root                            root                                                                                                                                                                                                                   #include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "util.c"
#include <curl/curl.h>
#include "cJSON.h"

char * res;

size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
	//printf("ptr = %s\n", ptr);
	//printf("userdata = %s\n", userdata);
    if (res == NULL) {
	    res = (char *)realloc(res, sizeof(char) * (strlen(ptr)+1));
	    strcpy(res, ptr);
    } else {
        res = (char *)realloc(res, sizeof(char) * (strlen(res) + strlen(ptr) + 1));
        strncat(res, ptr, strlen(ptr));
    }
	//printf("res_callback:=%s\n", res);
	return size * nmemb;
}

int main(int argc, char *argv[]) {
	res = (char *)malloc(sizeof(char) * 1000);
    res = NULL;
	//read stock code from config
	FILE *fp = fopen("config_x.ini", "r");
	if (NULL == fp) {
		perror("Open config file failed!");
	}
	
	//输出标题
	printf("%-15s|%-15s|%-15s|%-15s|%-15s|%-20s|%-20s|%-20s|%-20s|\n", "名称", "当前", "涨幅(%)", "昨收", "今开", "当日最高", "当日最低", "成交数(手)", "成交金额(万元)");

	char line[10];
	char * stockcode;
	stockcode = (char *)malloc(sizeof(char) * 10);
	stockcode = 0;
	char *flag = ",";
	while (!feof(fp)) {
		memset(line, 0, sizeof(line));
		if (fgets(line, sizeof(line), fp) != NULL) {
			int len = strlen(line);
			//printf("line:=%s, stockcode:=%s\n", line, stockcode);
			if (stockcode == 0) {
				stockcode = (char *)realloc(stockcode, sizeof(char) * (len + 1));
				strncpy(stockcode, line, len - 1);
			} else {
				int bigLen = strlen(stockcode);
				stockcode = (char *)realloc(stockcode, sizeof(char) * (bigLen + 1));
				strncat(stockcode, flag, 1);
				bigLen = strlen(stockcode);
				stockcode = (char *)realloc(stockcode, sizeof(char) * (bigLen + len + 1));
				strncat(stockcode, line, len - 1);
			}
		}
	}
	fclose(fp);

	CURL * curl = curl_easy_init();
	CURLcode code;
	char * url = NULL;
	char * sector = "https://stock.xueqiu.com/v5/stock/realtime/quotec.json?symbol=";
	url = (char *)malloc(sizeof(char) * (strlen(sector) + 1));
	strncpy(url, sector, strlen(sector));
	//printf("urlurlurl:=%s, len=%d\n\n", url, strlen(sector));
	int urllen = strlen(url);
	url = (char *)realloc(url, sizeof(char) * (urllen + strlen(stockcode) + 1));
	strncat(url, stockcode, strlen(stockcode));
	//printf("url:=%s\n", url);

	struct curl_slist * head = NULL;
	curl_slist_append(head, "authority: stock.xueqiu.com");
	curl_slist_append(head, "method: GET");
	curl_slist_append(head, "scheme: https");
	curl_slist_append(head, "accept-encoding: gzip, deflate, br");
	curl_slist_append(head, "User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/99.0.4844.51 Safari/537.36");
	

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	code = curl_easy_perform(curl);
	curl_slist_free_all(head);
	curl_easy_cleanup(curl);
	//cjson
    //printf("res:=%s\n", res);
    cJSON *parse = cJSON_Parse(res);
    const cJSON *data = NULL;
    const cJSON *item = NULL;
    const cJSON *codes = NULL;
    const cJSON *desc = NULL;

    codes = cJSON_GetObjectItemCaseSensitive(parse, "error_code");
    if (cJSON_IsNumber(codes) && codes->valuedouble == 0) {
        //printf("code:=%d\n", codes->valueint);
        data = cJSON_GetObjectItemCaseSensitive(parse, "data");
        cJSON_ArrayForEach(item, data) {
            cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "symbol");
            cJSON *current = cJSON_GetObjectItemCaseSensitive(item, "current");
            cJSON *percent = cJSON_GetObjectItemCaseSensitive(item, "percent");
            cJSON *last_close = cJSON_GetObjectItemCaseSensitive(item, "last_close");
            cJSON *open = cJSON_GetObjectItemCaseSensitive(item, "open");
            cJSON *high = cJSON_GetObjectItemCaseSensitive(item, "high");
            cJSON *low = cJSON_GetObjectItemCaseSensitive(item, "low");
            cJSON *volume = cJSON_GetObjectItemCaseSensitive(item, "volume");
            cJSON *amount = cJSON_GetObjectItemCaseSensitive(item, "amount");

            printf("%-13s|%-13.2f|%-13.2f|%-13.2f|%-13.2f|%-16.2f|%-16.2f|%-16d|%-14d|\n", name->valuestring, current->valuedouble, percent->valuedouble, last_close->valuedouble, open->valuedouble, high->valuedouble, low->valuedouble, volume->valueint/100, amount->valueint/1000);
        }
    }

    cJSON_Delete(parse);
    free(stockcode);
	free(res);
	free(url);
	return 0;
}
                                                                                                                                       util.c                                                                                              0000644 0000000 0000000 00000004672 14201137633 010706  0                                                                                                    ustar   root                            root                                                                                                                                                                                                                   #include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
#include<string.h>
#include<unistd.h>
#include<iconv.h>
#define OUTLEN 255

/*代码转换:从一种编码转为另一种编码*/
int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;
     
    cd = iconv_open(to_charset,from_charset);
    if (cd==0) return -1;
    memset(outbuf,0,outlen);
    if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
    iconv_close(cd);
    return 0;
}
/*UNICODE码转为GB2312码*/
int u2g(char *inbuf,int inlen,char *outbuf,int outlen) {
    return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}

/*GB2312码转为UNICODE码*/
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen) {
    return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}

void explode2(char * str, char seperator, char *data[]) {
	int len = strlen(str);
	int i;
	int pre_flag = 0;
	int count = 0;
	
	printf("data pointer = %p\n", data);
	for(i = 0; i < len; i++) {
		if (*(str + i) == seperator) {
			char *temp;
			printf("aaa = %d\n", i - pre_flag);
			temp = (char *)malloc(sizeof(char) * (i - pre_flag));
			strncpy(temp, str + pre_flag, i - pre_flag);
			printf("temp = %s\n", temp);
			data[count] = temp;
			printf("count = %d\n", count);
			printf("data pointer = %p\n", data[count]);
			printf("data['count'] = %s\n", data[count]);
			pre_flag = i + 1;
			count++;
		}
	}
	if (pre_flag < len) {
		data[count] = str + pre_flag;
	}
}

void explode(char str[], char seperator, char *data[]) {
	int len = strlen(str);
	//char * data[32];
	int i;
	int pre_flag = 0;
	int count = 0;
	
	//printf("data pointer = %p\n", data);
	//printf("len = %d\n", len);
	for(i = 0; i < len; i++) {
		if (str[i] == seperator) {
			str[i] = '\0';
			char * temp;
			temp = (char *)malloc(sizeof(char) * (i - pre_flag + 1));
            /*if (i - pre_flag > 0) {
                temp = (char *)realloc(temp, sizeof(char) * (i - pre_flag + 1));
            } else {
                temp = (char *)realloc(temp, sizeof(char) * 1);
            }*/
			strncpy(temp, str + pre_flag, i - pre_flag + 1);
            char *p = temp;
			//printf("temp = %s\n", temp);
			data[count] = temp;
			pre_flag = i + 1;
			count++;
		}
	}
	if (pre_flag < len) {
		data[count] = str + pre_flag;
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      