#ifndef SERVER_HH
# define SERVER_HH

# include <cstddef>
# include <cstdint>
# include <atomic>
# include <list>

# include <worker.hh>

template <typename UidGenerationPolicy, typename EncodingPolicy>
class Worker;

template <typename UidGenerationPolicy, typename EncodingPolicy>
class Server : public UidGenerationPolicy, public EncodingPolicy
{
public:
    typedef Server<UidGenerationPolicy, EncodingPolicy> self_type;
    typedef Worker<UidGenerationPolicy, EncodingPolicy> worker_type;

    Server(std::uint16_t port);
    ~Server(); // No virtual needed since no inheritance as of now.

    void run();

    using UidGenerationPolicy::generate_uid;
    using EncodingPolicy::encode;

    volatile std::atomic<unsigned int> threads_counter;

protected:
    bool init_();
    void cleanup_();

    const std::uint16_t port_;
    std::list<worker_type *> workers_; // One worker per client
    int socket_;
};

# include <server.hxx>

#endif // !SERVER_HH
