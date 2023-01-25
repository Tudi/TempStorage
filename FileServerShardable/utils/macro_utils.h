#ifndef MACRO_UTILS_H
#define MACRO_UTILS_H

#define ADD_STRING_QUOTES(x) #x
#define MACRO_TO_STRING(x) ADD_STRING_QUOTES(x)

#define ARRAY_COUNT(ARR) (sizeof(ARR)/sizeof(ARR[0]))

#define FUNC_ARG_VALUE(VALUE) (VALUE)
#define FUNC_ARG_ADDR(VALUE) (&VALUE)

#define DUMMY_INIT(VAL) ((void) (VAL))
#define DUMMY_FREE(VAL) ((void) (VAL))

#endif // MACRO_UTILS_H
