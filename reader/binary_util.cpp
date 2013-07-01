#include <iostream>
#include <string>

#include "binary_util.hpp"

namespace archive2 {

//
// Read exactly len bytes from the stream into the given string.
//
void
read_string_chunk(std::istream & is, size_t len, std::string & out)
{
    const size_t MAX_UNBOUNDED_READ = 20000;

    if (len == 0)
        len = MAX_UNBOUNDED_READ;

    std::auto_ptr<char> cstr(new char[len]);
    is.read(cstr.get(), len);

    out.resize(len);
    std::copy(cstr.get(), cstr.get() + len, out.begin());
}

//
// Read exactly len bytes from the stream and return them as a string.
//
std::string
read_string_chunk(std::istream & is, size_t len)
{
    std::string out;
    read_string_chunk(is, len, out);
    return out;
}

template <>
float
read_binary<ubig32_t, float>(std::istream & is)
{
    ubig32_t read_value(0);
    is.read(reinterpret_cast<char *>(&read_value), sizeof(read_value));

    union {
        uint32_t i;
        float    f;
    } converter;

    converter.i = read_value;
    return converter.f;
}

} // namespace archive2
