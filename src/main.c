#include <stdio.h>
#include <arpa/inet.h>
#include <trace.h>
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("usage: %s destination_ip\n", argv[0]);
        return 1;
    }
    struct in_addr dst;

    if (inet_aton(argv[1], &dst) == 0) {
        perror("inet_aton");
        printf("%s isn't a valid IP address\n", argv[1]);
        return 1;
    }

    trace(&dst);
}
