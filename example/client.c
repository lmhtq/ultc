#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#include <mtcp_api.h>
#include <mtcp_epoll.h>

#include "cpu.h"
#include "http_parsing.h"
#include "debug.h"

#define MAX_FLOW_NUM  (10000)

#define RCVBUF_SIZE (2*1024)
#define SNDBUF_SIZE (8*1024)

#define MAX_EVENTS (MAX_FLOW_NUM * 3)

#define MAX_CPUS 16
#define MAX_FILES 30

#define MAX(a, b) ((a)>(b)?(a):(b))
#define MIN(a, b) ((a)<(b)?(a):(b))

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef ERROR
#define ERROR (-1)
#endif

#define HT_SUPPORT FALSE

#define IP_RANGE 1
#define MAX_IP_STR_LEN 16

const int core = 1;
const short s_port = 81;
char host[MAX_IP_STR_LEN+1] = "10.0.0.4";
static in_addr_t daddr;
static in_port_t dport;
static in_addr_t saddr;

int server()
{
	mctx_t mctx;
	int ep_id;
	struct mtcp_epoll_event *events;
	int listen_id;
	int sockid;
	int ret;
	struct mtcp_epoll_event ev;
	struct sockaddr_in saddr;
	int c;	
	int i, n;
	int r_len;
	char r_buf[RCVBUF_SIZE];
	int s_len;
	char s_buf[SNDBUF_SIZE+2];

	ret = mtcp_init("server.conf");
	if (ret) {
		printf("Failed to initialize mtcp.\n");
		exit(-1);
	}

	mctx = mtcp_create_context(core);
	
	ep_id = mtcp_epoll_create(mctx, MAX_EVENTS);
	
	for (i = 0; i < SNDBUF_SIZE; i++)
		s_buf[i] = 'A' + i % 26;
	s_buf[i] = 0;

	listen_id = mtcp_socket(mctx, AF_INET, SOCK_STREAM, 0);
	if (listen_id < 0) {
		perror("Failed to create listening socket!\n");
	}

	ret = mtcp_setsock_nonblock(mctx, listen_id);
	if (ret < 0) {
		perror("Failed to set socket in nonblocking mode.\n");
	}

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(s_port);

	ret = mtcp_bind(mctx, listen_id, 
			(struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		perror("Failed to bind to the listening socket!\n");
	}

	events = (struct mtcp_epoll_event *)calloc(
			MAX_EVENTS, sizeof(struct mtcp_epoll_event));
	if (!events) {
		perror("Failed to create event struct.\n");
	}

	mtcp_listen(mctx, listen_id, 4096);

	while (1) {
		n = mtcp_epoll_wait(mctx, ep_id, events, core, -1);
		for (i = 0; i < n; i++) {
			sockid = events[i].data.sockid;
			if (sockid == listen_id) {
				c = mtcp_accept(mctx, listen_id, NULL, NULL);
				mtcp_setsock_nonblock(mctx, c);
				ev.events = MTCP_EPOLLIN | MTCP_EPOLLOUT;
				ev.data.sockid = c;
				mtcp_epoll_ctl(mctx, ep_id, MTCP_EPOLL_CTL_ADD, c, &ev);
			} else if (events[i].events == MTCP_EPOLLIN) {
				r_len = mtcp_read(mctx, sockid, r_buf, RCVBUF_SIZE);
				printf("lmhtq: read %dB\n%s\n", r_len, r_buf);
				if (r_len == 0) {
				 	printf("lmhtq: read 0\n");
					mtcp_close(mctx, sockid);
				} 
			} else if (events[i].events == MTCP_EPOLLOUT) {
				s_len = mtcp_write(mctx, sockid, s_buf, RCVBUF_SIZE);
				printf("lmhtq: write %dB\n");
			}
		}

	}
}


int client()
{
	struct mtcp_conf mcfg;
	mctx_t mctx;
	int ep_id;
	struct mtcp_epoll_event *events;
	int listen_id;
	int sockid;
	int ret;
	struct mtcp_epoll_event ev;
	struct sockaddr_in addr;
	int c;	
	int i, n;
	int r_len;
	char r_buf[RCVBUF_SIZE];
	int s_len;
	char s_buf[SNDBUF_SIZE+2];
	
	int core_nums;

	printf("client start...\n");
	core_nums = GetNumCPUs();

    ret = mtcp_init("client.conf");
	if (ret) {
		printf("Failed to initialize mtcp.\n");
		exit(-1);
	} else {
		printf("mtcp initialized.\n");
	}

	mctx = mtcp_create_context(core);

	ep_id = mtcp_epoll_create(mctx, MAX_EVENTS);

	for (i = 0; i < SNDBUF_SIZE; i++)
		s_buf[i] = 'A' + i % 26;
	s_buf[i] = 0;

	sockid = mtcp_socket(mctx, AF_INET, SOCK_STREAM, 0);
	if (sockid < 0) {
		printf("Failed to create socker!\n");
		exit(-1);
	}

	ret = mtcp_setsock_nonblock(mctx, listen_id);
	if (ret < 0) {
		printf("Failed to set socket in nonblocking mode.\n");
		exit(-1);
	}

	daddr = inet_addr(host);
	dport = htons(s_port);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = daddr;
	addr.sin_port = dport;

	ret = mtcp_connect(mctx, sockid, 
			(struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		perror("Failed to bind to the listening socket!\n");
		mtcp_close(mctx, sockid);
		exit(-1);
	}

	events = (struct mtcp_epoll_event *)calloc(
			MAX_EVENTS, sizeof(struct mtcp_epoll_event));
	if (!events) {
		perror("Failed to create event struct.\n");
	}

	while (1) {
		n = mtcp_epoll_wait(mctx, ep_id, events, core, -1);
		for (i = 0; i < n; i++) {
			sockid = events[i].data.sockid;
			if (sockid == listen_id) {
				c = mtcp_accept(mctx, listen_id, NULL, NULL);
				mtcp_setsock_nonblock(mctx, c);
				ev.events = MTCP_EPOLLIN | MTCP_EPOLLOUT;
				ev.data.sockid = c;
				mtcp_epoll_ctl(mctx, ep_id, MTCP_EPOLL_CTL_ADD, c, &ev);
			} else if (events[i].events == MTCP_EPOLLIN) {
				r_len = mtcp_read(mctx, sockid, r_buf, RCVBUF_SIZE);
				printf("lmhtq: read %dB\n%s\n", r_len, r_buf);
				if (r_len == 0) {
					printf("lmhtq: read 0\n");
					mtcp_close(mctx, sockid);
				}
			} else if (events[i].events == MTCP_EPOLLOUT) {
				s_len = mtcp_write(mctx, sockid, s_buf, RCVBUF_SIZE);
				printf("lmhtq: write %dB\n");
			}
		}

	}
}

int main()
{
	client();
	return 0;
}

