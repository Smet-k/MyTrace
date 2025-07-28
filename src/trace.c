#include <arpa/inet.h>
#include <linux/errqueue.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define PACKET_SIZE 64
#define TTL_MAX 30
#define BUFFERSIZE 1024

void send_icmp_packet(const int sock, const struct icmphdr* icmp_hdr, const struct sockaddr_in* destAddr);
void receive_icmp_packet(const int sock, char* packet, struct sockaddr_in* replyAddr);
int create_socket();
void read_errors(struct icmphdr icmph, struct sockaddr_in addr, int sock);

void trace(struct in_addr* dst) {
    const int sock = create_socket();
    struct icmphdr icmp_hdr;
    struct sockaddr_in destAddr;

    memset(&destAddr, 0, sizeof destAddr);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(33434);
    destAddr.sin_addr = *dst;

    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.un.echo.id = (uint16_t)getpid();
    icmp_hdr.code = 0;

    for (uint8_t ttl = 1; ttl <= TTL_MAX; ttl++) {
        struct sockaddr_in replyAddr;
        char packet[PACKET_SIZE];

        setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
        icmp_hdr.un.echo.sequence = ttl;

        send_icmp_packet(sock, &icmp_hdr, &destAddr);
        receive_icmp_packet(sock, packet, &replyAddr);

        read_errors(icmp_hdr, destAddr, sock);

        // printf("%s\n", inet_ntoa(replyAddr.sin_addr));
        if (replyAddr.sin_addr.s_addr == destAddr.sin_addr.s_addr){
            printf("%s\n", inet_ntoa(replyAddr.sin_addr));
            break;
        }
    }
    close(sock);
}

void send_icmp_packet(const int sock, const struct icmphdr* icmp_hdr,
                      const struct sockaddr_in* destAddr) {
    if (sendto(sock, icmp_hdr, sizeof(*icmp_hdr), 0,
               (struct sockaddr*)destAddr, sizeof(*destAddr)) == -1) {
        perror("Packet send failed");
    }
}

void receive_icmp_packet(const int sock, char* packet, struct sockaddr_in* replyAddr) {
    socklen_t addrLen = sizeof(*replyAddr);
    if (recvfrom(sock, packet, PACKET_SIZE, 0, (struct sockaddr*)replyAddr, &addrLen) == -1) {
        // perror("Packet receive failed");
    }
}

int create_socket() {
    const int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    const struct timeval timeout = {2, 0};  // temporary default timeout
    if (sock == -1) {
        perror("Socket creation failed");
    }
    int on = IP_PMTUDISC_DO;
    setsockopt(sock, SOL_IP, IP_MTU_DISCOVER, &on, sizeof(on));
    setsockopt(sock, SOL_IP, IP_RECVERR, &on, sizeof(on));
    setsockopt(sock, SOL_IP, IP_RECVTTL, &on, sizeof(on));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    return sock;
}

void read_errors(struct icmphdr icmph, struct sockaddr_in addr, int sock) {
    struct iovec iov;
    struct msghdr msg;                  /* Message header */
    struct cmsghdr* cmsg;               /* Control related data */
    struct sock_extended_err* sock_err; /* Struct describing the error */
    char buffer[BUFFERSIZE];
    int return_status = 0;
    iov.iov_base = &icmph;
    iov.iov_len = sizeof(icmph);

    msg.msg_name = (void*)&addr;
    msg.msg_namelen = sizeof(addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = buffer;
    msg.msg_controllen = sizeof(buffer);

    return_status = recvmsg(sock, &msg, MSG_ERRQUEUE);
    if (return_status >= 0) {
        /* Control messages are always accessed via some macros
         * http://www.kernel.org/doc/man-pages/online/pages/man3/cmsg.3.html
         */
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
            if (cmsg->cmsg_level == SOL_IP) {
                /* IP LEVEL*/
                if (cmsg->cmsg_type == IP_RECVERR || cmsg->cmsg_type == IP_RECVTTL) {
                    char rej_addr[INET_ADDRSTRLEN];
                    struct sockaddr_in* rejection_addr;
                    if (cmsg->cmsg_type == IP_RECVTTL) {
                        fprintf(stderr, "We got IP_RECVTTL message\n");
                    }
                    sock_err = (struct sock_extended_err*)CMSG_DATA(cmsg);

                    if (sock_err->ee_origin == SO_EE_ORIGIN_ICMP && (sock_err->ee_type == ICMP_TIME_EXCEEDED || sock_err->ee_type == ICMP_DEST_UNREACH)) {
                        switch (sock_err->ee_code) {
                            case ICMP_EXC_TTL:
                                rejection_addr = (struct sockaddr_in*)(sock_err + 1);
                                inet_ntop(AF_INET, &(rejection_addr->sin_addr), rej_addr, INET_ADDRSTRLEN);
                                printf("%s\n", rej_addr);
                                break;
                            case ICMP_DEST_UNREACH:
                                rejection_addr = (struct sockaddr_in*)(sock_err + 1);
                                inet_ntop(AF_INET, &(rejection_addr->sin_addr), rej_addr, INET_ADDRSTRLEN);
                                printf("%s\n", rej_addr);
                                break;
                            default:
                                printf("Unhandled Error:\n");
                                printf("\tError code: %d \n", sock_err->ee_code);
                                printf("\tError type: %d \n", sock_err->ee_type);
                                break;
                                /* Handle all other cases. Find more errors :
                                 * http://lxr.linux.no/linux+v3.5/include/linux/icmp.h#L39
                                 */
                        }
                    }
                }
            }
        }
    }
}