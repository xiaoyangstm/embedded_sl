/*
  Copyright (c) 2009 Dave Gamble
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C" {
#endif

#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

typedef struct cJSON {
	struct cJSON *next,*prev;
	struct cJSON *child;
	int type;
	char *valuestring;
	int valueint;
	double valuedouble;
	char *string;
} cJSON;

typedef struct cJSON_Hooks {
	void *(*malloc_fn)(size_t sz);
	void (*free_fn)(void *ptr);
} cJSON_Hooks;

extern void cJSON_InitHooks(cJSON_Hooks* hooks);
extern cJSON *cJSON_Parse(const char *value);
extern char  *cJSON_Print(cJSON *item);
extern char  *cJSON_PrintUnformatted(cJSON *item);
extern void   cJSON_Delete(cJSON *c);
extern int	  cJSON_GetArraySize(cJSON *array);
extern cJSON *cJSON_GetArrayItem(cJSON *array,int item);
extern cJSON *cJSON_GetObjectItem(cJSON *object,const char *string);
extern const char *cJSON_GetErrorPtr(void);

extern cJSON *cJSON_CreateNull(void);
extern cJSON *cJSON_CreateTrue(void);
extern cJSON *cJSON_CreateFalse(void);
extern cJSON *cJSON_CreateBool(int b);
extern cJSON *cJSON_CreateNumber(double num);
extern cJSON *cJSON_CreateString(const char *string);
extern cJSON *cJSON_CreateArray(void);
extern cJSON *cJSON_CreateObject(void);

extern void cJSON_AddItemToArray(cJSON *array, cJSON *item);
extern void	cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item);

#define cJSON_AddNullToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object,name)		cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddBoolToObject(object,name,b)	cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))
#define cJSON_AddNumberToObject(object,name,n)	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object,name,s)	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

#ifdef __cplusplus
}
#endif

#endif