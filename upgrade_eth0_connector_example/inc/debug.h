/*
 * $Id: debug.h,v 1.5 2006/01/30 23:07:57 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdlib.h>
#include <string.h> //added by yuegui.he
#include <malloc.h>


#ifdef __cplusplus
extern "C" {
#endif

extern void mc_set_debug(int debug);
extern int mc_get_debug(void);

extern void mc_set_syslog(int syslog);
extern void mc_abort(const char *msg, ...);
extern void mc_debug(const char *msg, ...);
extern void mc_error(const char *msg, ...);
extern void mc_info(const char *msg, ...);

#ifndef __STRING
#define __STRING(x) #x
#endif

#ifndef PARSER_BROKEN_FIXED

#define JASSERT(cond) do {} while(0)

#else

#define JASSERT(cond) do { \
		if (!(cond)) { \
			mc_error("cjson assert failure %s:%d : cond \"" __STRING(cond) "failed\n", __FILE__, __LINE__); \
			*(int *)0 = 1;\
			abort(); \
		}\
	} while(0)

#endif

#define MC_ABORT(x, ...) mc_abort(x, ##__VA_ARGS__)
#define MC_ERROR(x, ...) mc_error(x, ##__VA_ARGS__)

#ifdef MC_MAINTAINER_MODE
#define MC_SET_DEBUG(x) mc_set_debug(x)
#define MC_GET_DEBUG() mc_get_debug()
#define MC_SET_SYSLOG(x) mc_set_syslog(x)
#define MC_DEBUG(x, ...) mc_debug(x, ##__VA_ARGS__)
#define MC_INFO(x, ...) mc_info(x, ##__VA_ARGS__)
#else
#define MC_SET_DEBUG(x) if (0) mc_set_debug(x)
#define MC_GET_DEBUG() (0)
#define MC_SET_SYSLOG(x) if (0) mc_set_syslog(x)
#define MC_DEBUG(x, ...) if (0) mc_debug(x, ##__VA_ARGS__)
#define MC_INFO(x, ...) if (0) mc_info(x, ##__VA_ARGS__)
#endif

/*added by yuegui.he for JRD_MALLOC_JSONC*/
#define JRD_MALLOC_JSONC(sz,ptr) \
  do{  \
      (ptr)=malloc((sz));  \
      if(NULL!=(ptr))  \
      memset((ptr),0,(sz)); \
  }while(0)

/*added by yuegui.he for JRD_FREE_JSONC*/
#define JRD_FREE_JSONC(ptr) \
  do{  \
      if(NULL!=(ptr)) \
        free((ptr));  \
      (ptr) = NULL; \
  }while(0)


/*added by yuegui.he for JRD_REAALLOC*/
#define JRD_REALLOC_JSONC(ptr, sz_new, out_ptr) \
    do { \
        out_ptr = realloc(ptr, sz_new); \
    } while(0)


/*added by yuegui.he for JRD_CALLOC_JSONC*/
#define JRD_CALLOC_JSONC(count, sz, out_ptr) \
            do { \
                out_ptr = calloc(count, sz); \
            } while(0)

/*added by yuegui.he for JRD_MEMCPY_JSONC*/
#define JRD_MEMCPY_JSONC(dest, src, sz) \
                do{  \
                    if((dest) && (src) && (sz)) \
                      memcpy((dest), (src), (sz)); \
                }while(0)
                
/*added by yuegui.he for JRD_MEMSET_JSONC*/           
#define JRD_MEMSET_JSONC(dest, ch, sz) \
                do{  \
                    if((dest) && (sz)) \
                      memset((dest), (ch), (sz)); \
                }while(0)


#ifdef __cplusplus
}
#endif

#endif
