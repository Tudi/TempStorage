#ifndef ID_VALUE_COMPARISON_H
#define ID_VALUE_COMPARISON_H

#include <id_value.h>

void assert_Id_Int32Value_equal(const struct Id_Int32Value* op1, const struct Id_Int32Value* op2);
void assert_Id_StringValue_equal(const struct Id_StringValue* op1,
    const struct Id_StringValue* op2);
void assert_Id_TimeValue_equal(const struct Id_TimeValue* op1, const struct Id_TimeValue* op2);

#endif // ID_VALUE_COMPARISON_H
