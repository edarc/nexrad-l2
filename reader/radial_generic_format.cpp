#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>

#include "endian.hpp"

#include "binary_util.hpp"
#include "radial_generic_format.hpp"

namespace archive2 {

using boost::integer::ubig32_t;
using boost::integer::ubig16_t;
using boost::integer::big16_t;

radial_generic_format::radial_generic_format(const rda_message & rm)
{
    if (rm.message_type != 31)
        throw rda_message::wrong_type();

    std::istringstream iss(rm.payload);
    iss >> *this;
}

std::istream &
operator >> (std::istream & is, radial_generic_format & rgf)
{
    const size_t IDENTIFIER_LEN = 4;

    rgf.radar_identifier = read_string_chunk(is, IDENTIFIER_LEN);

    unsigned long mjd, msec;
    msec = read_binary<ubig32_t, unsigned long>(is);
    mjd  = read_binary<ubig16_t, unsigned long>(is);
    rgf.timestamp = convert_nexrad_mjd(mjd, msec);

    rgf.azimuth_nr = read_binary<ubig16_t, unsigned int>(is);
    rgf.azimuth    = read_binary<ubig32_t, float>(is);

    rgf.compression_indicator = read_binary<unsigned char, unsigned int>(is);
    is.ignore(1); // Spare
    
    unsigned int radial_length = read_binary<ubig16_t, unsigned int>(is);
    radial_length++;

    char azimuth_res_code = read_binary<unsigned char, char>(is);
    switch (azimuth_res_code)
    {
        case 1: rgf.azimuth_res = 0.5; break;
        case 2: rgf.azimuth_res = 1.0; break;
    }

    rgf.radial_status = read_binary<unsigned char, unsigned int>(is);

    rgf.elevation_nr = read_binary<unsigned char, unsigned int>(is);
    rgf.cut_sector_nr = read_binary<unsigned char, unsigned int>(is);
    rgf.elevation = read_binary<ubig32_t, float>(is);

    is.ignore(1); // Spot blanking status (I have no idea what this means)

    rgf.azimuth_indexing = read_binary<unsigned char, float>(is) / 100.0;

    // Here-on reads the data block payloads and constructs moments.
    unsigned int nr_data_blocks = read_binary<ubig16_t, unsigned int>(is);
    std::vector<unsigned long> block_ptrs;
    std::vector<std::string> block_payloads;

    const unsigned int MAX_BLOCK_PTRS = 9;
    unsigned int block_ptr_nr = 0;
    // Read block pointers for as many blocks as are given
    for (; block_ptr_nr != nr_data_blocks;
            ++block_ptr_nr)
        block_ptrs.push_back(read_binary<ubig32_t, unsigned long>(is));

    // Consume empty block pointer slots
    for (; block_ptr_nr != MAX_BLOCK_PTRS;
            ++block_ptr_nr)
        is.ignore(4);

    // Read the block payloads pointed to
    for (std::vector<unsigned long>::iterator it = block_ptrs.begin();
            it != block_ptrs.end();
            ++it)
    {
        if (it+1 != block_ptrs.end())
            // Not the last block pointer: read up to the next pointer
            block_payloads.push_back(read_string_chunk(is, *(it+1) - *it));
        else
            // Last pointer: read till the end of the payload
            block_payloads.push_back(read_string_chunk(is, 0));
    }

    // Attempt to convert each payload into a radial data moment.
    for (std::vector<std::string>::iterator it = block_payloads.begin();
            it != block_payloads.end();
            ++it)
    {
        try
            { rgf.moments.push_back(radial_moment(*it)); }
        catch (radial_moment::invalid_type & e)
        {
            try
                { rgf.vol_constants = volume_constants(*it); }
            catch (volume_constants::invalid_type & e)
                { }
        }
    }

    return is;
}

void
hex_encode(const std::string & in, std::string & out)
{
    out.resize(in.size() * 2);
    std::auto_ptr<char> out_auto(new char[out.size()]);
    char * out_c = out_auto.get();

    std::string::const_iterator in_it = in.begin();
    for (; in_it != in.end();
            ++in_it)
    {
        *out_c = ((*in_it & 0xF0) >> 8) + '0';
        *out_c = (*out_c > '9') ? *out_c += ('A'-'9'-1)
                                : *out_c;
        ++out_c;

        *out_c = (*in_it & 0x0F) + '0';
        *out_c = (*out_c > '9') ? *out_c += ('A'-'9'-1)
                                : *out_c;
        ++out_c;
    }
    std::copy(out_auto.get(), out_auto.get() + in.size() * 2, out.begin());
}

std::ostream &
operator << (std::ostream & os, const radial_generic_format & rgf)
{
    using std::endl;
    using boost::format;

    bt::time_facet * fmt = new bt::time_facet("%T.%f");
    os.imbue(std::locale(os.getloc(), fmt));

    os << "RAD " << rgf.timestamp 
       << format(" %|+02.5| %|03.5| %|.5|")
           % rgf.elevation
           % rgf.azimuth 
           % rgf.azimuth_res
           << '\n';
       
    for (std::vector<radial_moment>::const_iterator it = rgf.moments.begin();
            it != rgf.moments.end();
            ++it)
    {
        os << format("%|1| %|02.5| %|.5| %|+03.5| %|03.5|")
            % it->moment_type
            % it->start_range
            % it->range_res
            % it->scale
            % it->offset
            << '\n';

        const int GATES_PER_LINE = 40;

        std::string hex_out;
        hex_encode(it->gates, hex_out);

        for (unsigned int slice_counter = 0;
                slice_counter * 2 < hex_out.length();
                slice_counter += GATES_PER_LINE)
        {
            std::string hex_line(hex_out, slice_counter * 2,
                    GATES_PER_LINE * 2);
            os.write(hex_line.c_str(), hex_line.length());
            os.put('\n');
        }
    }

    os << std::flush;
    return os;
}

radial_moment::radial_moment(const std::string & str)
{
    if (str[0] != 'D')
        throw invalid_type();
    
    std::istringstream iss(str);
    iss.ignore(1);
    moment_type = read_string_chunk(iss, 3);
    iss.ignore(4);

    nr_gates    = read_binary<ubig16_t, unsigned int>(iss);
    start_range = static_cast<float>(read_binary<ubig16_t, int>(iss)) / 1000.0;
    range_res   = static_cast<float>(read_binary<ubig16_t, int>(iss)) / 1000.0;
    
    iss.ignore(6);

    scale  = read_binary<ubig32_t, float>(iss);
    offset = read_binary<ubig32_t, float>(iss);

    const size_t DATA_MOMENTS_OFFSET = 28;
    gates.assign(str, DATA_MOMENTS_OFFSET, nr_gates);
}

volume_constants::volume_constants(const std::string & str)
{
    if (str.substr(0, 4) != "RVOL")
        throw invalid_type();

    std::istringstream iss(str);
    iss.ignore(8);

    latitude  = read_binary<ubig32_t, float>(iss);
    longitude = read_binary<ubig32_t, float>(iss);

    const int site_elevation  = read_binary<big16_t, int>(iss);
    const int feedhorn_height = read_binary<ubig16_t, int>(iss);
    geo_elevation = site_elevation + feedhorn_height;
    
    iss.ignore(20);

    vcp = read_binary<ubig16_t, int>(iss);
}

} // namespace archive2
