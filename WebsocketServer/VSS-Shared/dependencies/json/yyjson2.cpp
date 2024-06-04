#include "yyjson2.h"
#include "yyjson.h"

yyJSONAutoDeleter::~yyJSONAutoDeleter()
{
    yyjson_doc_free(yydoc);
}