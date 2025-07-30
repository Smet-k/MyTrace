#include "trace.h"

#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PACKET_SIZE 64
#define WORD_LENGTH_IN_BYTES 16

static void send_packet(const int sock, const struct icmphdr* icmp_hdr, const struct sockaddr_in* destAddr);
static void recv_packet(const int sock, char* packet, struct sockaddr_in* replyAddr);
static int create_socket(struct Options options);
static uint16_t calculate_checksum(void* buffer, uint16_t length);
static void fill_header(struct icmphdr* icmph, const uint8_t ttl);
static struct sockaddr_in resolve_host(const char* dst);
static double calc_time_diff_ms(struct timeval *start, struct timeval *end);

void trace(struct Options options) {
    int sock = create_socket(options);
    struct icmphdr icmp_hdr;
    struct sockaddr_in destAddr = resolve_host(options.destination);
    struct timeval startTime, endTime;
    for (uint8_t ttl = 1; ttl <= options.maxTTL; ttl++) {
        struct sockaddr_in replyAddr;
        char packet[PACKET_SIZE];

        fill_header(&icmp_hdr, ttl);
        setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        gettimeofday(&startTime, NULL);
        send_packet(sock, &icmp_hdr, &destAddr);
        recv_packet(sock, packet, &replyAddr);
        gettimeofday(&endTime, NULL);

        printf("%3d\t%-15s\t%3.fms\n",ttl ,inet_ntoa(replyAddr.sin_addr), calc_time_diff_ms(&startTime, &endTime));
        if (replyAddr.sin_addr.s_addr == destAddr.sin_addr.s_addr) {
            break;
        }
    }
    close(sock);
}

static void fill_header(struct icmphdr* icmph, const uint8_t ttl) {
    if (icmph == NULL) {
        perror("ICMP header pointer is null");
    }

    memset(icmph, 0, sizeof(*icmph));

    icmph->type = ICMP_ECHO;
    icmph->code = 0;
    icmph->un.echo.id = (uint16_t)getpid();
    icmph->un.echo.sequence = ttl;
    icmph->checksum = calculate_checksum(icmph, sizeof(*icmph));
}

static uint16_t calculate_checksum(void* buffer, uint16_t length) {
    uint16_t* wordPointer = buffer;
    uint32_t sum = 0;

    for (sum = 0; length > 1; length -= 2) {
        sum += *wordPointer++;
    }

    if (length == 1) {
        sum += *(uint8_t*)wordPointer;
    }

    sum = (sum >> WORD_LENGTH_IN_BYTES) + (sum & 0xFFFF);
    sum += sum >> WORD_LENGTH_IN_BYTES;

    return (uint16_t)~sum;
}

static struct sockaddr_in resolve_host(const char* dest) {
    struct addrinfo hints = {0}, *result;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(dest, NULL, &hints, &result) != 0) {
        perror("Failed to resolve host");
    }

    const struct sockaddr_in destAddr = *(struct sockaddr_in*)result->ai_addr;
    freeaddrinfo(result);

    return destAddr;
}

static void send_packet(const int sock, const struct icmphdr* icmp_hdr, const struct sockaddr_in* destAddr) {
    int pack_bytes = sendto(sock, icmp_hdr, sizeof(*icmp_hdr), 0,
                            (struct sockaddr*)destAddr, sizeof(*destAddr));

    if (pack_bytes < 0) {
        perror("Sendto");
        return;
    }
}

static void recv_packet(const int socketFileDescriptor, char* packet, struct sockaddr_in* replyAddress) {
    socklen_t addrLen = sizeof(*replyAddress);
    if (recvfrom(socketFileDescriptor, packet, PACKET_SIZE, 0, (struct sockaddr*)replyAddress,
                 &addrLen) == -1) {
        perror("Recvfrom");
    }
}

static int create_socket(struct Options options) {
    const int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1) {
        perror("Socket creation failed");
    }

    const struct timeval timeout = {options.timeout, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    return sock;
}

static double calc_time_diff_ms(struct timeval *start, struct timeval *end){
    double start_ms = (start->tv_sec * 1000.0) + (start->tv_usec / 1000.0);
    double end_ms = (end->tv_sec * 1000.0) + (end->tv_usec / 1000.0);
    return end_ms - start_ms;
}
