/*************************************************************************
	> File Name: kehu.c
	> Author: 
	> Mail: 
	> Created Time: 2015年08月04日 星期二 10时43分54秒
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "my_recv.h"

#define INVALID_USERINFO  'n'
#define VALID_USERINFO    'y'

int get_userinfo(char *buf, int len) 
{
    int i;
    int c;
    if( buf == NULL ) {
        return -1;
    }

    i = 0;
    while (((c = getchar()) != '\n') && (c != EOF) && (i < len - 2)) {
        buf[i++] = c;
    }
    buf[i++] = '\n';
    buf[i++] = '\0';
    return 0;
}

void input_userinfo(int conn_fd, const char *string)
{
    char input_buf[32];
    char recv_buf[BUFSIZE];
    int flag_userinfo;

    do{
        printf("%s:", string);
        if(get_userinfo(input_buf, 32) < 0) {
            printf("error return form get_userinfo\n");
            exit(1);
        }

        if(send(conn_fd, input_buf, sizeof(recv_buf), 0) < 0){
            my_err("send", __LINE__);
        }

        if(my_recv(conn_fd, input_buf, strlen(input_buf)) < 0) {
            printf("data is too long\n");
            exit(1);
        }

        if(recv_buf[0] == VALID_USERINFO) {
            flag_userinfo = VALID_USERINFO;
        } else {
            printf("%s error ,input again,", string);
            flag_userinfo = INVALID_USERINFO;
        }
    }while(flag_userinfo == INVALID_USERINFO);
}

int main(int argc, char **argv)
{
    int i;
    int ret;
    int conn_fd;
    int serv_port;
    struct sockaddr_in serv_addr;
    char recv_buf[1024];


    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(4507);
    inet_aton("192.168.20.118",&serv_addr.sin_addr);
    
    conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn_fd < 0) {
        my_err("socket", __LINE__);
    }
    
    if(connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) {
        my_err("connect", __LINE__);
    }

    if((ret = recv(conn_fd,(void *)recv_buf, 17, 0)) < 0) {
        my_err("recv", __LINE__);
    }
    input_userinfo(conn_fd, "username");
    input_userinfo(conn_fd, "password");
    
    if((ret = my_recv(conn_fd, recv_buf, sizeof(recv_buf))) < 0) {
        printf("data is too long\n");
        exit(1);
    }

    for(i=0; i<ret; i++) {
        printf("%c", recv_buf[i]);
    }
    printf("\n");

    close(conn_fd);
    return 0;
}
