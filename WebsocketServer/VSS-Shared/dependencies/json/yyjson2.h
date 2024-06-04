#pragma once
   
class yyJSONAutoDeleter
{
public:
    yyJSONAutoDeleter(struct yyjson_doc* _yydoc) { yydoc = _yydoc; }
    ~yyJSONAutoDeleter();
private:
    yyjson_doc* yydoc;
};

#define yyJSON(varName) yyjson_doc* ##varName = NULL; yyJSONAutoDeleter deleteDoc( ##varName );
