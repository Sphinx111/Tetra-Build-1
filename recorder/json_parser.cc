#include "json_parser.h"


json_parser_t::json_parser_t(std::string data)
{
    // constructor
    b_valid = true;

    if (data == "")
    {
        b_valid = false;
    }
    else if (jdoc.Parse(data.c_str()).HasParseError())                          // parse and check parsing result for errors
    {
        b_valid = false;
        fprintf(stderr, "\nError(offset %u): %s\n'%s'\n",
                (unsigned)jdoc.GetErrorOffset(),
                rapidjson::GetParseError_En(jdoc.GetParseError()),
                data.c_str());
    }
}


json_parser_t::~json_parser_t()
{
    // destructor
}


bool json_parser_t::is_valid()
{
    return b_valid;
}


bool json_parser_t::read(const std::string field, std::string * result)
{
    // read field with error check
    bool ret = false;

    *result = "";                                                               // for sanity

    if (b_valid)                                                                // check if document is valid
    {
        if (jdoc.HasMember(field.c_str()))                                      // check if document has member field
        {
            if (jdoc[field.c_str()].IsString())                                 // check member field type
            {
                std::string txt(jdoc[field.c_str()].GetString());
                *result = txt;
                ret = true;
            }
        }
    }

    return ret;
}


bool json_parser_t::read(const std::string field, uint8_t * result)
{
    // read field with error check
    uint64_t val;

    bool ret = read(field, &val);

    *result = (uint8_t)val;

    return ret;
}


bool json_parser_t::read(const std::string field, uint16_t * result)
{
    // read field with error check
    uint64_t val;

    bool ret = read(field, &val);

    *result = (uint16_t)val;

    return ret;
}


bool json_parser_t::read(const std::string field, uint32_t * result)
{
    // read field with error check
    uint64_t val;

    bool ret = read(field, &val);

    *result = (uint32_t)val;

    return ret;
}


bool json_parser_t::read(const std::string field, uint64_t * result)
{
    // read field with error check
    bool ret = false;

    *result = 0;                                                                // for sanity

    if (b_valid)                                                                // check if document is valid
    {
        if (jdoc.HasMember(field.c_str()))                                      // check if document has member field
        {
            if (jdoc[field.c_str()].IsNumber())                                 // check member field type
            {
                uint64_t val = jdoc[field.c_str()].GetUint64();
                *result = val;
                ret = true;
            }
        }
    }

    return ret;
}


void json_parser_t::write_report(FILE * fd)
{
    if (b_valid)
    {
        rapidjson::StringBuffer buffer(0, 8192);                                // size of buffer is automatically increased
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        jdoc.Accept(writer);

        const char * buf = buffer.GetString();
        std::string txt = buf;

        fprintf(fd, "%s\n", txt.c_str());
        fflush(fd);
    }
}


std::string json_parser_t::to_string()
{
    std::string txt = "";

    if (b_valid)
    {
        rapidjson::StringBuffer buffer(0, 8192);                                // size of buffer is automatically increased
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        jdoc.Accept(writer);

        const char * buf = buffer.GetString();
        txt = buf;
    }

    return txt;
}
