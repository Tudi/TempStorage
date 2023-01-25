#include <boolean_parser.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

int test_boolean_parser_setUp(void** state)
{
    return 0;
}

int test_boolean_parser_tearDown(void** state)
{
    return 0;
}

void test_boolean_parser_succeeds(void** state)
{
    int32_t res;
    res = calculate_boolean_expression("1|0", NULL);
    assert_true(res == 1);

    res = calculate_boolean_expression("0|1", NULL);
    assert_true(res == 1);

    res = calculate_boolean_expression("0|0", NULL);
    assert_true(res == 0);

    res = calculate_boolean_expression("1|1", NULL);
    assert_true(res == 1);

    res = calculate_boolean_expression("1&1", NULL);
    assert_true(res == 1);

    res = calculate_boolean_expression("0&1", NULL);
    assert_true(res == 0);

    res = calculate_boolean_expression("1&0", NULL);
    assert_true(res == 0);

    res = calculate_boolean_expression("0&0", NULL);
    assert_true(res == 0);

    res = calculate_boolean_expression("1&0|1", NULL);
    assert_true(res == 1);

    res = calculate_boolean_expression("(1&0)|1", NULL);
    assert_true(res == 1);

    res = calculate_boolean_expression("(1&0)&1", NULL);
    assert_true(res == 0);

    res = calculate_boolean_expression("0&(0|1)", NULL);
    assert_true(res == 0);

    res = calculate_boolean_expression("(0&0)|1", NULL);
    assert_true(res == 1);

    res = calculate_boolean_expression("!0&0", NULL);
    assert_true(res == 0);

    res = calculate_boolean_expression("!(0&0)", NULL);
    assert_true(res == 1);

    res = calculate_boolean_expression("(1&0)&(0|1)", NULL);
    assert_true(res == 0);

    res = calculate_boolean_expression("(1&0&0)|1", NULL);
    assert_true(res == 1);

    res = calculate_boolean_expression("1|1&0", NULL);
    assert_true(res == 0);

    res = calculate_boolean_expression("0&1|1", NULL);
    assert_true(res == 1);

    res = calculate_boolean_expression("1|1&0|1&0", NULL);
    assert_true(res == 0);
    
    char variables[256];
    variables[0] = 1;
    variables[1] = 1;
    res = calculate_boolean_expression("a|b", variables);
    assert_true(res == 1);

    variables[0] = 1;
    variables[1] = 0;
    res = calculate_boolean_expression("a|b", variables);
    assert_true(res == 1);

    variables[0] = 0;
    variables[1] = 0;
    res = calculate_boolean_expression("a|b", variables);
    assert_true(res == 0);

    variables[0] = 1;
    variables[1] = 1;
    res = calculate_boolean_expression("a&b", variables);
    assert_true(res == 1);

    variables[0] = 1;
    variables[1] = 0;
    res = calculate_boolean_expression("a&b", variables);
    assert_true(res == 0);

    variables[0] = 0;
    variables[1] = 0;
    res = calculate_boolean_expression("a&b", variables);
    assert_true(res == 0);
    
    res = calculate_boolean_expression("1&!0", variables);
    assert_true(res == 1);
    
}