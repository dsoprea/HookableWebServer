#ifndef __CHEADER_H
#define __CHEADER_h

#include <stdlib.h>
#include <string.h>

#include "string_util.h"

class CHeader
{
    char *name;
    char *value;
    
    public:
        CHeader(char *name, char *value);
        ~CHeader();
    
        char *get_name() const { return name; }
        char *get_value() const { return value; }
};

#endif

