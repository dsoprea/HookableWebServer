#include "string_util.h"

int _strpos(char *haystack, const char *needle, int start_at)
{
    char *found_at;
    if((found_at = strstr(&haystack[start_at], needle)) == NULL)
        return -1;
    
    return (found_at - haystack);
}

char *_strdup(char *from, int count)
{
    if(count == 0)
        count = strlen(from);

    char *copy = (char *)malloc(count + 1);
    memcpy(copy, from, count);
    copy[count] = 0;
    
    return copy;
}

