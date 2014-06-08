#ifndef ENCODING_POLICY_BASE64_HH
# define ENCODING_POLICY_BASE64_HH

# include <cstddef> // for std::size_t
# include <cstdint> // for std::uint32_t

extern "C" {
# include <arpa/inet.h>
}

// base64 representation of 32 bits is 6 characters + 2 of padding
# define ENCODING_POLICY_OUTPUT_SIZE 8

class EncodingPolicyBase64
{
public:
    void encode(std::uint32_t input, char output[ENCODING_POLICY_OUTPUT_SIZE]) const
    {
        input = htonl(input); // To big endian
        unsigned char *data = (unsigned char *)(&input);
        for (int i = 0, j = 0; i < 4;) // 4 == sizeof(std::unit32_t)
        {
            uint32_t octet_a = i < 4 ? data[i++] : 0;
            uint32_t octet_b = i < 4 ? data[i++] : 0;
            uint32_t octet_c = i < 4 ? data[i++] : 0;
            uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
            output[j++] = encoding_table_[(triple >> 3 * 6) & 0x3F];
            output[j++] = encoding_table_[(triple >> 2 * 6) & 0x3F];
            output[j++] = encoding_table_[(triple >> 1 * 6) & 0x3F];
            output[j++] = encoding_table_[(triple >> 0 * 6) & 0x3F];
        }
        // We need 2 "=" for padding.
        output[ENCODING_POLICY_OUTPUT_SIZE - 2] = '=';
        output[ENCODING_POLICY_OUTPUT_SIZE - 1] = '=';
    }

    static const std::size_t encode_output_size;

private:
    static const char *encoding_table_;

};

const std::size_t EncodingPolicyBase64::encode_output_size = ENCODING_POLICY_OUTPUT_SIZE;

const char *EncodingPolicyBase64::encoding_table_ =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

# undef ENCODING_POLICY_OUTPUT_SIZE

#endif // !ENCODING_POLICY_BASE64_HH
