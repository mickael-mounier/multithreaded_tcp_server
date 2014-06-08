#ifndef ENCODING_POLICY_DECIMAL_HH
# define ENCODING_POLICY_DECIMAL_HH

# include <cstddef> // for std::size_t
# include <cstdint> // for std::uint32_t

// ASCII decimal representation of max uint32 is 10 characters
# define ENCODING_POLICY_OUTPUT_SIZE 10

class EncodingPolicyDecimal
{
public:
    void encode(std::uint32_t input, char output[ENCODING_POLICY_OUTPUT_SIZE]) const
    {
        // Fills ouput with decimal value of uint32 in ASCII padded with zeros.
        memset(output, '0', ENCODING_POLICY_OUTPUT_SIZE);
        output += ENCODING_POLICY_OUTPUT_SIZE;
        for (; input; input /= 10)
            *--output = '0' + input % 10;
    }

    static const std::size_t encode_output_size;
};

const std::size_t EncodingPolicyDecimal::encode_output_size = ENCODING_POLICY_OUTPUT_SIZE;

# undef ENCODING_POLICY_OUTPUT_SIZE

#endif // !ENCODING_POLICY_DECIMAL_HH
