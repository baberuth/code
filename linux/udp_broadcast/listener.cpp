#include <arpa/inet.h>
#include <assert.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "NetUtils.h"
#include "constants.h"


static int create_listen(const char* iface, const char* ipaddr, unsigned int port)  {
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(-1);
    }

    u_int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("reusing ADDR failed");
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //addr.sin_addr.s_addr = inet_addr(ipaddr);
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("bind");
        exit(-1);
    }

    int broadcast = 1;
    if (setsockopt(fd,SOL_SOCKET,SO_BROADCAST, &broadcast, sizeof(broadcast)) != 0) {
        perror("setsockopt - SO_SOCKET");
        exit(-1);
    }

#if 0
    // NOTE: binding needs some capabilities (Root access)
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name));
    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) != 0) {
        perror("setsockopt - SO_BINDTODEVICE");
        //exit(-1);
    }
#endif

    return fd;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s [iface]\n", argv[0]);
        return -1;
    }
    char* iface = argv[1];

    NetUtils::Interface interface;
    if (NetUtils::findInterface(iface, &interface)) {
        fprintf(stderr, "cannot find interface %s\n", iface);
        return -1;
    }
    printf("using interface %s  addr=%s  netmask=%s  bcast=%s\n", interface.name.c_str(),
        interface.addr.c_str(), interface.netmask.c_str(), interface.bcast.c_str());

    int fd = create_listen(iface, interface.bcast.c_str(), PORT);

    while (1) {
        int len = 1500;
        char buffer[len];
        int flags = 0;
        struct sockaddr_in remote;
        unsigned sin_size = sizeof(struct sockaddr_in);
        memset(&remote, 0, sin_size);
        int res = recvfrom(fd, buffer, len, flags, (struct sockaddr*)&remote, &sin_size);
        if (res == -1) {
            perror("recvfrom");
            break;
        }
        buffer[res] = 0;
        const char* src_addr = inet_ntoa(remote.sin_addr);
        int src_port = ntohs(remote.sin_port);
        assert (remote.sin_family == AF_INET);
        printf("received %d bytes from %s:%d %s\n", res, src_addr, src_port, buffer);
    }
    close(fd);
    return 0;
}



