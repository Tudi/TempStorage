#include <string_comparison.h>
#include <strings.h>
#include <stdio.h>

//
// External interface
//

bool assert_string_equal_impl(const char* str1, const char* str2)
{
    if(strcasecmp(str1, str2) != 0)
    {
        printf("Error: assert failed - \"%s\" == \"%s\".", str1, str2);
        return false;
    }

    return true;
}

bool assert_string_not_equal_impl(const char* str1, const char* str2)
{
    if(strcasecmp(str1, str2) == 0)
    {
        printf("Error: assert failed - \"%s\" != \"%s\".", str1, str2);
        return false;
    }

    return true;
}
