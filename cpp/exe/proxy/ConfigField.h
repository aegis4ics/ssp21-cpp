#ifndef SSP21PROXY_CONFIGFIELD_H
#define SSP21PROXY_CONFIGFIELD_H

#include "openpal/util/Uncopyable.h"

#include "SectionException.h"

template <class T>
class ConfigField : openpal::Uncopyable
{
public:

    ConfigField(const char* key_name) : key_name(key_name)
    {}

    bool is_defined() const
    {
        return defined;
    }

    inline const T& get(const std::string& section) const
    {
        if (!defined)
        {
            throw SectionException(section, "value not defined: ", key_name);
        }
        return value;
    }

    void set(const T& value, const std::string& section)
    {
        if (defined)
        {
            throw SectionException(section, "value already defined: ", key_name);
        }
        defined = true;
        this->value = value;
    }

    const char* const key_name;

private:

    T value;
    bool defined = false;
};

#endif
