#include "cheader.h"

CHeader::CHeader(char *name, char *value)
{
    this->name = _strdup(name);
    this->value = _strdup(value);
}

CHeader::~CHeader()
{
    free(name);
    free(value);
}

