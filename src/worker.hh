#ifndef WORKER_HH
# define WORKER_HH

template <typename UidGenerationPolicy, typename EncodingPolicy>
class Server;

template <typename UidGenerationPolicy, typename EncodingPolicy>
class Worker
{
public:
    typedef Worker<UidGenerationPolicy, EncodingPolicy> self_type;
    typedef Server<UidGenerationPolicy, EncodingPolicy> server_type;

    Worker(Worker::server_type &server, int socket);
    ~Worker(); // No virtual needed since no inheritance as of now.

    void run();

    static void *pthread_helper(void * worker);

protected:
    void init_();
    void exit_();
    void precompute_uid_();
    void send_uid_();
    void print_time_(const char *msg) const;

    server_type &server_;
    int socket_;
    char *uid_; // Where the precomputed uid is stored.
    std::size_t uid_len_;
    timespec next_tick_;

private:
    Worker(const self_type &); // No copies!
};

# include <worker.hxx>

#endif // !WORKER_HH
