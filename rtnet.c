/*
 * Quang: app nay de giao tiep voi retardnet module
 * muc dich de set rules 1 cach linh hoat hon
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>

#define NETLINK_USER 31

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

void send_target_ip(const char *ip)
{
    __be32 target_ip = inet_addr(ip); // Convert IP to network byte order
    memcpy(NLMSG_DATA(nlh), &target_ip, sizeof(target_ip));

    sendmsg(sock_fd, &msg, 0);
    printf("Sent IP to kernel: %s\n", ip);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <target_ip>\n", argv[0]);
        return -1;
    }

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
    {
        perror("socket");
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(__be32)));
    memset(nlh, 0, NLMSG_SPACE(sizeof(__be32)));
    nlh->nlmsg_len = NLMSG_SPACE(sizeof(__be32));
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    send_target_ip(argv[1]);

    close(sock_fd);
    return 0;
}
