#ifndef UID_GENERATOR_POLICY_RANDOM_HH
# define UID_GENERATOR_POLICY_RANDOM_HH

# include <random>
# include <mutex>
# include <unordered_set>
# include <cstddef>

// The idea of random number is really dumb. But why not?
class UidGeneratorPolicyRandom
{
public:
    std::uint32_t generate_uid()
    {
        std::uint32_t res;

        mutex_.lock();
        do {
            res = rd_();
        } while (map_.find(res) != map_.end());
        map_.insert(res);
        mutex_.unlock();

        return res;
    }

private:
    // It's C++11 but GCC's hash_map could have been used as well.
    std::unordered_set<std::uint32_t> map_;
    std::random_device rd_;
    std::mutex mutex_;
};

#endif // !UID_GENERATOR_POLICY_RANDOM_HH
