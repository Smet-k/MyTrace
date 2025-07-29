#include "trace.h"

#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static struct Options parse_options(const uint8_t argc, char* argv[]);

int main(int argc, char* argv[]) {
    const struct Options options = parse_options(argc, argv);

    trace(options);
}

static struct Options parse_options(const uint8_t argc, char* argv[]) {
    int currentOption;
    struct Options options = {
        .destination = NULL,
        .maxTTL = 30,
        .timeout = 2};

    static struct option long_options[] = {
        {"maxttl", required_argument, NULL, 'm'},
        {"timeout", required_argument, NULL, 't'},
        {"help", no_argument, NULL, 'h'}};

    while ((currentOption = getopt_long(argc, argv, "m:t:h", long_options, NULL)) != -1) {
        switch (currentOption) {
            case 'm':
                char* ttlEndPointer;
                const long ttl = strtol(optarg, &ttlEndPointer, 10);

                if (*ttlEndPointer != '\0') {
                    perror(("Invalid TTL value: %s\n", optarg));
                }

                if (ttl < 0 || ttl > UINT8_MAX) {
                    perror(("TTL is out of range(0-255): %s\n", optarg));
                }

                options.maxTTL = (uint8_t)ttl;
                break;
            case 't':
                char* timeoutEndPointer;
                const long timeout = strtol(optarg, &timeoutEndPointer, 10);

                if (*timeoutEndPointer != '\0') {
                    perror(("Invalid Timeout value: %s\n", optarg));
                }

                if (timeout < 0 || timeout > UINT8_MAX) {
                    perror(("Timeout is out of range(0-255): %s\n", optarg));
                }

                options.timeout = timeout;
                break;
            case 'h':
                printf("Usage: sudo .build/myTraceroute [-m maxttl] [-t timeout] destination\n");
                break;
            default:
                printf("Invalid argument. use -h for help.");
                break;
        }
    }

    if (optind == argc) {
        perror("Destination is required!");
    }
    options.destination = argv[optind];

    return options;
}