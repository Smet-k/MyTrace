#include <stdio.h>
#include <arpa/inet.h>
#include <trace.h>
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("usage: %s destination_ip\n", argv[0]);
        return 1;
    }

    trace(argv[1]);
}
