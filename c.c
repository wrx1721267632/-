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

#define MAX_TO 100

struct userinfo {
    char username[32];
    char password[32];
    int fd;
};


struct userinfo users[100]; //= {{"linux","unix",-1},{"4507","4508",-1},{"clh","clh",-1},{"xl","xl",-1},{" "," ",-1}};
void Memster()
{
    int i;
    for(i=0; i<100; i++) {
    	strcpy(users[i].username, " ");
    	users[i].fd = -1; 
    }
}

int my_err(const char *err_string, int line)
{
    fprintf(stdin, "line:%d", line);
    perror(err_string);
    exit(1);
}

void Read()
{
	int i=1;
        FILE *fp;
	fp=fopen("user.txt","rt");
	if(fp == NULL)
	{
		printf("cann't open!\n");
		sleep(3);
		exit(1);
	}
	if(fread(&users[0],sizeof(struct userinfo),1,fp)==0) {
		return ;
	}
	users[0].fd = -1;
	while(1){
		if(fread(&users[i],sizeof(struct userinfo),1,fp) == 0)
			break;
		i++;
		users[i].fd = -1;
	}
	fclose(fp);
}

void Write()
{
	int i;
	FILE *fp;
	fp=fopen("user.txt","wt");
	if(fp == NULL)
	{
		printf("cann't open!\n");
		sleep(3);
		exit(1);
	}
	for(i=0; users[i].username[0] != ' '; i++) {
		fwrite(&users[i],sizeof(struct userinfo),1,fp);
	}
	fclose(fp);
}

void ADD(char *username,char *password)
{
	int i;
	for(i=0; users[i].username[0] != ' '; i++) ;
	strcpy(users[i].username, username);
	strcpy(users[i].password, password);
}

/*void Destory()
{
    struct userinfo *p;
    while(p != NULL) {
        p = pHead->next;
        pHead->next = p->next;
        free(p);
    }
}*/

int find_user(char *username, char *userpassword)
{
    int i;
    int c;
    
    for(i=0; users[i].username[0] != ' '; i++) {
        if(strncmp(users[i].username, username, (strlen(users[i].username)-1)) == 0){
        //printf("find!\n");
            if(strncmp(users[i].password, userpassword, (strlen(users[i].password)-1)) == 0){
            //printf("online!\n");
                return i;
            }
        }
    }
    return -1;
}

void do_it(int conn_fd)
{
    int i, j;
    char username[32];
    char userpassword[32];
    char recv_buf[100],send_buf[100];
    char findname[32];
    
    
    while(1){
        recv(conn_fd, recv_buf, MAX_TO, 0);
    	if(strcmp(recv_buf,"denglu") == 0){
       	    recv(conn_fd, username, 32, 0);
    	    printf("%d input username is %s", conn_fd, username);
    	    if(recv(conn_fd, userpassword, 32, 0) < 0){
		my_err("recv",__LINE__);		
    	    }
    	    printf("%d input userpassword is %s", conn_fd, userpassword);		
    	    if((i = find_user(username,userpassword)) > -1){
    	   	if(users[i].fd == -1){
   	 	    users[i].fd = conn_fd;
   		    send(conn_fd, "y\n", 3, 0);
		    if(send (conn_fd, "服务器： welcome to it !", 100, 0) < 0) {
  		      	my_err("send", __LINE__);
   	 	    }
  	 	    printf("accept succes!\n");
  	 	    strcpy(recv_buf, users[i].username);
  	 	    recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    printf("%s is online!\n",recv_buf);
   	 	    strcat(recv_buf, " is online");
   	 	    for(j=0; users[j].username[0] != ' '; j++) {
   	 	    	if((users[j].fd != -1) && (i != j)){
   	 	    	    send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	    	    strcpy(recv_buf, users[j].username);
  	 	    	    recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    	    strcat(recv_buf, " is online");
   	 	    	    send(users[i].fd, recv_buf, MAX_TO, 0);
   	 	    	}
   	 	    }
 	   	    break;
 	   	}
  	  	else{
  	  	printf("user :%d\n", users[i].fd);
		    send(conn_fd, "n\n", 3, 0);
		    printf("accept error!\n");
	    //close(users[i].fd);
	    //users[i].fd = -1;
            //pthread_exit(0);
   	 	}
  	    }else{
		send(conn_fd, "n\n", 3, 0);
		printf("accept error!\n");
	//close(users[i].fd);
	//users[i].fd = -1;
        //pthread_exit(0);
 	   }
 	}
        else if(strcmp(recv_buf,"zhuce") == 0) {
       	      	recv(conn_fd, username, 32, 0);
    	    	printf("%d zhuce username is %s", conn_fd, username);
    	    	if(recv(conn_fd, userpassword, 32, 0) < 0){
			my_err("recv",__LINE__);		
    	    	}
    	    	printf("%d zhuce userpassword is %s", conn_fd, userpassword);		
    	    	if((i = find_user(username,userpassword)) > -1){
    	    		printf("error111\n");
    	   		send(conn_fd, "服务器： have tne person", MAX_TO, 0);  	   		
  	    	}else{
  	    		printf("ok!\n");
  	    		send(conn_fd, "服务器： zhuce sucess!", MAX_TO, 0);  	    		
			ADD(username, userpassword);
			Write();
			//break;
	//close(users[i].fd);
	//users[i].fd = -1;
        //pthread_exit(0);
 	   	}
    	}
    }
    //printf("aa\n");
    
    while(1) {
    	if(recv(conn_fd, recv_buf, MAX_TO, 0) == 0){
    	printf("%s go away\n", users[i].username);
            	send(conn_fd, "-exit\n", MAX_TO, 0);
            	for(j=0; users[j].username[0] != ' '; j++) {
   		    	    	//printf("%s  %d\n",users[j].username,users[j].fd);
   	    		if((users[j].fd != -1) && (i != j)){
   	 	    	    			//send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	 	    strcpy(recv_buf, users[i].username);
  	 		    recv_buf[strlen(recv_buf)-1] = '\0';
   	 		    strcat(recv_buf, " is go away");
   	 		    send(users[j].fd, recv_buf, MAX_TO, 0);
   	       	 	}
   	    	}
            Read();         			
    	    users[i].fd = -1;
    	    pthread_exit(0);
    	    }
        if(strcmp(recv_buf, "-exit\n") == 0){
            printf("%s go away\n", users[i].username);
            send(conn_fd, "-exit\n", MAX_TO, 0);
            strncpy(recv_buf, users[i].username, strlen(users[i].username)-1);
   	    strcat(recv_buf," is go away!\n");
   	    send(users[j].fd, recv_buf, MAX_TO, 0);
            for(j=0; users[j].username[0] != ' '; j++) {
   		    	    	//printf("%s  %d\n",users[j].username,users[j].fd);
   	    	if((users[j].fd != -1) && (i != j)){
   	 	    	    			//send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	    strcpy(recv_buf, users[j].username);
  	 	    recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    strcat(recv_buf, " is go away");
   	 	    send(users[j].fd, recv_buf, MAX_TO, 0);
   	        }
   	    }
            Read();         			
    	    users[i].fd = -1;
    	    pthread_exit(0);
        }
        //printf("recive a data: %s                 from %s\n",recv_buf,users[i].username);
        if(strncmp(recv_buf, "to ",3) == 0){
	    for(j=3; recv_buf[j] != '\n'; j++){
	        findname[j-3] = recv_buf[j];
	    }
	    findname[j++] = '\0';
	    //puts(findname);
	    //printf("aaa%sbb\n",findname);
	    for(j=0; users[j].username[0] != ' '; j++) {
	    //puts(users[j].username);
        	if((strncmp(users[j].username, findname,strlen(users[j].username)-1) == 0) && (users[j].fd != -1)){
            	    send(users[i].fd, "服务器： it's ok,you can talk with your friend\n", MAX_TO, 0);
            	    break;
        	}        	
    	    } 
    	    if(users[j].username[0] == ' '){
        	    send(users[i].fd, "服务器： don,t online or don't have the person!\n", MAX_TO, 0);
            }
            else{
            	//send(users[j].fd, users[i].username, 100, 0);
            	//send(users[j].fd, " do you agree(y/n)the other input look at n\n", 100, 0);
   		//recv(users[j].fd, recv_buf, 100, 0);
   		//printf("recive a data: %s    from %s",recv_buf,users[j].username);
   		//if(strcmp(recv_buf, "y\n") == 0){
   		    //send(users[i].fd, "there is agree,you can input to there\n", 100, 0);
   		    //send(users[j].fd, "please input :to his id\n", 100, 0);
   		    while(1){
   		    	//printf("i = %d, j = %d\n",users[i].fd,users[j].fd);
   		    	if(recv(users[i].fd, recv_buf, MAX_TO, 0) == 0){
   		    	printf("%s go away\n", users[i].username);
            		send(conn_fd, "-exit\n", MAX_TO, 0);
            		for(j=0; users[j].username[0] != ' '; j++) {
   		    	    	//printf("%s  %d\n",users[j].username,users[j].fd);
   	    		if((users[j].fd != -1) && (i != j)){
   	 	    	    			//send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	 	    strcpy(recv_buf, users[i].username);
  	 		    recv_buf[strlen(recv_buf)-1] = '\0';
   	 		    strcat(recv_buf, " is go away");
   	 		    send(users[j].fd, recv_buf, MAX_TO, 0);
   	       	 	}
   	    		}
            		Read();         			
    	    		users[i].fd = -1;
    	    		pthread_exit(0);
    	    		}
   		    	printf("recive a data to %s: %s    from the %s\n",users[j].username, recv_buf,users[i].username);
   		    	if(strcmp(recv_buf, "exit\n") == 0) {
   		    	    for(j=0; users[j].username[0] != ' '; j++) {
   	 	    		if((users[j].fd != -1) && (i != j)){
   	 	    	    	send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	    	    	strcpy(recv_buf, users[j].username);
  	 	    	    	recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    	    	strcat(recv_buf, " is online");
   	 	    	    	send(users[i].fd, recv_buf, MAX_TO, 0);
   	 	    		}
   	 	    	    }
   		    	    //send(users[j].fd, recv_buf, MAX_TO, 0);		  
   		    	    break;
   		    	}
   		    	else if(strcmp(recv_buf, "-exit\n") == 0){
            			printf("%s go away\n", users[i].username);
            			send(conn_fd, "-exit\n", MAX_TO, 0);
            			strncpy(recv_buf, users[i].username, strlen(users[i].username)-1);
   		    	    	strcat(recv_buf," is go away!!!!!!!!!\n");
   		    	    	send(users[j].fd, recv_buf, MAX_TO, 0);
   		    	    	for(j=0; users[j].username[0] != ' '; j++) {
   		    	    	//printf("%s  %d\n",users[j].username,users[j].fd);
   	 	    			if((users[j].fd != -1) && (i != j)){
   	 	    	    			//send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	    	    			strcpy(recv_buf, users[j].username);
  	 	    	    			recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    	    			strcat(recv_buf, " is go away!!!!!!!!!");
   	 	    	    			send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	    			}
   	 	    		}
            			Read();         			
    	    			users[i].fd = -1;
    	    			pthread_exit(0);
        		}
   		    	usleep(10);
   		    	recv_buf[strlen(recv_buf)-1] = '\0';
   		    	strcpy(send_buf, users[i].username);
   		    	send_buf[strlen(users[i].username) - 1] = '\0';
   		    	strcat(send_buf, ": ");
   		    	strcat(send_buf, recv_buf);
   		    	//printf("i = %d, j = %d\n",users[i].fd,users[j].fd);
   		    	//puts(recv_buf);
   		    	if(send(users[j].fd, send_buf, MAX_TO, 0) < 0){
   		    		printf("error!\n");
   		    	}
   		    	//printf("i = %d, j = %d\n",users[i].fd,users[j].fd);
   		    	usleep(10);
   		    	
   		    }
   		//}
   		//else{
   		    //send(users[i].fd, "there is disagree,you can't talk with there\n", 100, 0);
   		//}
            }   	           
        }
        if(strcmp(recv_buf, "all\n") == 0){
        	send(conn_fd, "成功进入群聊\n", MAX_TO, 0);
   		    while(1){
   		    	//printf("i = %d, j = %d\n",users[i].fd,users[j].fd);
   		    	if(recv(users[i].fd, recv_buf, MAX_TO, 0) == 0){
   		    	printf("%s go away\n", users[i].username);
            		send(conn_fd, "-exit\n", MAX_TO, 0);
            		for(j=0; users[j].username[0] != ' '; j++) {
   		    	    	//printf("%s  %d\n",users[j].username,users[j].fd);
   	    		if((users[j].fd != -1) && (i != j)){
   	 	    	    			//send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	 	    strcpy(recv_buf, users[i].username);
  	 		    recv_buf[strlen(recv_buf)-1] = '\0';
   	 		    strcat(recv_buf, " is go away!!!!!!!!!");
   	 		    send(users[j].fd, recv_buf, MAX_TO, 0);
   	       	 	}
   	    		}
           		 Read();         			
    	    		users[i].fd = -1;
    	    		pthread_exit(0);
    	    		}
   		    	//printf("recive a data to %s: %s    all talk from the %s\n",users[j].username, recv_buf,users[i].username);
   		    	if(strcmp(recv_buf, "exit\n") == 0) {
   		    	    for(j=0; users[j].username[0] != ' '; j++) {
   	 	    		if((users[j].fd != -1) && (i != j)){
   	 	    	    	send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	    	    	strcpy(recv_buf, users[j].username);
  	 	    	    	recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    	   	strcat(recv_buf, " is online");
   	 	    	    	send(users[i].fd, recv_buf, MAX_TO, 0);
   	 	    		}
   	 	    	}
   		    	    //send(users[j].fd, recv_buf, MAX_TO, 0);		  
   		    	    break;
   		    	}
   		    	else if(strcmp(recv_buf, "-exit\n") == 0){
            			printf("%s go away\n", users[i].username);
            			send(conn_fd, "-exit\n", MAX_TO, 0);
            			strncpy(recv_buf, users[i].username, strlen(users[i].username)-1);
   		    	    	strcat(recv_buf," is go away!\n");
   		    	    	send(users[j].fd, recv_buf, MAX_TO, 0);
   		    	    	for(j=0; users[j].username[0] != ' '; j++) {
   		    	    	//printf("%s  %d\n",users[j].username,users[j].fd);
   	 	    			if((users[j].fd != -1) && (i != j)){
   	 	    	    			//send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	    	    			strcpy(send_buf, users[i].username);
   	 	    	    			send_buf[strlen(users[i].username) - 1] = '\0';
   		    				strcat(send_buf, ": ");
   		    				strcat(send_buf, recv_buf);
   	 	    	    			send(users[j].fd, send_buf, MAX_TO, 0);
   	 	    			}
   	 	    		}
            			Read();         			
    	    			users[i].fd = -1;
    	    			pthread_exit(0);
        		}
   		    	usleep(10);
   		    	//recv_buf[strlen(recv_buf)-1] = '\0';
   		    	strcpy(send_buf, " 群聊消息，来自 ");
   		    	strcat(send_buf, users[i].username);
   		    	send_buf[strlen(send_buf)-1] = '\0';
   		    	strcat(send_buf, ": ");
   		    	strcat(send_buf, recv_buf);
   		    	//printf("i = %d, j = %d\n",users[i].fd,users[j].fd);
   		    	//puts(recv_buf);
   		    	for(j=0; users[j].username[0] != ' '; j++) {
   		    	    	//printf("%s  %d\n",users[j].username,users[j].fd);
   	 	    		if((users[j].fd != -1) && (i != j)){
   	 	    	    		//send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	    	    		send(users[j].fd, send_buf, MAX_TO, 0);
   	 	    		}
   	 	    	}
   		    	//printf("i = %d, j = %d\n",users[i].fd,users[j].fd);
   		    	usleep(10);
   		    	
   		//}
   		//else{
   		    //send(users[i].fd, "there is disagree,you can't talk with there\n", 100, 0);
   		//}
            }   	           
        }
    }
    Read();	   
    users[i].fd = -1;
    pthread_exit(0);
}

int main()
{
    int     sock_fd, conn_fd[12];
    int     optval;
    int     ret;
    int     name_num;
    pid_t   pid;
    socklen_t   cli_len;
    struct sockaddr_in   cli_addr, serv_addr;
    char    recv_buf[128];
    int j=0;
    pthread_t thid;
    
    
    Memster();
    Read();
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
    inet_aton("0.0.0.0", &serv_addr.sin_addr);

    if( bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) < 0) {
        my_err("bind", __LINE__);
    }

    /*if(listen(sock_fd, LISTENQ) < 0) {
        my_err("bind", __LINE__);
    }

    cli_len = sizeof(struct sockaddr_in);*/
     while(1) {
     	if(listen(sock_fd, LISTENQ) < 0) {
        	my_err("bind", __LINE__);
    	}

    	cli_len = sizeof(struct sockaddr_in);
     	j++;
     	if(j >= 11){
     	     j = 0;
     	}
        conn_fd[j] = accept(sock_fd, (struct sockaddr*)&cli_addr, &cli_len);
//printf("%d\n", conn_fd);
        if(conn_fd[j] < 0){
            my_err("accept", __LINE__);
        }
	printf("accept a new client, ip:%s\n", inet_ntoa(cli_addr.sin_addr));
	pthread_create(&thid, NULL, (void*)do_it, (int *)conn_fd[j] );
    }
    close(sock_fd);
    Write();
    
    return 0;
}
