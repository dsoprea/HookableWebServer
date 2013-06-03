#ifndef __CHEADER_H
#define __CHEADER_h

#include <string>

namespace web
{
    class Header
    {
        std::string name;
        std::string value;

        public:
            Header(std::string name, std::string value);
            ~Header();

            std::string get_name() const { return name; }
            std::string get_value() const { return value; }
    };
}

#endif

