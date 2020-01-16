#include <stdlib.h>
#include <string.h>

#include "utils.h"

static int parse_long(const char *str, long int *result)
{
    char *end;
    long int val;

    val = strtol(str, &end, 0);

    if (*str == '\0' || *end != '\0') {
        return -EINVAL;
    }

    *result = val;
    return 0;
}


 int parse_u32(const char *str, u32_t *result)
{
    long val;

    if (parse_long(str, &val) || val > UINT32_MAX || val < 0 ) 
    {
        return -EINVAL;
    }
    *result = (u32_t)val;
    return 0;
}

 int parse_i32(const char *str, int32_t *result)
{
    long val;

    if (parse_long(str, &val) || val < INT32_MIN || val > INT32_MAX ) 
    {
        return -EINVAL;
    }
    *result = (int32_t)val;
    return 0;
}


