#ifndef UID_GENERATOR_POLICY_SEQUENTIAL_HH
# define UID_GENERATOR_POLICY_SEQUENTIAL_HH

# include <atomic>

class UidGeneratorPolicySequential
{
public:
    UidGeneratorPolicySequential()
        : next_(0)
    {
    }

    std::uint32_t generate_uid()
    {
        return next_++;
    }

private:
    std::atomic<std::uint32_t> next_;
};

#endif // !UID_GENERATOR_POLICY_SEQUENTIAL_HH
