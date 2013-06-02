#ifndef __CHEADER_H
#define __CHEADER_h

#include <string>

using namespace std;

class Header
{
    string name;
    string value;
    
    public:
        Header(string name, string value);
        ~Header();
    
        string get_name() const { return name; }
        string get_value() const { return value; }
};

#endif

