#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>

#include <config.hh>

#include <server.hh>
#include <uid_generator_policy_sequential.hh>
#include <uid_generator_policy_random.hh>
#include <encoding_policy_decimal.hh>
#include <encoding_policy_base64.hh>

void usage(bool error = false)
{
    if (error)
        std::cerr << "error: invalid arguments" << std::endl;
    std::cout << "server [OPTION]" << std::endl
              << "    -p PORT    the listening TCP port" << std::endl
              << "    -i METHOD  the uid generation method (sequential or random)" << std::endl
              << "    -e METHOD  the ASCII encoding method (decimal or base64)" << std::endl
              << "    -h         show this help" << std::endl
              << "    -v         display version" << std::endl;
    std::exit(!!error);
}

void version()
{
    std::cout << PACKAGE_NAME << " server " << PACKAGE_VERSION
              << " by " << PACKAGE_BUGREPORT << std::endl;
    std::exit(0);
}

int parse_int(char *str)
{
    try
    {
        // FIXME: atoi is bad since parse_int("100abc") will return
        // 100 instead of an error
        return std::atoi(str);
    }
    catch (std::invalid_argument e)
    {
        usage(true);
    }
    return 0;
}

int main(int argc, char **argv)
{
    // Default parameters
    int port = 7684;
    std::string uid_generator = "sequential";
    std::string encoder = "decimal";

    // Arguments parsing
    char c = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (c == 0)
        {
            if (!std::strcmp(argv[i], "-h"))
                usage();
            else if (!std::strcmp(argv[i], "-v"))
                version();
            else if (!std::strcmp(argv[i], "-p"))
                c = 1;
            else if (!std::strcmp(argv[i], "-i"))
                c = 2;
            else if (!std::strcmp(argv[i], "-e"))
                c = 3;
            else
                usage(true);
        }
        else if (c == 1) // -p
        {
            port = parse_int(argv[i]);
            if (port < 1 || port > 65535)
            {
                std::cerr << "error: invalid port number" << std::endl;
                std::exit(1);
            }
            c = 0;
        }
        else if (c == 2) // -i
        {
            uid_generator = argv[i];
            if (uid_generator != "sequential" && uid_generator != "random")
            {
                std::cerr << "error: invalid uid generator name" << std::endl;
                std::exit(1);
            }
            c = 0;
        }
        else if (c == 3) // -e
        {
            encoder = argv[i];
            if (encoder != "decimal" && encoder != "base64")
            {
                std::cerr << "error: invalid encoder name" << std::endl;
                std::exit(1);
            }
            c = 0;
        }
    }
    // Still expecting an argument?
    if (c != 0)
        usage(true);

    // Registrering signal handlers for main thread
    register_sighandlers();

    // Launching the server
    // FIXME: this is ugly, it can be improved
    if (uid_generator == "sequential" && encoder == "decimal")
        Server<UidGeneratorPolicySequential, EncodingPolicyDecimal>(port).run();
    else if (uid_generator == "sequential" && encoder == "base64")
        Server<UidGeneratorPolicySequential, EncodingPolicyBase64>(port).run();
    else if (uid_generator == "random" && encoder == "decimal")
        Server<UidGeneratorPolicyRandom, EncodingPolicyDecimal>(port).run();
    else if (uid_generator == "random" && encoder == "base64")
        Server<UidGeneratorPolicyRandom, EncodingPolicyBase64>(port).run();

    return 0;
}
