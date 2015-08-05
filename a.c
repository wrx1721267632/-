/*************************************************************************
	> File Name: a.c
	> Author: 
	> Mail: 
	> Created Time: 2015年08月04日 星期二 18时47分05秒
 ************************************************************************/

#include<stdio.h>
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

struct userinfo {
    char username[32];
    char password[32];
};

struct userinfo users[] = {{"linux","unix"},{"4507","4508"},{"clh","clh"},{"xl","xl"},{" "," "}};

int my_err(const char *err_string, int line)
{
    fprintf(stdin, "line:%d", line);
    perror(err_string);
    exit(1);
}

int find_user(char *username, char *userpassword)
{
    int i;
    int c;
    for(i=0; users[i].username[0] != ' '; i++) {
        if(strncmp(users[i].username, username, (strlen(username)-1)) == 0){
            if(strncmp(users[i].password, userpassword, (strlen(userpassword)-1)) == 0){
                return 1;
            }
        }
    }
    return 0;
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
    char username[32];
    char userpassword[32];
    
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

    if( bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) < 0) {
        my_err("bind", __LINE__);
    }

    if(listen(sock_fd, LISTENQ) < 0) {
        my_err("bind", __LINE__);
    }

    cli_len = sizeof(struct sockaddr_in);
     while(1) {
        conn_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &cli_len);
//printf("%d\n", conn_fd);
        if(conn_fd < 0){
            my_err("accept", __LINE__);
        }
	printf("accept a new client, ip:%s\n", inet_ntoa(cli_addr.sin_addr));
	if((pid = fork()) == 0) {
		recv(conn_fd, username, 1024, 0);
printf("\n");
printf("it input username is %s\n",username);
		if(recv(conn_fd, userpassword, 1024, 0) < 0){
			my_err("recv",__LINE__);		
		}
printf("it input userpassword is %s\n",userpassword);		
		if(find_user(username,userpassword) == 1){
			send(conn_fd, "y\n", 3, 0);
			if(send (conn_fd, "welcome to it !\n", 17, 0) < 0) {
        			my_err("send", __LINE__);
    			}
		}else{
			send(conn_fd, "n\n", 3, 0);
		}
		close(conn_fd);
		close(sock_fd);
 		exit(0);
	} else {
            close(conn_fd);
        }
    }
    return 0;
}

