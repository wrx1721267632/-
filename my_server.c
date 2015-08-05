/*************************************************************************
	> File Name: my_server.c
	> Author: 
	> Mail: 
	> Created Time: 2015年08月03日 星期一 16时02分03秒
 ************************************************************************/

#include<stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>

#define SERV_PORT   4507
#define LISTENQ     12

int my_err(const char *err_string, int line)
{
    fprintf(stdin, "line:%d", line);
    perror(err_string);
    exit(1);
}

void output_data(int conn_fd)
{
    char recv_buf[32];
    while(1){
        recv(conn_fd, recv_buf, sizeof(recv_buf), 0);
        if(strcmp(recv_buf, "exit\n") == 0) {
            break;
        }
        printf("recive a data: %s",recv_buf);
    }
}

void get_data(char *input_buf)
{
    int i = 0;
    char c;
    while( i < 30) {
        c = getchar();
        if(c == '\n') {
            break;
        }
        input_buf[i++] = c;
    }
    input_buf[i++] = '\n';
    input_buf[i++] = '\0';
}

void input_data(int conn_fd)
{
    char input_buf[32];
    while(1) {
        get_data(input_buf);
        send(conn_fd, input_buf, sizeof(input_buf),0); 
        if(strcmp(input_buf, "exit\n") == 0) {
            break;
        }
    }
}


int main()
{
    int     sock_fd, conn_fd;
    int     optval;
    int     ret;
    int     name_num;
    pid_t   pid;
    socklen_t   cli_len;
    struct sockaddr_in   cli_addr, serv_addr;
    char    recv_buf[128];
    pthread_t thid1;
    pthread_t thid2;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0) {
        my_err("socket", __LINE__);
    }

    optval = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(int)) < 0) {
        my_err("setsockopt", __LINE__);
    }

    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_aton("192.168.20.118", &serv_addr.sin_addr);

    if(bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) < 0) {
        my_err("bind", __LINE__);
    }

    if(listen(sock_fd, LISTENQ) < 0) {
        my_err("bindd", __LINE__);
    }

    cli_len = sizeof(struct sockaddr_in);
    conn_fd = accept(sock_fd, (struct sockaddr *)&cli_addr, &cli_len);
    if(conn_fd < 0) {
    my_err("accept", __LINE__);
    }
    printf("accept a new client, ip: %s\n", inet_ntoa(cli_addr.sin_addr));
    if(send (conn_fd, "welcome to it !\n", 17, 0) < 0) {
        my_err("send", __LINE__);
    }
    pthread_create(&thid1, NULL, (void *) input_data,  (int *)conn_fd); 
    pthread_create(&thid2, NULL, (void *) output_data,  (int *)conn_fd); 
    pthread_join(thid1, NULL);
    close(conn_fd);
    return 0;
}
