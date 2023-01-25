#include <bitfield.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>

int test_BitField_setUp(void** state)
{
    return 0;
}

int test_BitField_tearDown(void** state)
{
    return 0;
}

void test_BitField_succeeds(void** state)
{
    int32_t res;
    BitField bf;
    initBitField(bf);
    
    BitFieldResize(bf, 80);
    for(size_t i=0;i<90;i++)
    {
        BitFieldHasValue(bf, i, res); // boundary checks without init
        assert_true(res == 0);
    }
    
    BitFieldSet(bf, 7);
    BitFieldSet(bf, 15);
    BitFieldSet(bf, 16);

    // test size change and reallocation
    BitFieldSet(bf, 200);
       
    BitFieldHasValue(bf, 14, res);
    assert_true(res == 0);

    BitFieldHasValue(bf, 15, res);
    assert_true(res != 0);
    
    BitFieldHasValue(bf, 16, res);
    assert_true(res != 0);

    BitFieldHasValue(bf, 17, res);
    assert_true(res == 0);

    BitFieldHasValue(bf, 200, res);
    assert_true(res != 0);

    freeBitField(bf);
}