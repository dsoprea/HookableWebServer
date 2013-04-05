#include <stdlib.h>
#include <string.h>

template <class T>
class DString
{
    char *buffer;
    int _length;
    
    public:
        DString();
        ~DString();
        DString(const DString<char> &from, int count=0);
        DString(char *from, int count=0);

        int append(DString from);
        int find(char *needle, int start_at=0);
        vector<DString> split(char *separator, int max_splits=0);
        DString substr(int pos, int count=0);
        int length();
        char *get_char_array() const;
};

template <>
class DString <char>
{
    char *buffer;
    int _length;
    
    public:
        DString();
        ~DString();
        DString(const DString<char> &from, int count=0);
        DString(char *from, int count=0);

        int append(DString from);
        int find(char *needle, int start_at=0);
        vector<DString> split(char *separator, int max_splits=0);
        DString substr(int pos, int count=0);
        int length();
        char *get_char_array() const;
};

template <class T>
DString<T>::DString()
{
    buffer = NULL;
    _length = -1;
}

template <class T>
DString<T>::~DString()
{
    if(buffer != NULL)
        free(buffer);
}

template <class T>
DString<T>::DString(const DString<char> &from, int count)
{
    DString(from.get_char_array(), count);
}

template <class T>
DString<T>::DString(char *from, int count)
{
    _length = (count == 0 ? strlen(from) : count);
    buffer = malloc(_length);
    
    memcpy(buffer, from, _length);
    buffer[_length] = 0;
// TODO: Assert.

}

template <class T>
char *DString<T>::get_char_array() const
{
    return buffer;
}

