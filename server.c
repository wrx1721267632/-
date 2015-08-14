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
#include <time.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

//=============================================================宏定义=================================
#define SERV_PORT   4507
// 特定的端口号
#define LISTENQ     12
//监听函数链接队列的最大长度
#define MAX_TO 500
//=============================================================全局变量================================
pthread_mutex_t mutex_denglu;
//系统日志文件的文件互斥锁
pthread_mutex_t mutex_error;
//错误日志文件的文件互斥锁
char all_name[32] = "linux\n";
//管理员帐号
char all_pass[32] = "xiyou_linux";
//管理员密码
//==============================================================结构体================================
struct userinfo {
    char username[32];
    //用户帐号
    char password[32];
    //用户密码
    int fd;
    //用户套接字
};
struct userinfo users[100]; 
//==============================================================函数声明==============================
void Memster();
//用户数组的初始话函数
int my_err(const char *err_string, int line);
//错误提示函数
void Read();
//用户数组的获取
void Write();
//用户数组的写入
void ADD(char *username,char *password);
//用户数组的添加
int find_user(char *username, char *userpassword);
//查找在用户数组中是否存在对应的帐号密码
void do_it(int conn_fd);
//客户端发送消息的解析的函数
int find_delete(char *username);
//查找管理员要删除的帐号是否存在的函数
void my_guanli(int conn_fd);
//处理管理员的函数

//===================================================初始化用户数组=================================
void Memster()
{
    int i;
    for(i=0; i<100; i++) {
    	strcpy(users[i].username, " ");
    	users[i].fd = -1; 
    }
    //将用户数组的用户名初始化为空格键，其套接字初始化为-1
}

//====================================================错误提示函数=================================
int my_err(const char *err_string, int line)
{
    fprintf(stdin, "line:%d", line);
    perror(err_string);
    exit(1);
}

//====================================================读取用户数组函数=============================
void Read()
{
	int i=1;
    int fp;
    //存储用户信息的文件的文件描述符
	
	fp=open("user.txt",O_RDONLY);
	if(fp == -1)
	{
		printf("cann't open!\n");
		sleep(3);
		exit(1);
	}
	
	//存储用户信息的文件为空特殊处理
	if(read(fp,&users[0],sizeof(struct userinfo))==0) {
		return ;
	}
	users[0].fd = -1;
	
	//循环读取用户数组
	while(1){
		if(read(fp,&users[i],sizeof(struct userinfo)) == 0)
			break;
		i++;
		users[i].fd = -1;
	}
	close(fp);
}

//================================================将用户数组写入存储用户信息的文件=====================================
void Write()
{
	int i;
	int fp;
	//用户信息文件的文件描述符
	
	fp=open("user.txt",O_WRONLY|O_CREAT|O_TRUNC, 0777);
	
	if(fp == -1)
	{
		printf("cann't open!\n");
		sleep(3);
		exit(1);
	}
	
	for(i=0; users[i].username[0] != ' '; i++) {
		write(fp,&users[i],sizeof(struct userinfo));
	}
	close(fp);
}

//=======================================================用户数组的添加===========================================
void ADD(char *username,char *password)
{
	int i;
	
	for(i=0; i<100; i++){
		if((users[i].username[0] == '\n') || (users[i].username[0] == ' ')){
			strcpy(users[i].username, username);
			strcpy(users[i].password, password);
			break;
		}
	}
}

//==================================================查找在用户数组中是否存在对应的帐号密码===============================
int find_user(char *username, char *userpassword)
{
    int i;
    int c;
    
    for(i=0; users[i].username[0] != ' '; i++) {
        if(strcmp(users[i].username, username) == 0){
            if(strcmp(users[i].password, userpassword) == 0){
                return i;
            }
        }
    }
    return -1;
    //返回值用于判断其是否存在
}

//===========================================================查找帐号是否存在=====================================
int find_delete(char *username)
{
	int i;
	for(i=0; users[i].username[0] != ' '; i++) {
        if(strcmp(users[i].username, username) == 0){
                return i;
        }
    }
    return -1;
}

//===========================================================处理管理员的函数==============================================
void my_guanli(int conn_fd)
{
	char recv_buf[1024];
	//接受与发送的数组
	char name[32];
	//接受客户端发送来的帐号
	char pass[32];
	//接受客户端发送来的密码
	int i;
	int fd;
	//聊天记录文件的描述符
	char a[32];
	DIR *dir;
	struct dirent *ptr;
	
	//接收管理员的用户名与密码进行比较，正确返回y，错误返回n
	recv(conn_fd, name, MAX_TO, 0);
	recv(conn_fd, pass, MAX_TO, 0);
	if((strcmp(name, all_name) == 0) && (strcmp(pass, all_pass) == 0))
	{
		send(conn_fd,"y\n", MAX_TO, 0);
	} else{
		send(conn_fd,"n\n", MAX_TO, 0);
		return ;
	}
	
	recv(conn_fd, recv_buf, 32, 0);
	//接受客户端发来的操作提醒
	
	//recv_buf若为delete，则选择删除用户，若未jilu，则选择查看聊天记录
	if(strcmp(recv_buf, "delete") == 0){
		while(1){
			if(recv(conn_fd, recv_buf, MAX_TO, 0) == 0){         			
    			break;
    			//用于防止客户端异常下线
    		}
    		
    		if(strcmp(recv_buf, "-exit\n") == 0){
    			break;
    			//收到-exit则为客户端正常下线
    		}
    		
    		puts(recv_buf);
    		if((i = find_delete(recv_buf)) > -1){
    			//若fd ！= -1， 则代表其在线，需先将其踢下线再进行删号
    			if(users[i].fd != -1){   		
    				send(users[i].fd, "你已经被管理员踢出！！！", MAX_TO, 0);
    				send(users[i].fd, "-exit\n", MAX_TO, 0);
    			}
    			
    			//将帐号密码更新为\n则为删除
    			strcpy(users[i].username,"\n");
    			strcpy(users[i].password,"\n");
    			Write();
    			send(conn_fd, "sucess delete!\n", MAX_TO, 0);
    		} else {
    			send(conn_fd, "don't have the person!\n", MAX_TO, 0);
    		}	
    	}
    } else if (strcmp(recv_buf, "jilu") == 0){
    recv(conn_fd, recv_buf, 32, 0);
    while(1){
    	i = 0;
    	
    	if(recv(conn_fd, recv_buf, 32, 0) == 0){
    		break;
    		//用于防止客户端异常下线
    	}
    	
    	if(strcmp(recv_buf, "-exit.txt") == 0){
    		break;
    		//收到-exit则为客户端正常下线，但客户端还对-exit增加了.txt
    	}
    	
    	if((dir = opendir("./")) == NULL){
    		send(conn_fd, "don't find!", MAX_TO, 0);
    		//查找服务器当前目录下是否有该用户的聊天记录文件
    		continue;
    	}
    	
    	//若查找到要查找的聊天记录文件，则标志置为一
    	while((ptr = readdir(dir)) != NULL){
    		if(strcmp(recv_buf, ptr->d_name) == 0){
    			i = 1;
    			break;
    		}
    	}
    	
    	//标志仍未0,则代表没找到要找的聊天记录文件
    	if(i == 0){
    		send(conn_fd, "don't find!", MAX_TO, 0);
    		continue;
    	}
    	
    	send(conn_fd, "find it!", MAX_TO, 0);
    			fd = open(recv_buf, O_RDONLY);
				while(1){
					i = read(fd, a, 499);
					send(conn_fd, a, 499,0);
					memset(a,0,sizeof(a));
					if(i != 499){				
						break;
					}								
				}
				close(fd);
	}
    }
    return ;
}

//=============================================服务器接收到的数据的解析与处理=========================
void do_it(int conn_fd)
{
    int i, j ,k, m;
    char username[32];
    //接收到的用户（包括管理员的用户名）
    char userpassword[32];
    //接收到的密码（包括管理员的密码）
    char recv_buf[500],send_buf[500];
    //接收到的数据或发送的数据
    char findname[32];
    //用私聊时查找到的要接受数据的用户名
    time_t timep;
    int beiyong_fd;
    //记录私聊时要接受数据的套接字
    int 	fp;
    char	a[500]; 
    int 	fd; 
    int 	fd1;
    char 	filename_lixian[32];
    DIR *dir;
	struct dirent *ptr;

//==========================================第一个大循环相当于第一个界面的选择所发过来的数据的解析=================================    
    while(1){
    	//选择退出或异常退出
        if(recv(conn_fd, recv_buf, MAX_TO, 0) == 0){         			
    	    users[i].fd = -1;
    	    return ;
    	}
    	
    	//选择以管理员身份进入
    	if(strcmp(recv_buf,"guanli") == 0){
    		my_guanli(conn_fd);
    		return ;
    	}
    	
    	//选择以普通身份进入
    	if(strcmp(recv_buf,"denglu") == 0){
    		//接收用户名和密码，并判断对错
       	    recv(conn_fd, username, 32, 0);
    	    if(recv(conn_fd, userpassword, 32, 0) < 0){
				my_err("recv",__LINE__);		
    	    }		
    	   
    	    if((i = find_user(username,userpassword)) > -1){
    	   		if(users[i].fd == -1){
   	 	    		users[i].fd = conn_fd;
   		    		send(conn_fd, "y\n", 3, 0);
   		    		
		    		if(send (conn_fd, "服务器： welcome to it !", 500, 0) < 0) {
  		      			my_err("send", __LINE__);
   	 	   			}
   	 	   			
   	 	    		strcpy(recv_buf, users[i].username);
   		    		strcat(recv_buf, "-1.txt");
   		    		dir = opendir("./");
    				while((ptr = readdir(dir)) != NULL){
    					if(strcmp(recv_buf, ptr->d_name) == 0){
    						break;
    					}
    				}
   		    		if(ptr != NULL){
   		    			fd1 = open(recv_buf, O_RDONLY);
   		    			while(1){
							if(read(fd1, a, 100) != 100){
								break;
							}
							a[100] = '\0';
							send(users[i].fd, a, 500,0);
							memset(a,0,sizeof(a));					
						}
						close(fd);
   		    			rename(recv_buf, "#nouse#.txt");
   		    		}
   		    		
   	 	   			//登录成功在服务器端提醒登录成功的信息
   	 	    		time (&timep);
  	 	    		printf("accept succes!\n");
  	 	    		printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
  	 	    		strcpy(recv_buf, users[i].username);
  	 	    		recv_buf[strlen(recv_buf)-1] = '\0';
  	 	    		time (&timep);
   	 	    		printf("%s is online!\n",recv_buf);
   	 	   	 		printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
   	 	   	 		strcat(recv_buf, " is online");
   	 	   	 	
   	 	   	 		//将登录成功的消息保存与系统日志
   	 	   	 		pthread_mutex_lock(&mutex_denglu);
					fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
					sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
					write(fp, a, strlen(a));
					close(fp);
					pthread_mutex_unlock(&mutex_denglu);
   		    		
   	 	    		//给所有在线的人发消息，登录成功的人以上线
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
 	   			
 	   			//登陆失败，在服务器端显示，并保存信息于错误日志中
 	   			else{
  	  				printf("user :%d\n", users[i].fd);
		    		send(conn_fd, "n\n", 3, 0);
		    		time (&timep);
		    		printf("accept error!\n");
		    		printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
		    		
		    		//登录错误，保存在错误日志中
		    		pthread_mutex_lock(&mutex_error);
					fp = open("error.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
					sprintf(a, "%s is online,accept error!\n %s\n", users[i].username, ctime(&timep));
					write(fp, a, strlen(a));
					close(fp);
					pthread_mutex_unlock(&mutex_error);
   	 			}
  	   		}else{
  	   			printf("%d\n",i);
				send(conn_fd, "n\n", 3, 0);
				time (&timep);
				printf("accept error!\n");
				printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
				
				//登录错误，保存在错误日志中
				pthread_mutex_lock(&mutex_error);
				fp = open("error.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
				sprintf(a, "%s is online,accept error!\n %s\n", users[i].username, ctime(&timep));
				write(fp, a, strlen(a));
				close(fp);
				pthread_mutex_unlock(&mutex_error);
 	   		}
 		}
 		
 		//选择注册
 		else if(strcmp(recv_buf,"zhuce") == 0) {
 		//获取注册的用户名，密码
       	    recv(conn_fd, username, 32, 0);
       	    time (&timep);
    	   	printf("%d zhuce username is %s", conn_fd, username);
    	    printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
    	    if(recv(conn_fd, userpassword, 32, 0) < 0){
				my_err("recv",__LINE__);		
    	    }	
    	    
    	    //判断用户名是否存在	    	    
    	    if((i = find_delete(username)) > -1){
    	   		send(conn_fd, "服务器： have tne person", MAX_TO, 0);  	   		
  	    	}else{
  	    		printf("ok!\n");
  	    		send(conn_fd, "服务器： zhuce sucess!", MAX_TO, 0);
  	    		strcpy(recv_buf,username);
  	    		recv_buf[strlen(recv_buf)-1] = '\0';
				
				//将注册成功的信息存入系统日志中
  	    		pthread_mutex_lock(&mutex_denglu);
				fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
				sprintf(a, "%s zhuce sucess!\n %s\n",recv_buf,ctime(&timep));
				write(fp, a, strlen(a));
				close(fp);
				pthread_mutex_unlock(&mutex_denglu);
				
				//注册成功后将其添加在，用户数组中，并保存在文件中 	    		
				ADD(username, userpassword);
				Write();
 	   		}
    	}
    }
 
//===========================================以普通用户进入后的解析==========================================    
    while(1) {
    //防止用户异常退出
    	if(recv(conn_fd, recv_buf, MAX_TO, 0) == 0){
    			time (&timep);
    			strcpy(recv_buf, users[i].username);
  				recv_buf[strlen(recv_buf)-1] = '\0';
   				strcat(recv_buf, " is  error go away");
    			printf("%s \n", recv_buf);
    			printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
    			
    			//将异常推出的信息保存在系统日志中
    			pthread_mutex_lock(&mutex_denglu);
				fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
				sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
				write(fp, a, strlen(a));
				close(fp);
				pthread_mutex_unlock(&mutex_denglu);
				
				//将异常推出的信息保存在错误日志中
				pthread_mutex_lock(&mutex_error);
				fp = open("error.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
				sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
				write(fp, a, strlen(a));
				close(fp);
				pthread_mutex_unlock(&mutex_error);
            	
            	//向所有在线用户发送该用户退出的信息
            	send(conn_fd, "-exit\n", MAX_TO, 0);
            	for(j=0; users[j].username[0] != ' '; j++) {
   	    			if((users[j].fd != -1) && (i != j)){
   	 	 	    		strcpy(recv_buf, users[i].username);
  	 		    		recv_buf[strlen(recv_buf)-1] = '\0';
   	 		    		strcat(recv_buf, " is go away");
   	 		    		send(users[j].fd, recv_buf, MAX_TO, 0);
   	       	 		}
   	    		}         			
    	    	users[i].fd = -1;
    	    	return ;
    	}
        
    	//正常退出
        if(strcmp(recv_buf, "-exit\n") == 0){
    	    users[i].fd = -1;
        	//接收退出用户的聊天记录文件
        	strcpy(a,users[i].username);
        	a[strlen(a)-1] = '\0';
        	strcat(a,".txt");
        	fd = open(a, O_WRONLY|O_CREAT|O_TRUNC, 0777);
        	close(fd);
        	fd = open(a, O_WRONLY|O_APPEND|O_CREAT, 0777);
        	while(1){
        		if(recv(conn_fd, recv_buf, 499, 0) != 499){
        			break;
        		}
        		j = strlen(recv_buf);    		
        		if( (write(fd, recv_buf, j)) != 499){
        			break;
        		}
        	}
        	close(fd);
        	
        	//服务器显示错误提示
            time (&timep);
            strcpy(recv_buf, users[i].username);
            recv_buf[strlen(recv_buf)-1] = '\0';
   	    	strcat(recv_buf, " is go away");
    	    printf("%s \n", recv_buf);
            printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
            
            //将退出的信息保存在日志文件中
            pthread_mutex_lock(&mutex_denglu);
			fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
			sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
			write(fp, a, strlen(a));
			close(fp);
			pthread_mutex_unlock(&mutex_denglu);
            
            //使客户端两线程同步退出
            send(conn_fd, "-exit\n", MAX_TO, 0);
            strncpy(recv_buf, users[i].username, strlen(users[i].username)-1);
   	    	strcat(recv_buf," is go away!\n");
   	    	send(users[j].fd, recv_buf, MAX_TO, 0);
   	    	//提醒在线的所有人，该用户已退出
            for(j=0; users[j].username[0] != ' '; j++) {
   	    		if((users[j].fd != -1) && (i != j)){
   	 	    		strcpy(recv_buf, users[i].username);
  	 	    		recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    		strcat(recv_buf, " is go away");
   	 	    		send(users[j].fd, recv_buf, MAX_TO, 0);
   	        	}
   	    	}         			
    	    users[i].fd = -1;
    	    pthread_exit(0);
        }
       	
       	//查看在线的信息
        if(strcmp(recv_buf, "#show\n") == 0) {
   		    for(j=0; users[j].username[0] != ' '; j++) {
   	 	    	if((users[j].fd != -1) && (i != j)){
   	 	    	    strcpy(recv_buf, users[j].username);
  	 	    	    recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    	    strcat(recv_buf, " is online");
   	 	    	    send(users[i].fd, recv_buf, MAX_TO, 0);
   	 	    	}
   	 		}		  
   		}
        
//============================================================私聊=================================================================
        if(strncmp(recv_buf, "#to ",4) == 0){
	    	for(j=4; recv_buf[j] != '\n'; j++){
	        	findname[j-4] = recv_buf[j];
	    	}
	    	findname[j++] = '\0';
	    	
	    	//查看用户是否存在
	    	for(j=0; users[j].username[0] != ' '; j++) {
        		if((strncmp(users[j].username, findname,strlen(users[j].username)-1) == 0) && (users[j].fd != -1) && (i != j)){
            	    send(users[i].fd, "服务器： it's ok,you can talk with your friend\n", MAX_TO, 0);
            	    beiyong_fd = users[j].fd;
            	    break;
        		}        	
    	    } 
    	    
    	    if(users[j].username[0] == ' '){
        	    send(users[i].fd, "服务器： don,t online or don't have the person!\n", MAX_TO, 0);
            } else{
   		    	while(1){
   		    	//防止客户端异常退出
   		    		if(recv(users[i].fd, recv_buf, MAX_TO, 0) == 0){
   		    			time (&timep);
   		    			strcpy(recv_buf, users[i].username);
   		    			recv_buf[strlen(recv_buf)-1] = '\0';
   						strcat(recv_buf, " is error go away");
    					printf("%s \n", recv_buf);
   		    			printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
   		    			
   		    			//将异常退出信息存入系统日志文件
   		    			pthread_mutex_lock(&mutex_denglu);
						fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
						sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
						write(fp, a, strlen(a));
						close(fp);
						pthread_mutex_unlock(&mutex_denglu);
						
						//将异常退出信息存入错误日志文件
						pthread_mutex_lock(&mutex_error);
						fp = open("error.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
						sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
						write(fp, a, strlen(a));
						close(fp);
						pthread_mutex_unlock(&mutex_error);
   		    			
   		    			//给在线用户提示该用户已经退出
   		    			users[i].fd = -1;
            			send(beiyong_fd, "#exit\n", MAX_TO, 0);
            			usleep(10);            		
            			for(j=0; users[j].username[0] != ' '; j++) {
   	    					if((users[j].fd != -1) && (i != j)){
   	 	 	   					strcpy(recv_buf, users[i].username);
  	 		    				recv_buf[strlen(recv_buf)-1] = '\0';
   	 		    				strcat(recv_buf, " is go away");
   	 		    				send(users[j].fd, recv_buf, MAX_TO, 0);
   	       	 				}
   	    				}         			
    	    			return ;
    	    		}
    	    		
    	    		//传输的是文件内容是直接传送给课户端，并不进行特殊处理
    	    		if(strncmp(recv_buf,"@###@",5) == 0){
    	    			send(users[j].fd, recv_buf, MAX_TO, 0);
    	    			continue;
    	    		}
    	    		
    	    		//在私聊时切换私聊对象
    	    		if(strncmp(recv_buf, "#to ",4) == 0){
	    				for(j=4; recv_buf[j] != '\n'; j++){
	        				findname[j-4] = recv_buf[j];
	    				}
	    				findname[j++] = '\0';
	    				
	    				for(j=0; users[j].username[0] != ' '; j++) {
        					if((strncmp(users[j].username, findname,strlen(users[j].username)-1) == 0) && (users[j].fd != -1)){
            	    			send(users[i].fd, "服务器： it's ok,you can talk with your friend\n", MAX_TO, 0);
            	    			beiyong_fd = users[j].fd;
            	    			break;
        					}        	
    	    			} 
    	    			
    	    			if(users[j].username[0] == ' '){
        	    			send(users[i].fd, "服务器： don,t online or don't have the person!\n", MAX_TO, 0);
        	    			break;
            			} else {
            				continue;
            			}
            		}
    	    		
    	    		//退出私聊
   		    		if(strcmp(recv_buf, "#exit\n") == 0) {
   		    			//退出私聊时，告诉该用户在县情况
   		    	    	for(j=0; users[j].username[0] != ' '; j++) {
   	 	    				if((users[j].fd != -1) && (i != j)){
   	 	    	    			strcpy(recv_buf, users[j].username);
  	 	    	    			recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    	    			strcat(recv_buf, " is online");
   	 	    	    			send(users[i].fd, recv_buf, MAX_TO, 0);
   	 	    				}
   	 	    	    	}		  
   		    	    	break;
   		    		} 
   		    		
   		    		//正常退出
   		    		else if(strcmp(recv_buf, "-exit\n") == 0){ 
   		    			
   		    			//接收客户端用户的聊天记录
   		    			users[i].fd = -1;
   		    			strcpy(a,users[i].username);
        				a[strlen(a)-1] = '\0';
        				strcat(a,".txt");
        				fd = open(a, O_WRONLY|O_CREAT|O_TRUNC, 0777);
        				close(fd);
        				fd = open(a, O_WRONLY|O_APPEND|O_CREAT, 0777);
        				while(1){
        					if(recv(conn_fd, recv_buf, 499, 0) != 499){
        						break;
        					}
        					j = strlen(recv_buf);   		
        					if( (write(fd, recv_buf, j)) != 499){
        						break;
        					}
        				}
        				close(fd);
        				
   		    			//服务器显示用户下线
   		    			time (&timep);
   		    			strcpy(recv_buf, users[i].username);
            			recv_buf[strlen(recv_buf)-1] = '\0';
   						strcat(recv_buf, " is go away");
    					printf("%s \n", recv_buf);
            			printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
            			
            			//将用户退出的信息写入日志文件中
            			pthread_mutex_lock(&mutex_denglu);
						fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
						sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
						write(fp, a, strlen(a));
						close(fp);
						pthread_mutex_unlock(&mutex_denglu);
            			
            			//告诉所有玩家该用户已退出
   		    	    	for(j=0; users[j].username[0] != ' '; j++) {
   	 	    				if((users[j].fd != -1) && (i != j)){
   	 	    	    			strcpy(recv_buf, users[i].username);
  	 	    	    			recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    	    			strcat(recv_buf, " is go away!!!!!!!!!");
   	 	    	    			send(users[j].fd, recv_buf, MAX_TO, 0);
   	 	    				}
   	 	    			}  
   	 	    			
   	 	    			send(beiyong_fd, "#exit\n", MAX_TO, 0);       			
    	    			users[i].fd = -1;
    	    			return ;
        			}else if(strcmp(recv_buf, "#show\n") == 0) {
   		    			//告诉用户在线情况
   		    	    	for(j=0; users[j].username[0] != ' '; j++) {
   	 	    				if((users[j].fd != -1) && (i != j)){
   	 	    	    			strcpy(recv_buf, users[j].username);
  	 	    	    			recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    	    			strcat(recv_buf, " is online");
   	 	    	    			send(users[i].fd, recv_buf, MAX_TO, 0);
   	 	    				}
   	 	    	    	}		  
   		    	    	continue;
   		    		}
   		    		
   		    		//无特殊情况时，进行普通处理 
   		    		usleep(10);
   		    		recv_buf[strlen(recv_buf)-1] = '\0';
   		    		strcpy(send_buf, users[i].username);
   		    		send_buf[strlen(users[i].username) - 1] = '\0';
   		    		strcat(send_buf, ": ");
   		    		strcat(send_buf, recv_buf);
   		    		
   		    		//若发现私聊对象已退出时，告诉该用户情况   		    		
   		    		if(send(users[j].fd, send_buf, MAX_TO, 0) < 0){
   		    			printf("error!\n");
   		    			
   		    			pthread_mutex_lock(&mutex_error);
						fp = open("error.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
						sprintf(a, "%s 已退出，无法通话！\n %s\n", users[j].username,ctime(&timep));
						write(fp, a, strlen(a));
						close(fp);
						pthread_mutex_unlock(&mutex_error);
   		    			
   		    			send(users[i].fd,"该用户已退出，现已退出与其对话，请重新输入！\n", MAX_TO, 0);
   		    			usleep(10);
   		    			send(users[i].fd, "#exit\n", MAX_TO, 0);
   		    		}
   		    		usleep(10);
   		    	
   		    	}
            }   	           
        }
        
//=================================================================群聊 =======================================================        
        if(strcmp(recv_buf, "#all\n") == 0){
        	send(conn_fd, "成功进入群聊\n", MAX_TO, 0);
   		    while(1){
   		    	//防止异常退出
   		    	if(recv(users[i].fd, recv_buf, MAX_TO, 0) == 0){
   		    		time (&timep);
   		    		strcpy(recv_buf, users[i].username);
   		    		recv_buf[strlen(recv_buf)-1] = '\0';
   					strcat(recv_buf, " is  error go away");
    				printf("%s \n", recv_buf);
   		    		printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
   		    		
   		    		//将异常推出信息存入日志文件中
   		    		pthread_mutex_lock(&mutex_denglu);
					fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
					sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
					write(fp, a, strlen(a));
					close(fp);
					pthread_mutex_unlock(&mutex_denglu);
					
					//将异常退出信息存入错误日志中
					pthread_mutex_lock(&mutex_error);
					fp = open("error.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
					sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
					write(fp, a, strlen(a));
					close(fp);
					pthread_mutex_unlock(&mutex_error);
            		
            		send(conn_fd, "-exit\n", MAX_TO, 0);
            		
            		//提醒在线用户该用户已退出
            		for(j=0; users[j].username[0] != ' '; j++) {
   	    				if((users[j].fd != -1) && (i != j)){
   	 	 	    			strcpy(recv_buf, users[i].username);
  	 		    			recv_buf[strlen(recv_buf)-1] = '\0';
   	 		    			strcat(recv_buf, " is go away!!!!!!!!!");
   	 		    			send(users[j].fd, recv_buf, MAX_TO, 0);
   	       	 			}
   	    			}        			
    	    		users[i].fd = -1;
    	    		return ;
    	    	}
   		    	
   		    	//退出群聊
   		    	if(strcmp(recv_buf, "#exit\n") == 0) {
   		    		//提醒该用户在线的情况
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
   		    	
   		    	//用户下线
   		    	else if(strcmp(recv_buf, "-exit\n") == 0){
   		    		//将该用户聊天记录文件发送给服务器端
   		    		strcpy(a,users[i].username);
        			a[strlen(a)-1] = '\0';
        			strcat(a,".txt");
        			fd = open(a, O_WRONLY|O_CREAT|O_TRUNC, 0777);
        			close(fd);
        			fd = open(a, O_WRONLY|O_APPEND|O_CREAT, 0777);
        			while(1){
        				if(recv(conn_fd, recv_buf, 499, 0) != 499){
        					break;
        				}
        				j = strlen(recv_buf);  		
        				if( (write(fd, recv_buf, j)) != 499){
        					break;
        				}
        			}
        			close(fd);
   		    		  		    		
   		    		//将用户离线消息在服务器端显示
   		    		time (&timep);
   		    		strcpy(recv_buf, users[i].username);
            		recv_buf[strlen(recv_buf)-1] = '\0';
   					strcat(recv_buf, " is go away");
    				printf("%s \n", recv_buf);
            		printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
            		
            		//将用户离线消息存入日志文件中
            		pthread_mutex_lock(&mutex_denglu);
					fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
					sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
					write(fp, a, strlen(a));
					close(fp);
					pthread_mutex_unlock(&mutex_denglu);
            		
            		//提醒所有在先用户该用户已退出
            		send(conn_fd, "-exit\n", MAX_TO, 0);
            		strncpy(recv_buf, users[i].username, strlen(users[i].username)-1);
   		    	    strcat(recv_buf," is go away!\n");
   		    	    send(users[j].fd, recv_buf, MAX_TO, 0);
   		    	    
   		    	    for(j=0; users[j].username[0] != ' '; j++) {
   	 	    			if((users[j].fd != -1) && (i != j)){
   	 	    	    		strcpy(send_buf, users[i].username);
   	 	    	    		send_buf[strlen(users[i].username) - 1] = '\0';
   		    				strcat(send_buf, ": ");
   		    				strcat(send_buf, recv_buf);
   	 	    	    		send(users[j].fd, send_buf, MAX_TO, 0);
   	 	    			}
   	 	    		}         			
    	    			users[i].fd = -1;
    	    			return ;
        		}  else if (strcmp(recv_buf, "#show\n") == 0) {
   		    			//告诉用户在线情况
   		    	    	for(j=0; users[j].username[0] != ' '; j++) {
   	 	    				if((users[j].fd != -1) && (i != j)){
   	 	    	    			strcpy(recv_buf, users[j].username);
  	 	    	    			recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    	    			strcat(recv_buf, " is online");
   	 	    	    			send(users[i].fd, recv_buf, MAX_TO, 0);
   	 	    				}
   	 	    	    	}		  
   		    	    	continue;
   		    	}
   		    	
   		    	//普通群聊消息
   		    	usleep(10);
   		    	strcpy(send_buf, " 群聊消息，来自 ");
   		    	strcat(send_buf, users[i].username);
   		    	send_buf[strlen(send_buf)-1] = '\0';
   		    	strcat(send_buf, ": ");
   		    	strcat(send_buf, recv_buf);
   		    	
   		    	for(j=0; users[j].username[0] != ' '; j++) {
   	 	    		if((users[j].fd != -1) && (i != j)){
   	 	    	    		send(users[j].fd, send_buf, MAX_TO, 0);
   	 	    		}
   	 	    	}
   		    	usleep(10);
            }   	           
        }
        
//===========================================================修改密码=========================================================       
        if(strcmp(recv_buf, "#xiugai\n") == 0){
            send(users[i].fd, "#xiugai\n", MAX_TO, 0);
            recv(users[i].fd, recv_buf, MAX_TO, 0);
            strcat(recv_buf, "\n");
            strcpy(users[i].password, recv_buf);
            Write();
        }
//==========================================================离线消息==========================================================
    	if(strncmp(recv_buf,"#lixianto ", 10) == 0){
    		for(j=10; recv_buf[j] != '\0'; j++){
    			filename_lixian[j-10] = recv_buf[j];
    		}
    		filename_lixian[j-10] = '\0';
    		puts(filename_lixian);
    		if(((m = find_delete(filename_lixian)) != -1) && (users[m].fd == -1)){    			
    			strcat(filename_lixian, "-1.txt");
    			while(1){
    				if(recv(users[i].fd, recv_buf, MAX_TO, 0) == 0){
    					if(users[m].fd != -1){
    						send(users[i].fd, "对方以上线！\n",500,0);
    						break;
    					}
   		    			time (&timep);
   		    			strcpy(recv_buf, users[i].username);
   		    			recv_buf[strlen(recv_buf)-1] = '\0';
   						strcat(recv_buf, " is  error go away");
    					printf("%s \n", recv_buf);
   		    			printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
   		    		
   		    			//将异常推出信息存入日志文件中
   		    			pthread_mutex_lock(&mutex_denglu);
						fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
						sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
						write(fp, a, strlen(a));
						close(fp);
						pthread_mutex_unlock(&mutex_denglu);
					
						//将异常退出信息存入错误日志中
						pthread_mutex_lock(&mutex_error);
						fp = open("error.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
						sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
						write(fp, a, strlen(a));
						close(fp);
						pthread_mutex_unlock(&mutex_error);
            		
            			send(conn_fd, "-exit\n", MAX_TO, 0);
            		
            			//提醒在线用户该用户已退出
            			for(j=0; users[j].username[0] != ' '; j++) {
   	    					if((users[j].fd != -1) && (i != j)){
   	 	 	    				strcpy(recv_buf, users[i].username);
  	 		    				recv_buf[strlen(recv_buf)-1] = '\0';
   	 		    				strcat(recv_buf, " is go away!!!!!!!!!");
   	 		    				send(users[j].fd, recv_buf, MAX_TO, 0);
   	       	 				}
   	    				}        			
    	    			users[i].fd = -1;
    	    			return ;
    	    		}
    				if(strcmp(recv_buf, "#exit\n") == 0) {
    					break;
    				}
    				else if(strcmp(recv_buf, "-exit\n") == 0){
   		    			//将该用户聊天记录文件发送给服务器端
   		    			strcpy(a,users[i].username);
        				a[strlen(a)-1] = '\0';
        				strcat(a,".txt");
        				fd = open(a, O_WRONLY|O_CREAT|O_TRUNC, 0777);
        				close(fd);
        				fd = open(a, O_WRONLY|O_APPEND|O_CREAT, 0777);
        				while(1){
        					if(recv(conn_fd, recv_buf, 499, 0) != 499){
        						break;
        					}
        					j = strlen(recv_buf);  		
        					if( (write(fd, recv_buf, j)) != 499){
        						break;
        					}
        				}
        				close(fd);
   		    		  		    		
   		    			//将用户离线消息在服务器端显示
   		    			time (&timep);
   		    			strcpy(recv_buf, users[i].username);
            			recv_buf[strlen(recv_buf)-1] = '\0';
   						strcat(recv_buf, " is go away");
    					printf("%s \n", recv_buf);
            			printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
            		
            			//将用户离线消息存入日志文件中
            			pthread_mutex_lock(&mutex_denglu);
						fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
						sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
						write(fp, a, strlen(a));
						close(fp);
						pthread_mutex_unlock(&mutex_denglu);
            			
            			//提醒所有在先用户该用户已退出
            			send(conn_fd, "-exit\n", MAX_TO, 0);
            			strncpy(recv_buf, users[i].username, strlen(users[i].username)-1);
   		    		    strcat(recv_buf," is go away!\n");
   		    		    send(users[j].fd, recv_buf, MAX_TO, 0);
   		    	    
   		    		    for(j=0; users[j].username[0] != ' '; j++) {
   	 	    				if((users[j].fd != -1) && (i != j)){
   	 	   		 	    		strcpy(send_buf, users[i].username);
   	 	   		 	    		send_buf[strlen(users[i].username) - 1] = '\0';
   		    					strcat(send_buf, ": ");
   		    					strcat(send_buf, recv_buf);
   	 	    		    		send(users[j].fd, send_buf, MAX_TO, 0);
   	 	   		 			}
   	 	    			}         			
    	    				users[i].fd = -1;
    	    				return ;
        			}
        			strcpy(a,users[i].username);
        			a[strlen(a)-1] = '\0';
        			strcat(a,"'s unline： ");
        			strcat(a,recv_buf);
        			k = strlen(a);
        			if(k >= 100){
        				send(users[i].fd,"数据过长!\n",500,0);
        				continue;
        			}
        			for(k; k < 100; k++){
        				a[k] = ' ';
        			}
        			fp = open(filename_lixian,O_WRONLY|O_CREAT|O_APPEND, 0777);
        			write(fp, a, 100);
        			close(fp);
    			}
    		} else {
    			send(users[i].fd, "don't have the person\n", MAX_TO, 0);
    		}
    	}
    }	   
    users[i].fd = -1;
    return ;
}

//============================================================主函数============================================================
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
    int 	j=0;
    pthread_t thid;
    time_t 	timep;
    int 	fp;
    char	a[200];  

	//初始化用户数组并读取    
    Memster();
    Read();
    
    //创建套接字
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0) {
        my_err("socket", __LINE__);
    }

    optval = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(int)) < 0) {
        my_err("setsockopt", __LINE__);
    }
	
	//初始化套接字
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_aton("0.0.0.0", &serv_addr.sin_addr);

    if( bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) < 0) {
        my_err("bind", __LINE__);
    }
    
    //初始系统日志文件互斥锁
    pthread_mutex_init(&mutex_denglu,NULL);
    //初始化错误日志文件互斥锁
    pthread_mutex_init(&mutex_error,NULL);

	//处理每个连接的客户端
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

        if(conn_fd[j] < 0){
            my_err("accept", __LINE__);
        }
        time (&timep);
		printf("accept a new client, ip:%s\n", inet_ntoa(cli_addr.sin_addr));
		printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
		
		pthread_mutex_lock(&mutex_denglu);
		fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
		sprintf(a, "accept a new client,ip %s\n %s\n",inet_ntoa(cli_addr.sin_addr),ctime(&timep));
		write(fp, a, strlen(a));
		close(fp);
		pthread_mutex_unlock(&mutex_denglu);
		
		pthread_create(&thid, NULL, (void*)do_it, (int *)conn_fd[j] );
    }
    close(sock_fd);
    Write();
    
    return 0;
}
//======================================================END======================================================
