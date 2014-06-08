#include <signals.hh>

#include <csignal>

volatile bool stop_requested = false;

static void sigint_handler(int)
{
    stop_requested = true;
}

void register_sighandlers()
{
    // SIGINT
    struct sigaction sigint_sigaction;
    sigint_sigaction.sa_handler = sigint_handler;
    sigemptyset(&sigint_sigaction.sa_mask);
    sigint_sigaction.sa_flags = 0;
    sigaction(SIGINT, &sigint_sigaction, nullptr);
}
