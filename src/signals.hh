#ifndef SIGNALS_HH
# define SIGNALS_HH

// No choice but using a global variable for this AFAIK.
extern volatile bool stop_requested;

void register_sighandlers();

#endif // !SIGNALS_HH
