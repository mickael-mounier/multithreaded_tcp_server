#ifndef WORKER_HXX
# define WORKER_HXX

# include <worker.hh>
# include <server.hh>
# include <signals.hh>

# include <ctime>
# include <string>
# include <iomanip>

extern "C" {
# include <sys/socket.h>
# include <unistd.h>
# include <fcntl.h>
}

namespace {
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
Worker<UidGenerationPolicy, EncodingPolicy>::Worker(server_type &server, int socket)
    : server_(server)
    , socket_(socket)
    , uid_(0)
    , uid_len_(0)
    , next_tick_()
{
    uid_len_ = server_.encode_output_size + 1;
    uid_ = new char[uid_len_ + 1];
    uid_[uid_len_ - 1] = '\n';
    uid_[uid_len_] = '\0';
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
Worker<UidGenerationPolicy, EncodingPolicy>::~Worker()
{
    delete[] uid_;
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
void Worker<UidGenerationPolicy, EncodingPolicy>::print_time_(const char *msg) const
{
    timespec t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    std::cout << t.tv_sec << "." << std::setfill('0') << std::setw(9) << t.tv_nsec
              << ":" << socket_
              << ": " << msg << std::endl;
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
void Worker<UidGenerationPolicy, EncodingPolicy>::init_()
{
    print_time_("start");
    ++server_.threads_counter;
    // We want non-blocking reads
    fcntl(socket_, F_SETFL, fcntl(socket_, F_GETFL, 0) | O_NONBLOCK);
    // The store the start time and compute the id to be sent in one second
    clock_gettime(CLOCK_MONOTONIC_RAW, &next_tick_);
    ++next_tick_.tv_sec; // We do not tick on start, next tick in 1s.
    precompute_uid_();
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
void Worker<UidGenerationPolicy, EncodingPolicy>::exit_()
{
    close(socket_);
    --server_.threads_counter;
    std::cerr << "client lost (total = " << server_.threads_counter << ")" << std::endl;
    pthread_exit(nullptr);
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
void Worker<UidGenerationPolicy, EncodingPolicy>::precompute_uid_()
{
    server_.encode(server_.generate_uid(), uid_);
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
void Worker<UidGenerationPolicy, EncodingPolicy>::run()
{
    init_();

    long next_tick_ns;
    timespec current_time;
    timespec sleep_time;
    sleep_time.tv_sec = 0; // We will always sleep for less than a second

    char buffer[1024];

    for (;;)
    {
        int read_size;
        while ((read_size = read(socket_, buffer, sizeof(buffer))) > 0)
        {
            for (int i = 0; i < read_size; ++i)
                if (buffer[i] == '\n')
                {
                    std::string response = std::to_string(server_.threads_counter) + "\n";
                    if (write(socket_, response.c_str(), response.size()) < 0)
                    {
                        std::cerr << "error: unable to write socket " << socket_ << std::endl;
                        exit_();
                    }
                }
        }
        if (read_size == 0)
        {
            std::cout << "client disconnected" << std::endl;
            exit_();
        }
        if (stop_requested)
        {
            if (write(socket_, "Bye\n", 4) < 0)
                std::cerr << "error: unable to write socket " << socket_ << std::endl;
            exit_();
        }

        // Computing time until next tick
        clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
        next_tick_ns = next_tick_.tv_nsec - current_time.tv_nsec;
        if (next_tick_ns < 0)
            next_tick_ns += 1000000000; // +1s

        if (next_tick_ns > 150000000) // next tick > 150ms
        {
            // Sleep for 100ms
            sleep_time.tv_nsec = 100000000;
            nanosleep(&sleep_time, nullptr);
        }
        else // next tick <= 150ms
        {
            // Sleep the necassry time minus 3ms
            sleep_time.tv_nsec = next_tick_ns - 3000000;
            if (sleep_time.tv_nsec > 0)
                nanosleep(&sleep_time, nullptr);

            // Busy-waiting
            clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
            while (current_time.tv_nsec < next_tick_.tv_nsec ||
                   current_time.tv_sec  < next_tick_.tv_sec)
            {
                clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
            }

            // Time to send!
            if (write(socket_, uid_, uid_len_) < 0)
            {
                std::cerr << "error: unable to write socket " << socket_ << std::endl;
                exit_();
            }
            print_time_(std::string(uid_).substr(0, uid_len_ - 1).c_str()); // Dirty trick to avoid double '\n'
            precompute_uid_();
            ++next_tick_.tv_sec;
        }
    }
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
void *Worker<UidGenerationPolicy, EncodingPolicy>::pthread_helper(void * worker)
{
    register_sighandlers();
    static_cast<self_type *>(worker)->run();
    return nullptr;
}

#endif // !WORKER_HXX
