#ifndef SERVER_HXX
# define SERVER_HXX

# include <server.hh>
# include <worker.hh>

# include <iostream>
# include <cerrno>
# include <ctime>

extern "C" {
# include <pthread.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <fcntl.h>
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
Server<UidGenerationPolicy, EncodingPolicy>::Server(unsigned short port)
    : threads_counter(0)
    , port_(port)
    , workers_()
    , socket_(-1)
{
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
Server<UidGenerationPolicy, EncodingPolicy>::~Server()
{
    for (auto i = workers_.begin(); i != workers_.end(); ++i)
        delete *i;
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
bool Server<UidGenerationPolicy, EncodingPolicy>::init_()
{
    if ((socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        std::cerr << "error: cannot create socket" << std::endl;
        return false;
    }

    int yes = 1;
    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)))
    {
        std::cerr << "error: unable to set socket option" << std::endl;
        return false;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    if (bind(socket_, (sockaddr *)(&address), sizeof(sockaddr_in)) < 0)
    {
        std::cerr << "error: cannot bind socket to port " << port_ << std::endl;
        return false;
    }

    if (listen(socket_, port_) < 0)
    {
        std::cerr << "error: cannot listen on port " << port_ << std::endl;
        return false;
    }

    std::cout << "listening on port " << port_ << std::endl;

    return true;
}

template <typename UidGenerationPolicy, typename EncodingPolicy>
void Server<UidGenerationPolicy, EncodingPolicy>::cleanup_()
{
    if (socket_ >= 0)
    {
        close(socket_);
        socket_ = -1;
    }

    // Waiting for threads to exit
    timespec sleep_time;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = 10000000; // 10ms

    // This shouldn't be necessary, but just in case...
    stop_requested = true;

    while (threads_counter > 0)
    {
        nanosleep(&sleep_time, nullptr);
    }
}


template <typename UidGenerationPolicy, typename EncodingPolicy>
void Server<UidGenerationPolicy, EncodingPolicy>::run()
{

    if (!init_())
    {
        cleanup_();
        return;
    }

    for (;;)
    {
        int worker_socket = accept(socket_, nullptr, nullptr);
        if (worker_socket < 0 || stop_requested)
        {
            if (errno == EINTR)
            {
                std::cout << "stopping server, waiting for threads to terminate" << std::endl;
                break;
            }
            else
            {
                std::cerr << "error: unable to accept client" << std::endl;
            }
        }
        else
        {
            std::cout << "new client (total = " << (threads_counter + 1) << ")" << std::endl;
            worker_type * worker = new worker_type(*this, worker_socket);
            workers_.push_back(worker);

            pthread_t thread;
            if (pthread_create(&thread, 0, &worker_type::pthread_helper, worker))
            {
                std::cerr << "error: unable to create thread" << std::endl;
                continue;
            }
            if (pthread_detach(thread))
            {
                std::cerr << "error: unable to detach thread" << std::endl;
                continue;
            }
        }
    }

    cleanup_();
}

#endif // !SERVER_HXX
