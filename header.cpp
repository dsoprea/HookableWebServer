#include "header.h"

namespace web
{
    Header::Header(std::string name_, std::string value_)
    {
        this->name = name_;
        this->value = value_;
    }

    Header::~Header()
    {

    }
}
