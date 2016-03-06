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
#define MAX_TO 100
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
//用户结构体
struct userinfo {
    char username[32];
    //用户帐号
    char password[32];
    //用户密码
    int fd;
    //用户套接字
    char qunname[5][32];
};
struct userinfo users[100];
//群结构体
struct data {
    char name[32];
    //群名
    int user[30];
    //群里人的编号
};
struct data qun[30];

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
void guanli_delete(int conn_fd);
//管理员删除用户的函数
void guanli_look(int conn_fd);
//管理员查看用户记录的函数
void denglu_sucess(int i, int conn_fd);
//普通用户登录成功后处理的函数
int usual_denglu(int conn_fd);
//选择以普通用户登录函数
void my_zhuce(int conn_fd);
//选择注册后需要进的函数
int my_data(int conn_fd);
//解析客户端开始界面的选择的函数
void error_goaway(int i, char *a,char *recv_buf);
//服务器显示推出的函数
void goaway_keep(char *recv_buf);
//将退出信息存入系统日志中的函数
void error_keep(char *recv_buf);
//将错误信息存入错误日志中的函数
void goaway_talk_all(int conn_fd,int i);
//向在线用户提示当前用户退出的信息的函数
void recv_jilu(int i, int conn_fd);
//用户退出时接收其聊天记录文件
void all_online(char *recv_buf,int i);
//告诉当前用户在线情况的函数
int usual_mingling(int conn_fd,int i, char *recv_buf);
//普通用户进入后的命令解析函数
int siliao_sucess(int i, int conn_fd, int j, int beiyong_fd, char *findname);
//私聊成功进入后的消息解析函数
int my_siliao(char *recv_buf, int i, int conn_fd);
//判断私聊命令是否成功的函数
int my_qunliao(int conn_fd, int i);
//群聊函数
int my_lixian(int i, int conn_fd, char *recv_buf);
//离线消息的函数
int my_qun(int i, int conn_fd, char recv_buf[100]);
//建群的群聊
void show_qun(int k, int conn_fd);
//显示自己的群
void show_person(int conn_fd, int i, char recv_buf[100]);
//显示群人员
void delete_qun(int conn_fd, int i, char recv_buf[100]);
//删除群
void goaway_qun(int conn_fd, int i, char recv_buf[100]);
//退出群
void add_qun(int conn_fd, int i, char recv_buf[100] );
//加群
void create_qun(int conn_fd, int i, char recv_buf[100] );
//创建群的函数

//===================================================初始化用户数组=================================
void Memster()
{
    int i;
    int j;

    for(i=0; i<100; i++) {
    	strcpy(users[i].username, " ");
    	users[i].fd = -1; 
        for(j=0 ; j<5; j++){
            strcpy(users[i].qunname[j], "\n");
        }
    }
    for(i=0; i<30; i++){
        strcpy(qun[i].name, "\n");
        for(j=0; j<30; j++){
            qun[i].user[j] = -1;
        }
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
	int i=0, j;
    int fp;
    //存储用户信息的文件的文件描述符
	
	fp=open("user.txt",O_RDONLY|O_CREAT,0777);
	
	//存储用户信息的文件为空特殊处理
	//if(read(fp,&users[0],sizeof(struct userinfo))==0) {
    //    close(fp);
	//	return ;
	//}
    //for(j=0 ; j<5; j++){
    //   strcpy(users[0].qunname[j], "\n");
    //}
	//users[0].fd = -1;
	
	//循环读取用户数组
	while(1){
		if(read(fp,&users[i],sizeof(struct userinfo)) == 0)
			break;
        //for(j=0 ; j<5; j++){
        //    strcpy(users[i].qunname[j], "\n");
        //}
		users[i++].fd = -1;
	}
	close(fp);

    fp = open("qun.txt", O_RDONLY|O_CREAT, 0777);
    i=0;
    if(read(fp, &qun[i], sizeof(struct data)) == 0){
        close(fp);
        for(i=0; i<30; i++){
            strcpy(qun[i].name, "\n");
            for(j=0; j<30; j++){
                qun[i].user[j] = -1;
            }
        }
        return ;
        Write();
    }
    
    for(i=1; i<30; i++){
        read(fp, &qun[i], sizeof(struct data));
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
	
	for(i=0; users[i].username[0] != ' '; i++) {
		write(fp,&users[i],sizeof(struct userinfo));
	}
	close(fp);

    fp=open("qun.txt", O_WRONLY|O_CREAT|O_TRUNC, 0777);

    for(i=0 ;i<30; i++){
        write(fp, &qun[i],sizeof(struct data));
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

//============================================================管理员删除用户=============================================
void guanli_delete(int conn_fd)
{
	char recv_buf[100];
	int i;
    int j;

		while(1){
			if(recv(conn_fd, recv_buf, MAX_TO, 0) == 0){         			
    			break;
    			//用于防止客户端异常下线
    		}
    		
    		if(strcmp(recv_buf, "-exit\n") == 0){
    			break;
    			//收到-exit则为客户端正常下线
    		}
    		
    		if((i = find_delete(recv_buf)) > -1){
    			//删除其聊天记录
    			strcpy(recv_buf, users[i].username);
    			recv_buf[strlen(recv_buf)-1] = '\0';
    			strcat(recv_buf,".txt");
    			rename(recv_buf,"#nouse#.txt");
    			
    			//若fd ！= -1， 则代表其在线，需先将其踢下线再进行删号
    			if(users[i].fd != -1){   		
    				send(users[i].fd, "你已经被管理员踢出！！！", MAX_TO, 0);
    				send(users[i].fd, "-exit\n", MAX_TO, 0);
    			}
    			
    			//将帐号密码更新为\n则为删除
    			strcpy(users[i].username,"\n");
    			strcpy(users[i].password,"\n");
                for(j=0 ; j<5; j++){
                    strcpy(users[i].qunname[j], "\n");
                }
    			Write();
    			send(conn_fd, "sucess delete!\n", MAX_TO, 0);
    		} else {
    			send(conn_fd, "don't have the person!\n", MAX_TO, 0);
    		}	
    	}
}

//=========================================================管理员查看聊天记录======================================================= 
void guanli_look(int conn_fd)
{
	int k;
	DIR *dir;
	struct dirent *ptr;
	int i;
	char recv_buf[100];
	int fd;
	int a[200];
	
	while(1){
    	if((k=recv(conn_fd, recv_buf, 32, 0)) == 0){
    		break;
    		//用于防止客户端异常下线
    	}
    	
    	if(k == -1){
    		break;
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
					i = read(fd, a, 99);
					a[i] = '\0';
					send(conn_fd, a, 99,0);
					memset(a,0,sizeof(a));
					if(i != 99){				
						break;
					}								
				}
				close(fd);
	}
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
	int i,k,j;
	int fd;
	//聊天记录文件的描述符
	char a[32];
	
	//接收管理员的用户名与密码进行比较，正确返回y，错误返回n
	i = recv(conn_fd, name, MAX_TO, 0);
	j = recv(conn_fd, pass, MAX_TO, 0);
	if((i == 0)||(j == 0)){
		return ;
	}
	if((strcmp(name, all_name) == 0) && (strcmp(pass, all_pass) == 0))
	{
		send(conn_fd,"y\n", MAX_TO, 0);
	} else{
		send(conn_fd,"n\n", MAX_TO, 0);
		return ;
	}
	
	if(recv(conn_fd, recv_buf, 32, 0) <= 0)
	{
		printf("guanli go away!\n");
		return ;
	}
	//接受客户端发来的操作提醒
		
	//recv_buf若为delete，则选择删除用户，若未jilu，则选择查看聊天记录
	if(strcmp(recv_buf, "delete") == 0){
		//调用管理员删除用户的函数
		guanli_delete(conn_fd);
    } else if (strcmp(recv_buf, "jilu") == 0){
    	recv(conn_fd, recv_buf, 32, 0);
    	//调用管理员查看聊天记录函数
    	guanli_look(conn_fd);
    }
    return ;
}

//==========================================================登录成功后的处理===================================================
void denglu_sucess(int i, int conn_fd)
{
	time_t timep;
	int j;
	int fp;
	char a[200];
	char recv_buf[100];
	int fd1;	
	DIR *dir;
	struct dirent *ptr;
	
	if(send (conn_fd, "服务器： welcome to it !", 100, 0) < 0) {
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
			send(users[i].fd, a, 100,0);
			memset(a,0,sizeof(a));					
		}
		close(fd1);
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
}

//====================================================选择以普通用户进入======================================================
int usual_denglu(int conn_fd)
{
	char username[32];
    //接收到的用户（包括管理员的用户名）
    char userpassword[32];
    //接收到的密码（包括管理员的密码）
	int i;    
    time_t timep;
    int fp;
    char a[100];
    
	//接收用户名和密码，并判断对错
    if(recv(conn_fd, username, 32, 0) == 0){
       	printf("error go away!\n");
       	return -1;
    }
   	if(recv(conn_fd, userpassword, 32, 0) == 0){
		printf("error go away!\n");
       	return -1;	
    }		

    if((i = find_user(username,userpassword)) > -1){
    	if(users[i].fd == -1){
   	 	    users[i].fd = conn_fd;
   		    send(conn_fd, "y\n", 3, 0);
			denglu_sucess(i,conn_fd);
 	   	    return i;
 	   	}
 	} 
 	   			
 	//登陆失败，在服务器端显示，并保存信息于错误日志中
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
	
	return -1;
}

//====================================================注册======================================================
void my_zhuce(int conn_fd)
{
	char username[32];
    //接收到的用户（包括管理员的用户名）
    char userpassword[32];
    //接收到的密码（包括管理员的密码）
    time_t timep;
    int i;
    char recv_buf[100];
    int fp;
    char a[100];
    
		//获取注册的用户名，密码
       	    if(recv(conn_fd, username, 32, 0) == 0){
       	    	printf("error zhuce!\n");
       	    	return ;
       	    }
       	    time (&timep);
    	   	printf("%d zhuce username is %s", conn_fd, username);
    	    printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
    	    if(recv(conn_fd, userpassword, 32, 0) == 0){
				printf("error zhuce!\n");
       	    	return ;		
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

//==========================================第一个大循环相当于第一个界面的选择所发过来的数据的解析=================================    
int my_data(int conn_fd)
{
	int i;
	char recv_buf[100];	
	//接受的数据
	char username[32];
    //接收到的用户（包括管理员的用户名）
    char userpassword[32];
    //接收到的密码（包括管理员的密码）
    time_t timep;
    int fp;
    char a[100];
		
    while(1){
    	//选择退出或异常退出
        if(recv(conn_fd, recv_buf, MAX_TO, 0) == 0){         			
    	    //users[i].fd = -1;
    	    return -1;
    	}
    	
    	//选择以管理员身份进入
    	if(strcmp(recv_buf,"guanli") == 0){
    		my_guanli(conn_fd);
    		return -1;
    	}
    	
    	//选择以普通身份进入
    	if(strcmp(recv_buf,"denglu") == 0){
    		if((i = usual_denglu(conn_fd)) != -1){
    			break;
    		}
 		}
 		
 		//选择注册
 		else if(strcmp(recv_buf,"zhuce") == 0) {
 			my_zhuce(conn_fd);
    	}
    }
    return i;
}

//==================================================服务器显示退出信息=======================================================
void error_goaway(int i, char *a,char *recv_buf)
{
	time_t timep;	
	
	time (&timep);
    strcpy(recv_buf, users[i].username);
  	recv_buf[strlen(recv_buf)-1] = '\0';
   	strcat(recv_buf, a);
    printf("%s \n", recv_buf);
    printf("\033[01;34m       %s\033[0m\n",ctime(&timep));
}


//================================================将推出的信息保存在系统日志中============================================
void goaway_keep(char *recv_buf)
{
	int fp;
	char a[100];
	time_t timep;	
	
	pthread_mutex_lock(&mutex_denglu);
	fp = open("denglu.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
	sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
	write(fp, a, strlen(a));
	close(fp);
	pthread_mutex_unlock(&mutex_denglu);
}

//================================================将异常推出的信息保存在错误日志中===========================================
void error_keep(char *recv_buf)
{
	int fp;
	char a[100];
	time_t timep;

	pthread_mutex_lock(&mutex_error);
	fp = open("error.txt",O_WRONLY|O_CREAT|O_APPEND, 0777);
	sprintf(a, "%s\n %s\n",recv_buf,ctime(&timep));
	write(fp, a, strlen(a));
	close(fp);
	pthread_mutex_unlock(&mutex_error);
}

//=========================================向所有在线用户发送该用户退出的信息================================================
void goaway_talk_all(int conn_fd,int i)
{
	int j;
	char recv_buf[100];

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
}

//===============================================/接收退出用户的聊天记录文件==============================================
void recv_jilu(int i, int conn_fd)
{
	char a[100];
	int fd;
	char recv_buf[100];
	int j;

	users[i].fd = -1;
    //接收退出用户的聊天记录文件
    strcpy(a,users[i].username);
    a[strlen(a)-1] = '\0';
    strcat(a,".txt");
    fd = open(a, O_WRONLY|O_CREAT|O_TRUNC, 0777);
    close(fd);
    fd = open(a, O_WRONLY|O_APPEND|O_CREAT, 0777);
    while(1){
        if(recv(conn_fd, recv_buf, 99, 0) != 99){
        	break;
        }
        j = strlen(recv_buf);    		
        if( (write(fd, recv_buf, j)) != 99){
        	break;
        }
    }
    close(fd);
}

//==================================================告诉用户在线情况====================================================
void all_online(char *recv_buf,int i)
{
	int j;

   	for(j=0; users[j].username[0] != ' '; j++) {
   	 	if((users[j].fd != -1) && (i != j)){
   	 	   	strcpy(recv_buf, users[j].username);
  	 	    recv_buf[strlen(recv_buf)-1] = '\0';
   	 	    strcat(recv_buf, " is online");
   	 	    send(users[i].fd, recv_buf, MAX_TO, 0);
   	 	}
   	}		  
}

//=================================================创建群的函数======================================================
void create_qun(int conn_fd, int i, char recv_buf[100] )
{
    int j;
    int n;

    for(j=8; recv_buf[j] != '\n'; j++){
        recv_buf[j-8] = recv_buf[j];
    }
    recv_buf[j-8] = '\0';
    for(j=0; j<30; j++){
        if(strcmp(recv_buf, qun[j].name) == 0){
            break;
        }
    }
    if(j == 30){
        for(j=0; (qun[j].name[0]!='\n') && (j<30); j++ );
        if(j == 30){
            send(conn_fd, "qun number is too big!", 100, 0);
        } else {
            for(n=0; n<5; n++){
                if(strcmp(users[i].qunname[n],"\n") == 0){
                    break;
                } 
            }
            if(n == 5){
                send(conn_fd, "one person can only join 5 qun!", 100, 0);
                return ;
            }
            send(conn_fd, "sucess create qun!", 100, 0);
            strcpy(qun[j].name, recv_buf);
            qun[j].user[0] = i;
            strcpy(users[i].qunname[n],recv_buf);
            puts(qun[j].name);
            Write();
        }

    } else{
        send(conn_fd, "have the qun", 100, 0);
    }
    
}

//=====================================================加群=======================================================
void add_qun(int conn_fd, int i, char recv_buf[100] )
{
    int j;
    int k;
    int n;

    for(j=5; recv_buf[j] != '\n'; j++){
        recv_buf[j-5] = recv_buf[j];
    }
    recv_buf[j-5] = '\0';

    for(j=0; j<30; j++){
        if(strcmp(recv_buf, qun[j].name) == 0){
            break;
        }
            return ;
    }

    if(j < 30){
        for(k=0; k<30; k++){
            if(qun[j].user[k] == i){
                send(conn_fd, "you in the qun!", 100, 0);
                return ;
            }
        }
        for(k=0; (qun[j].user[k] != -1) && (k < 30); k++);
        if(k == 30){
            send(conn_fd, "qun person number is to big!", 100, 0);
            return ;
        }
        for(n=0; n<5; n++){
            if(strcmp(users[i].qunname[n],"\n") == 0){
                break;
            } 
        }
        if(n == 5){
            send(conn_fd, "one person can only join 5 qun!", 100, 0);
            return ;
        }
        send(conn_fd, "it,s ok!", 100, 0);
        qun[j].user[k] = i;
        strcpy(users[i].qunname[n], recv_buf);
        Write();
    } else{
        send(conn_fd, "don,t have the qun!", 100, 0);
    }
}

//=====================================================推出群==========================================================
void goaway_qun(int conn_fd, int i, char recv_buf[100])
{
    int j;
    int k;
    int n;

    for(j=8; recv_buf[j] != '\n'; j++){
        recv_buf[j-8] = recv_buf[j];
    }
    recv_buf[j-8] = '\0';

    for(j=0; j<30; j++){
        if(strcmp(recv_buf, qun[j].name) == 0){
            break;
        }
            return ;
    }

    if(j < 30){
        for(k=0; (qun[j].user[k] != -1) && (k < 30); k++);
        if(k == 30){
            send(conn_fd, "qun person number is to big!", 100, 0);
            return ;
        }
        for(k=0; k<30; k++){
            if(qun[j].user[k] == i){
                break ;
            }
        }
        if(k == 30){
            send(conn_fd, "you don,t in the qun!", 100, 0);
            return ;
        }
        if(k == 0){
            send(conn_fd, "you are the master,can,t goaway!", 100, 0);
            return ;
        }
        send(conn_fd, "it,s ok!", 100, 0);
        qun[j].user[k] = -1;
        for(n=0; n<5; n++){
            if(strcmp(users[i].qunname[n],recv_buf) == 0){
                break;
            } 
        }
        if(n == 5){
            printf("error !\n");
        } 
        strcpy(users[i].qunname[n], "\n");
        Write();
    } else{
        send(conn_fd, "don,t have the qun!", 100, 0);
    }
}

//=====================================================删除群==========================================================
void delete_qun(int conn_fd, int i, char recv_buf[100])
{
    int j;
    int k;
    int n;

    for(j=8; recv_buf[j] != '\n'; j++){
        recv_buf[j-8] = recv_buf[j];
    }
    recv_buf[j-8] = '\0';

    for(j=0; j<30; j++){
        if(strcmp(recv_buf, qun[j].name) == 0){
            break;
        }
            return ;
    }

    if(j < 30){
        for(k=0; (qun[j].user[k] != -1) && (k < 30); k++);
        if(k == 30){
            send(conn_fd, "qun person number is to big!", 100, 0);
            return ;
        }
        for(k=0; k<30; k++){
            if(qun[j].user[k] == i){
                break ;
            }
        }
        if(k != 0){
            send(conn_fd, "you are not the master!", 100, 0);
            return ;
        }
        send(conn_fd, "it,s ok!", 100, 0);
        for(k=0; k<30; k++){
            if(qun[j].user[k] == -1){
                continue;
            }
            for(n=0; n<5; n++){
                if(strcmp(users[qun[j].user[k]].qunname[n], recv_buf)==0){
                    break;
                }
            }
            strcpy(users[qun[j].user[k]].qunname[n],"\n");
            qun[j].user[k] = -1;
        }
        strcpy(qun[j].name, "\n");
        Write();
    } else{
        send(conn_fd, "don,t have the qun!", 100, 0);
    }
}

//=====================================================显示群人员=======================================================
void show_person(int conn_fd, int i, char recv_buf[100])
{
    int j;
    int k;
    int n;
printf("1\n");

    for(j=12; recv_buf[j] != '\n'; j++){
        recv_buf[j-12] = recv_buf[j];
    }
    recv_buf[j-12] = '\0';

    for(j=0; j<30; j++){
        if(strcmp(recv_buf, qun[j].name) == 0){
            break;
        }
            return ;
    }

    if(j < 30){
        for(k=0; (qun[j].user[k] != -1) && (k < 30); k++);
        if(k == 30){
            send(conn_fd, "qun person number is to big!", 100, 0);
            return ;
        }
        for(k=0; k<30; k++){
            if(qun[j].user[k] == i){
                break ;
            }
        }
        if(k == 30){
            send(conn_fd, "you don,t in the qun!", 100, 0);
            return ;
        }
        send(conn_fd, "#show_person\n", 100, 0);
        for(n=0; n<30; n++){
            if(qun[j].user[n] != -1){
                if(users[qun[j].user[n]].fd != -1){
                    strcpy(recv_buf, users[qun[j].user[n]].username);
                    strcat(recv_buf, " is online\n");
                    send(conn_fd, recv_buf, 100, 0);
                }
                if(users[qun[j].user[n]].fd == -1){
                    strcpy(recv_buf, users[qun[j].user[n]].username);
                    strcat(recv_buf, " is not online\n");
                    send(conn_fd, recv_buf, 100, 0);
                }
                usleep(10000);
            }
        }
        send(conn_fd, "#it,s over\n", 100, 0);
        send(conn_fd, "it,s   over\n", 100, 0);
    } else{
        send(conn_fd, "don,t have the qun!", 100, 0);
    }
}

//=====================================================现实自己的群=====================================================
void show_qun(int k, int conn_fd)
{
    int i;
    int j;
    char a[100];

    for (i=0; i<5; i++){
        for(j=0; (users[k].qunname[i][j] != '\0') && j<30; j++);
        if(j == 1)
            break;
        sprintf(a, "%d: %s", i+1, users[k].qunname[i]);
        send(conn_fd, a, 100, 0);
    }
    //for(i=0; i<30; i++){
    //    printf("%d: %s\n", i+1, qun[i].name);
    //    for(j=0; j<30; j++){
    //        printf("%d(%d)   ", j+1,qun[i].user[j]);
    //    }
    //    printf("\n");
    //}
}

//===========================================以普通用户进入后的解析======================================================== 
int usual_mingling(int conn_fd,int i, char *recv_buf)
{	
	//接受的数据
    time_t timep;
    int fp;
    int fd;
    int j;
    char a[100];
    
		//防止用户异常退出
    	if(recv(conn_fd, recv_buf, MAX_TO, 0) == 0){
    			//服务器显示退出信息
        printf("%d\n", j);
    			error_goaway(i, " is error go away", recv_buf);
    			
    			//将异常推出的信息保存在系统日志中
    			goaway_keep(recv_buf);
				
				//将异常推出的信息保存在错误日志中
				error_keep(recv_buf);
            	
            	//向所有在线用户发送该用户退出的信息
            	goaway_talk_all(conn_fd, i);
    	    	return -1;
    	}
        
    	//正常退出
        if(strcmp(recv_buf, "-exit\n") == 0){
    	    recv_jilu(i, conn_fd);
        	
        	//服务器显示错误提示
           	error_goaway(i, " is go away", recv_buf);
            
            //将退出的信息保存在日志文件中
            goaway_keep(recv_buf);
            
            //使客户端两线程同步退出
           // send(conn_fd, "-exit\n", MAX_TO, 0);	    	
   	    	//提醒在线的所有人，该用户已退出
            goaway_talk_all(conn_fd, i);
    	    return -1;
        }
       	
       	//查看在线的信息
       	if(strcmp(recv_buf, "#show\n") == 0) {
        	all_online(recv_buf, i);
        }

    if(strncmp(recv_buf, "#create ", 8) == 0){
        create_qun(conn_fd, i, recv_buf);
    }

    if(strncmp(recv_buf, "#add ", 5) == 0){
        add_qun(conn_fd, i, recv_buf);
    }
    if(strcmp(recv_buf, "#showqun\n") == 0){
        show_qun(i, conn_fd);
    }
    if(strncmp(recv_buf, "#showperson ", 12) == 0){
        show_person(conn_fd, i, recv_buf);
    }
    
    if(strncmp(recv_buf, "#goaway ", 8) == 0){
        goaway_qun(conn_fd, i, recv_buf);
           }
    if(strncmp(recv_buf, "#delete ", 8) == 0){
        delete_qun(conn_fd, i, recv_buf);
    }
   	return 1;
}

//=====================================================私聊成功后的消息解析====================================================
int siliao_sucess(int i, int conn_fd, int j, int beiyong_fd, char *findname)
{
	char recv_buf[100];
	char send_buf[100];
	int fp;
	char a[100];
	time_t timep;
	
	//防止客户端异常退出
   	if(recv(users[i].fd, recv_buf, MAX_TO, 0) == 0){
   		//服务器显示退出信息
    	error_goaway(i, " is error go away", recv_buf);
   		    			
   		//将异常推出的信息保存在系统日志中
    	goaway_keep(recv_buf);
				
		//将异常推出的信息保存在错误日志中
		error_keep(recv_buf);
            	
        //向所有在线用户发送该用户退出的信息
        goaway_talk_all(conn_fd, i);
    	return -1;
    }
    	    		
    //传输的是文件内容是直接传送给课户端，并不进行特殊处理
    if(strncmp(recv_buf,"@###@",5) == 0){
    	send(users[j].fd, recv_buf, MAX_TO, 0);
    	return 0;
    }
    	    		
    //在私聊时切换私聊对象
    if(strncmp(recv_buf, "#to ",4) == 0){
	    for(j=4; recv_buf[j] != '\n'; j++){
	        findname[j-4] = recv_buf[j];
	    }
	    findname[j++] = '\0';
	    				
	for(j=0; users[j].username[0] != ' '; j++) {
      	if(strncmp(users[j].username, findname,strlen(users[j].username)-1) == 0){
            if(i == j){   
           	    send(users[i].fd, "服务器： you can,t talk with yourself\n", MAX_TO, 0);
                return 1;
            }
            if(users[j].fd == -1){    
    	        send(users[i].fd, "服务器： don't online\n", MAX_TO, 0);
                return 1;
            }
           	send(users[i].fd, "服务器： it's ok,you can talk with your friend\n", MAX_TO, 0);
            beiyong_fd = users[j].fd;
            break;
        }
    } 
	    /*for(j=0; users[j].username[0] != ' '; j++) {
      	    if((strncmp(users[j].username, findname,strlen(users[j].username)-1) == 0) && (users[j].fd != -1) && (i != j)){
            	send(users[i].fd, "服务器： it's ok,you can talk with your friend\n", MAX_TO, 0);
            	beiyong_fd = users[j].fd;
            	break;
        	}        	
    	}*/ 
    	    			
    	if(users[j].username[0] == ' '){
        	send(users[i].fd, "服务器： don't have the person!\n", MAX_TO, 0);
        	return 1;
        } else {
            return 0;
        }
 	}
    	    		
    //退出私聊
   	if(strcmp(recv_buf, "#exit\n") == 0) {
   		//退出私聊时，告诉该用户在县情况
   		all_online(recv_buf, i);
   		return 1;
   	} 
   		    		
   	//正常退出
   	else if(strcmp(recv_buf, "-exit\n") == 0){ 
		//接收客户端用户的聊天记录
   		recv_jilu(i, conn_fd);
        	
       	//服务器显示错误提示
        error_goaway(i, " is go away", recv_buf);
            
       	//将退出的信息保存在日志文件中
        goaway_keep(recv_buf);
            
        //使客户端两线程同步退出
        //send(conn_fd, "-exit\n", MAX_TO, 0);	    	
   	   	//提醒在线的所有人，该用户已退出
        goaway_talk_all(conn_fd, i);      			
   	 	//send(beiyong_fd, "#exit\n", MAX_TO, 0);       			
    	return -1;
    }else if(strcmp(recv_buf, "#show\n") == 0) {
   		//告诉用户在线情况
		all_online(recv_buf, i);		  
   		return 0;
   	}
   		    		
   	//无特殊情况时，进行普通处理 
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
   		send(users[i].fd, "#exit\n", MAX_TO, 0);
   		return 1;
   	}
   	return 0; 	
}

//============================================================私聊=================================================================
int my_siliao(char *recv_buf, int i, int conn_fd)
{
	int j, k;
	char findname[32];
    //用私聊时查找到的要接受数据的用户名
    int beiyong_fd;
    //记录私聊时要接受数据的套接字
    
	//查看用户是否存在
	for(j=4; recv_buf[j] != '\n'; j++){
	    findname[j-4] = recv_buf[j];
	}
	findname[j++] = '\0';
	for(j=0; users[j].username[0] != ' '; j++) {
      	if(strncmp(users[j].username, findname,strlen(users[j].username)-1) == 0){
            if(i == j){   
           	    send(users[i].fd, "服务器： you can,t talk with yourself\n", MAX_TO, 0);
                return 1;
            }
            if(users[j].fd == -1){    
    	        send(users[i].fd, "服务器： don't online\n", MAX_TO, 0);
                return 1;
            }
           	send(users[i].fd, "服务器： it's ok,you can talk with your friend\n", MAX_TO, 0);
            beiyong_fd = users[j].fd;
            break;
        }
    } 
    	    
    //不存在
    if(users[j].username[0] == ' '){
    	send(users[i].fd, "服务器： don't have the person!\n", MAX_TO, 0);
    } 
            
    //存在
    else{
   		while(1){
   		    if((k = siliao_sucess(i, conn_fd, j, beiyong_fd, findname)) == -1){
   		    	return -1;
   		    } else if(k == 1){
   		     	return 1;
   		    }		    	
   		}
   	}  
   	return 1; 	           
}

//=================================================================群聊 ======================================================= 
int my_qunliao(int conn_fd, int i)
{
	char recv_buf[100];
	char send_buf[100];
	int j;
	
	send(conn_fd, "成功进入群聊\n", MAX_TO, 0);
   	while(1){
   		//防止异常退出
   		if(recv(users[i].fd, recv_buf, MAX_TO, 0) == 0){
   		 	//服务器显示退出信息
    		error_goaway(i, " is error go away", recv_buf);
   		    			
   			//将异常推出的信息保存在系统日志中
    		goaway_keep(recv_buf);
				
			//将异常推出的信息保存在错误日志中
			error_keep(recv_buf);
            	
        	//向所有在线用户发送该用户退出的信息
        	goaway_talk_all(conn_fd, i);
    		return -1;
    	}
   		    	
   		//退出群聊
   		if(strcmp(recv_buf, "#exit\n") == 0) {
   		  	//提醒该用户在线的情况
   		    all_online(recv_buf, i);
   		    return 1;
   		} 
   		    	
   		//用户下线
   		else if(strcmp(recv_buf, "-exit\n") == 0){
   		    //接收客户端用户的聊天记录
   			recv_jilu(i, conn_fd);
        	
       		//服务器显示错误提示
        	error_goaway(i, " is go away", recv_buf);
            
       		//将退出的信息保存在日志文件中
        	goaway_keep(recv_buf);
            		
            //使客户端两线程同步退出
        	//send(conn_fd, "-exit\n", MAX_TO, 0);	    	
   	   		
   	   		//提醒在线的所有人，该用户已退出
       		goaway_talk_all(conn_fd, i);      		
    	    return -1;
       	} else if (strcmp(recv_buf, "#show\n") == 0) {
   		    //告诉用户在线情况
   		    all_online(recv_buf, i);	
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
	}   	           
}

//==========================================================离线消息==========================================================
int my_lixian(int i, int conn_fd, char *recv_buf)
{
	int j;	
	char 	filename_lixian[32];
	int m;
	char a[100];
	int k;
	int fp;
	
	for(j=10; recv_buf[j] != '\0'; j++){
    	filename_lixian[j-10] = recv_buf[j];
    }
    filename_lixian[j-10] = '\0';
    if(((m = find_delete(filename_lixian)) != -1) && (users[m].fd == -1)){    			
    	send(users[i].fd, "it,s ok!\n", MAX_TO, 0);
    	strcat(filename_lixian, "-1.txt");
    	while(1){
    		if(recv(users[i].fd, recv_buf, MAX_TO, 0) == 0){
   		    	//服务器显示退出信息
    			error_goaway(i, " is error go away", recv_buf);
   		    			
   				//将异常推出的信息保存在系统日志中
    			goaway_keep(recv_buf);
				
				//将异常推出的信息保存在错误日志中
				error_keep(recv_buf);
            	
        		//向所有在线用户发送该用户退出的信息

        		goaway_talk_all(conn_fd, i);
    	    	return -1;
    	   	}
    		if(strcmp(recv_buf, "#exit\n") == 0) {
    			return 1;
    		}
    		else if(strcmp(recv_buf, "-exit\n") == 0){
   		    	//接收客户端用户的聊天记录
   				recv_jilu(i, conn_fd);
        	
            	//服务器显示错误提示
        		error_goaway(i, " is go away", recv_buf);
            
       			//将退出的信息保存在日志文件中
        		goaway_keep(recv_buf);
            		
            	//使客户端两线程同步退出
        		//send(conn_fd, "-exit\n", MAX_TO, 0);	    	
   	   			
   	   			//提醒在线的所有人，该用户已退出
       			goaway_talk_all(conn_fd, i);   
    	    	return -1;
        	}
        	//普通离线消息
        	strcpy(a,users[i].username);
        	a[strlen(a)-1] = '\0';
        	strcat(a,"'s unline： ");
        	strcat(a,recv_buf);
        	k = strlen(a);
    		if(users[m].fd != -1){
    			send(users[i].fd, "对方以上线！\n",100,0);
    			break;
			}
        	if(k >= 100){
        		send(users[i].fd,"数据过长!\n",100,0);
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
    return 1;
}

//=============================================建群的群聊===========================================
int my_qun(int i, int conn_fd, char recv_buf[100])
{
    int j;
    int k;
    char send_buf[100];

    for(j=5; recv_buf[j] != '\n'; j++){
    	recv_buf[j-5] = recv_buf[j];
    }
    recv_buf[j-5] = '\0';

    for(j=0; j<30; j++){
        if(strcmp(recv_buf, qun[j].name) == 0){
            break;
        }
    }
    if(j == 30){
        send(conn_fd,"don't have the qun!", 100, 0);
        return 1;
    }
    for(k=0; k<5; k++){
        if(strcmp(qun[j].name, users[i].qunname[k]) == 0){
            break;
        }
    }
    if(k == 5){
        send(conn_fd, "you don't in tne qun!", 100, 0);
        return 1;
    }
    send(conn_fd, "you can talk in qun!", 100, 0);
    while(1){
    		if(recv(users[i].fd, recv_buf, MAX_TO, 0) == 0){
   		    	//服务器显示退出信息
    			error_goaway(i, " is error go away", recv_buf);
   		    			
   			//将异常推出的信息保存在系统日志中
    			goaway_keep(recv_buf);
				
			//将异常推出的信息保存在错误日志中
			error_keep(recv_buf);
            	
        		//向所有在线用户发送该用户退出的信息
        		goaway_talk_all(conn_fd, i);
    	    		return -1;
    	   	}
    		if(strcmp(recv_buf, "#exit\n") == 0) {
    			return 1;
    		}
    		else if(strcmp(recv_buf, "-exit\n") == 0){
   		    	//接收客户端用户的聊天记录
   				recv_jilu(i, conn_fd);
        	
            	//服务器显示错误提示
        		error_goaway(i, " is go away", recv_buf);
            
       			//将退出的信息保存在日志文件中
        		goaway_keep(recv_buf);
            		
            	//使客户端两线程同步退出
        		//send(conn_fd, "-exit\n", MAX_TO, 0);	    	
   	   			
   	   			//提醒在线的所有人，该用户已退出
       			goaway_talk_all(conn_fd, i);   
    	    	return -1; 
            }
        strcpy(send_buf, qun[j].name);
   		strcat(send_buf, " 群聊消息，来自 ");
   		strcat(send_buf, users[i].username);
   		send_buf[strlen(send_buf)-1] = '\0';
   		strcat(send_buf, ": ");
   		strcat(send_buf, recv_buf);

        for(k=0; k<30; k++){
            if((qun[j].user[k] != -1) && (qun[j].user[k] != i)){
                if(users[qun[j].user[k]].fd != -1){
                    send(users[qun[j].user[k]].fd, send_buf, 100, 0);
                }
            }
        }
    }
}

//=============================================服务器接收到的数据的解析与处理=========================
void do_it(int conn_fd)
{
    int i;
    char username[32];
    //接收到的用户（包括管理员的用户名）
    char userpassword[32];
    //接收到的密码（包括管理员的密码）
    char recv_buf[100];
    //接收到的数据或发送的数据
    time_t timep;

//界面的选择所发过来的数据的解析
	if((i = my_data(conn_fd)) == -1){
		return ;
	}
   
    while(1) {
    	//以普通用户进入后的解析
    	if(usual_mingling(conn_fd, i, recv_buf) == -1){
			return ;
		}
        
		//私聊	
        if(strncmp(recv_buf, "#to ",4) == 0){
        	 if(my_siliao(recv_buf, i, conn_fd) == -1){
        	 	return;
        	 }   
     	}
        
		//群聊       
        if(strcmp(recv_buf, "#all\n") == 0){
        	if(my_qunliao(conn_fd, i) == -1){
        		return ;
        	}
        }
        
		//修改密码
        if(strcmp(recv_buf, "#xiugai\n") == 0){
            send(users[i].fd, "#xiugai\n", MAX_TO, 0);
            recv(users[i].fd, recv_buf, MAX_TO, 0);
            strcat(recv_buf, "\n");
            strcpy(users[i].password, recv_buf);
            Write();
        }
		
		//离线消息
    	if(strncmp(recv_buf,"#lixianto ", 10) == 0){
    		if(my_lixian(i, conn_fd, recv_buf) == -1){
    			return ;
    		}
    	}

        //建群的群聊
        if(strncmp(recv_buf, "#qun ", 5) == 0){
            if(my_qun(i, conn_fd, recv_buf) == -1){
                return ;
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
