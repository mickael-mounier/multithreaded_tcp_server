========================
Multithreaded TCP server
========================

This is a multithreaded C++ server I did for a job interview.


Assignment
==========

Write a server in C or C++ language serving arbitrary number of TCP
clients.

 - For every TCP client connected a new thread in the server will be
   spawned. The thread will send a server-wide unique binary 32 bit ID
   number every second. The ID will be ASCII coded before sending to
   client and terminated by a new line character. The mechanism for
   finding unique IDs is performance sensitive and cannot be
   pre-computed.

 - The server also responds new line character received from the
   client with number of clients connected. Other characters received
   are ignored.

 - Do use only basic system libraries (no zmq, boost); STL is allowed.

 - Ctrl-C will send "Bye\n" to all the connected clients and
   immediately terminate the server cleanly.

 - Limit the usage of global variables.


Notes
=====

General
-------

The project has been written in C++11 language using the GNU autotools
build system. To respect the assignment, no major feature from C++11
has been used (i.e. things coming from the boost library like
threading, networking and such).

Policy-based class design for the Server and Worker classes have been
used to maximise performances while keeping the flexibility over the
unique ID computation and ASCII encoding.

std::atomic has been used in this code. It's a C++11 feature but it
can be easily replaced either by a non-portable solution like GCC's
__sync_add_and_fetch or by protecting the code with mutexes.

No mutexes have been used except in uid_generator_policy_random to
lock a set.


Main Thread
-----------

The main thread is using a blocking socket to accept connections. It
uses a thread counter that can indicate the number of connected
clients. This counter is also used when the server receives a SIGINT:
we have an infinite loop waiting for the counter to reach 0.


Client Threads
--------------

I call here a tick the time we need to send the unique ID to the
client (every seconds since client's connection).

The client thread is using a non-blocking socket trying to read every
100ms if a '\n' has been received. When it comes around 150ms to the
next tick, it will sleep for the necessary time (between 50ms and
150ms) in order to send the next ID right on time.

Note that the sending of the ID can be delayed if the client sends a
lot of data for some reason since I used a "while read" loop to read
incoming data. This is easily fixable.

For time management clock_gettime has been used with the
CLOCK_MONOTONIC_RAW parameter as well as the nanosleep function. Since
an actual vanilla Linux kernel has a jiffy around 1ms I decided to
implement busy waiting for the last 3ms.

This way I managed to achieve a precision around the microsecond on a
vanilla Linux 3.14 kernel by running the process with high priority.

To avoid busy-sleep with many system calls, busy CPU and improved
accuracy it will be necessary to use and tweak the kernel's scheduler,
or even better: use a real-time Linux kernel.


ID Generation
-------------

The default ID generator is just a counter starting from 0. I wasn't
really sure if that was what was expected so I made another based on
random numbers (but it ended up pretty dumb).

You can switch from one to another using the -i parameter, the allowed
values are "sequential" (default) and "random".


ASCII encoding
--------------

Here again I wasn't really sure of what was expected so I made two
different encoders that can be swiched using the -e argument. The
allowed values are "decimal" (default) and "base64".


Files
=====

In ./src:

 - main.cc: the entry point with arguments parsing
 - server.*: the server class (i.e. the main thread)
 - worker.*: the worker class (i.e. the client threads)
 - signal.*: SIGINT handling and the stop_requested global variable
 - ui_generator_*: the UID generators
 - encoding_*: the ASCII encoders


Building and running the server
===============================

Nothing fancy here, typical GNU build sytem:

<code>
$ autoreconf --install
$ mkdir build
$ cd build
$ ../configure
$ make
</code>

The program will be created in the build/src directory.

There are four different combination for the actual output to clients:

<code>
$ ./server -e decimal -i sequential : sending incremental numbers in decimal
$ ./server -e base64 -i sequential  : sending incremental numbers in base64
$ ./server -e decimal -i random     : sending random numbers in decimal
$ ./server -e base64 -i random      : sending random numbers in base64
</code>

Dependencies
============

The project has been tested using :
 - gcc 4.9.0
 - automake 1.14.1
 - autoconf 2.69
 - make 4.0
