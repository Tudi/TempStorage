#ifndef K_UTILS_H
#define K_UTILS_H

#include <macro_utils.h>
#include <utils.h>
#include <stdbool.h>
#include <kvec.h>

#define FREE_KVEC(VEC, FREE_FUNC, FREE_FUNC_ARG) \
    do { \
        size_t i = 0; \
        for(; i < kv_size(VEC); ++i) { \
            FREE_FUNC(FREE_FUNC_ARG(kv_A(VEC, i))); \
        } \
\
        kv_destroy(VEC); \
        kv_init(VEC); \
    } while(false)

#define FREE_KVEC_VALUE(VEC, FREE_FUNC) FREE_KVEC(VEC, FREE_FUNC, FUNC_ARG_VALUE)

#define FREE_KVEC_ADDR(VEC, FREE_FUNC) FREE_KVEC(VEC, FREE_FUNC, FUNC_ARG_ADDR)

#define COPY_KVEC(TYPE, FREE_FUNC, COPY_FUNC, FUNC_ARG, DEST_VEC, SRC_VEC, RET) \
    do { \
        kv_destroy(DEST_VEC); \
        kv_init(DEST_VEC); \
        kv_resize(TYPE, DEST_VEC, kv_size(SRC_VEC)); \
\
        size_t i = 0; \
        for(; i < kv_max(DEST_VEC); ++i) \
        { \
            (void) kv_a(TYPE, DEST_VEC, i); \
            if(!COPY_FUNC(FUNC_ARG(kv_A(DEST_VEC, i)), FUNC_ARG(kv_A(SRC_VEC, i)))) \
            { break; } \
        } \
\
        if(i < kv_max(DEST_VEC)) \
        { \
            size_t j = 0; \
            for(; j < i; ++j) { \
                FREE_FUNC(FUNC_ARG(kv_A(DEST_VEC, i))); \
            } \
\
            kv_destroy(DEST_VEC); \
            kv_init(DEST_VEC); \
            RET = false; \
            break; \
        } \
\
        RET = true; \
    } while(false)

#define COPY_KVEC_VALUE(TYPE, FREE_FUNC, COPY_FUNC, DEST_VEC, SRC_VEC, RET) \
    COPY_KVEC(TYPE, FREE_FUNC, COPY_FUNC, FUNC_ARG_VALUE, DEST_VEC, SRC_VEC, RET)

#define COPY_KVEC_ADDR(TYPE, FREE_FUNC, COPY_FUNC, DEST_VEC, SRC_VEC, RET) \
    COPY_KVEC(TYPE, FREE_FUNC, COPY_FUNC, FUNC_ARG_ADDR, DEST_VEC, SRC_VEC, RET)

#define COPY_STRING(DEST_STR, SRC_STR) ((DEST_STR) = strdup(SRC_STR))

#define COPY_LOWER_STRING(DEST_STR, SRC_STR) ((DEST_STR) = strdupLower(SRC_STR))

#define kv_move(v1, v0) do {							\
		(v1).m = (v0).m;								\
		(v1).n = (v0).n;								\
		(v1).a = (v0).a;	                            \
		kv_init(v0);		                            \
	} while (0)											
 
#endif // K_UTILS_H
