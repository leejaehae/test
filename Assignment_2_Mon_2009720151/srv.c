/////////////////////////////////////////////////////////////////////// 
// File Name : srv.c // 
// Date : 2014/04/22 // 
// Os : Ubuntu 12.04 LTS 64bits // 
// Author : Lee Jae Hae // 
// Student ID : 2009720151 // 
// ----------------------------------------------------------------- // 
// Title : System Programming Assignment #2 // 
// Description :  // 
///////////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#define BUF_MAX_SIZE 8192

typedef struct node
{
	int node_pid;
	int node_port;
	int node_time;
	struct node* next;
}node;

void node_input(int node_pid, int node_port, int node_time);
void node_output();
void node_search_del(int node_pid);
void node_delete();

void command(char buf[BUF_MAX_SIZE]);//command check
void NLST(char buf[BUF_MAX_SIZE], char buf2[BUF_MAX_SIZE]);//ls and dir command
void PWD(char buf2[BUF_MAX_SIZE]);//pwd command
void CWD(char buf[BUF_MAX_SIZE]);//cd command
void CDUP();//cd .. command
void MKD(char buf[BUF_MAX_SIZE]);//mkdir command
void DELE(char buf[BUF_MAX_SIZE]);//delete command
void RMD(char buf[BUF_MAX_SIZE]);//redir command
void RNFR_RNTO(char buf[BUF_MAX_SIZE], char *dest);//rename command
int make_argv(char buf[BUF_MAX_SIZE], char argv[1024][128]);//make arguments
void sort(int argc, char argv[1024][128]);//sort function
void uitoa(unsigned int in, char *output);//in->output

void sh_usr1(int); // signal handler for SIGUSR1
void sh_chld(int); // signal handler for SIGCHLD 
void sh_alrm(int); // signal handler for SIGALRM 
void sh_int(int); // signal handler for SIGINT

int client_fd; 
int client_cnt=0;

struct timeval timer, check;

pid_t PID, timer_id;

int client_info(struct sockaddr_in* client_addr);

node *client_list=NULL;

int main(int argc, char **argv) 
{ 
	char buff[BUF_MAX_SIZE]; 
	int n; 
	struct sockaddr_in server_addr, client_addr; 
	int server_fd;

	int len; 
	int port;

	struct timeval now;

	static struct sigaction act;

	signal(SIGUSR1, sh_usr1);
	signal(SIGALRM, sh_alrm);
	signal(SIGCHLD, sh_chld);
	signal(SIGINT, sh_int);

	if((PID = fork()) == 0)
	{
		timer_id = getpid();
		gettimeofday(&timer, NULL);
		
		while(1)
		{
			sleep(1);
			gettimeofday(&check, NULL);
			if(((int)timer.tv_sec - (int)check.tv_sec)%10 == 0)
				kill(getppid(), SIGALRM);
		}
		return 0;
	}
	else
	{
	server_fd = socket(PF_INET, SOCK_STREAM, 0);
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(atoi(argv[1]));
	bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	listen(server_fd, 15);

	while(1)
	{
		pid_t pid;
		len = sizeof(client_addr);
		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
		
		if((PID = fork()) == 0) //child process
		{
			while(1)
			{
				memset(buff, 0, sizeof(buff));
				n=read(client_fd, buff, BUF_MAX_SIZE);

				command(buff);

				if(strncmp(buff, "QUIT", 4) == 0)
					kill(getpid(), SIGINT);
				
				n=strlen(buff);
				write(client_fd, buff, BUF_MAX_SIZE);
			}
		}
		else //parent process
		{
			kill(timer_id, SIGUSR1);
			gettimeofday(&now, NULL);
			node_input(PID, ntohs(client_addr.sin_port), ((int)now.tv_sec));
			client_cnt++;
			client_info(&client_addr);
			node_output(client_list);
		}

		close(client_fd);
	}
	}
	return 0;
}

void node_input(int node_pid, int node_port, int node_time)
{
	node *item=client_list;
	node *new;
	new = (node*)malloc(sizeof(node));

	new->node_pid = node_pid;
	new->node_port = node_port;
	new->node_time = node_time;

	new->next=NULL;
	if(client_list == NULL)
		client_list=new;
	else
	{
		for(; item->next!=NULL; item=item->next) {}
		item->next=new;
	}

}
void node_output()
{
	node *item=client_list;

	struct timeval now;

	gettimeofday(&now, NULL);

	if(PID!=0 && client_list==NULL)
		printf("Current Nunber of Client : 0\n");
	else if(PID!=0)
	{
		printf("Current Nunber of Client : %d\n", client_cnt);
		printf("PID\tPORT\tTIME\n");
		for(; item!=NULL; item=item->next)
			printf("%d\t%d\t%d\n", item->node_pid, item->node_port, ((int)now.tv_sec) - ((int)item->node_time));
	}
}
void node_search_del(int node_pid)
{
	node *item=client_list;
	node *pre=item;
	node *del;

	if(client_list->node_pid == node_pid)
	{
		del=client_list;
		client_list=client_list->next;
		free(del);
	}
	else
	while(item)
	{
		if(item->node_pid == node_pid)
		{
			del=item;
			item=item->next;
			pre->next=item;
			free(del);
			pre=item;
		}
		else
		{
			pre=item;
			item=item->next;
		}
	}

}
void node_delete()
{
	node *del;

	while(client_list)
	{
		del=client_list;
		client_list=client_list->next;
		if(del->node_pid != timer_id)
			kill(del->node_pid, SIGINT);
		//sleep(1);
		free(del);
	}
	
}

void sh_usr1(int signum) {
	gettimeofday(&timer, NULL);
}

void sh_chld(int signum) {
	int temp;
	temp=wait(NULL);
	client_cnt--;
	node_search_del(temp);
	printf("\n");
	node_output(client_list);

}

void sh_alrm(int signum) {
	node_output();
}

void sh_int(int signum) {
	if(PID!=0)
		node_delete();
	else
	{
		if(timer_id != getpid()) {
		close(client_fd);
		printf("Client( %d)'s Release\n", getpid());
		}
	}
	exit(1);
}

int client_info(struct sockaddr_in* client_addr)
{
	printf("==========Client info===========\nclient IP: %s\nclient port: %d\n================================\nChild Process ID : %d\n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port), PID);
}

void command(char buf[BUF_MAX_SIZE])
{
	int ck=0;
	int n;
	char *token, *temp;
	char buf2[BUF_MAX_SIZE];

	memset(buf2, 0, sizeof(buf2));

	token=strtok(buf, "\n");
	while(token)
	{
		if(strncmp(token, "NLST", 4)==0)
		{
			printf("%s\n", token);
			NLST(buf, buf2);
		}
		else if(strncmp(token, "NIST", 4)==0)
		{
			printf("%s\n", token);
			NLST(buf, buf2);
		}
		else if(strncmp(token, "PWD", 3)==0) 
		{
			printf("%s\n", token);
			PWD(buf2);
		}
		else if(strncmp(token, "CWD", 3)==0) 
		{
			printf("%s\n", token);
			CWD(buf);
		}
		else if(strncmp(token, "CDUP", 4)==0) 
		{
			printf("%s\n", token);
			CDUP();
		}
		else if(strncmp(token, "MKD", 3)==0) 
		{
			printf("%s\n", token);
			MKD(buf);
		}
		else if(strncmp(token, "DELE", 4)==0) 
		{
			printf("%s\n", token);
			DELE(buf);
		}
		else if(strncmp(token, "RMD", 3)==0) 
		{
			printf("%s\n", token);
			RMD(buf);
		}
		else if(strncmp(token, "RNFR", 4)==0)
		{
			printf("%s\n", token);
			temp=strtok(NULL, "\n");
			RNFR_RNTO(token, temp);
		}
		else if(strncmp(token, "QUIT", 4)==0)
		{
			printf("%s\n", token);
			strcpy(buf2, "QUIT");
		}
		else
		{
			n=strlen(token);
			if(buf2==NULL)
			{
				strcpy(buf2, token);
				strcat(buf2, "\n");
			}
			else
			
			{
				strcat(buf2, token);
				strcat(buf2, "\n");
			}
		}

		token=strtok(NULL, "\n");
	}

	memset(buf, 0, sizeof(buf));
	strcpy(buf, buf2);
	memset(buf2, 0, sizeof(buf2));

}

void NLST(char buf[BUF_MAX_SIZE], char buf2[BUF_MAX_SIZE])
{
	char temp[BUF_MAX_SIZE];
	char *token;
	char getStr[128], getStr_o[128];
	char argv[1024][128];
	char data[1024][128];
	char dir_data[1024][128];
	char output[8192];
	int i, j, k, n, argc, max, a;
	int row, remain, cnt, dir_cnt;
	int option_check=0;

	unsigned int ui_temp;

	struct passwd *uid;
	struct group *gid;

	struct tm *atime;

	struct dirent *d;
	DIR *dp;

	struct stat buf_stat;

	if(strncmp(buf, "LIST", 4)==0)
		option_check=3;

	strcpy(temp, buf);

	token=strtok(buf, "\t");
	
	while(token=strtok(NULL, "\t"))
	{
		if(strcmp(token, "-a")==0)
		{
			if(option_check==0)
				option_check=1;
			else
				option_check=3;
		}
		else if(strcmp(token, "-l")==0)
		{
			if(option_check==0)
				option_check=2;
			else
				option_check=3;
		}
		else if(strcmp(token, "-al")==0)
		{
			if(option_check==0)
				option_check=3;
		}
		else if(strcmp(token, "-la")==0)
		{
			if(option_check==0)
				option_check=3;
		}
	}

	strcpy(buf, temp);

	argc=make_argv(buf, argv);
	for(i=1; i<argc; i++)
	{
		if(argv[i][0]=='-')
		{
			for(j=i; j<argc-1; j++)
			{
				strcpy(argv[j], argv[j+1]);
			}

			argc--;
			i--;		
		}
	}

	getcwd(getStr_o, 128);
	
	dir_cnt=0; k=0; a=0;
while(1)
{
	memset(data, 0, sizeof(data));
	memset(output, 0, sizeof(output));
	if(option_check==0)//ls
	{
		cnt=0; i=0;

		if(argc==1)
		{
			getcwd(getStr, 128);
			dp=opendir(getStr);
			while (d = readdir(dp)) 
			{
				if(strncmp(d->d_name, ".", 1)!=0)
				{
					strcpy(data[i], d->d_name);
					stat(data[i], &buf_stat);
					if(S_ISDIR(buf_stat.st_mode))
						strcat(data[i], "/");
					cnt++; i++;
				}
			}
			closedir(dp);k++;
		}
		else
		{
			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode)){}
				else
				{
					strcpy(data[j], argv[i]);
					cnt++;j++;
				}
			}

			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode))
				{
					strcpy(dir_data[j], argv[i]);
					strcat(dir_data[j], "/");
					j++; dir_cnt++;
				}
			}

			k=cnt; max=cnt+dir_cnt;
		}

		sort(cnt, data);

		row=cnt/5;
		remain=cnt%5;
		if(remain!=0)
			row++;

		for(i=0; i<row; i++)
		{
			if(remain==0)
			{
				for(j=0; j<5; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);
					strcat(output, "\t");
				}
			}
			else if(i==row-1)
			{
				for(j=0; j<remain; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);
						strcat(output, "\t\t");
				}
			}
			else
			{
				for(j=0; j<=remain; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);
					strcat(output, "\t\t");
				}
				for(; j<5; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*(row-1)+i+remain]);
					else
						strcat(output, data[j*(row-1)+i+remain]);
					strcat(output, "\t\t");
				}
					strcat(output, " ");
			}
			
			if(data==NULL)
				strcpy(output, "\n");
			else
				strcat(output, "\n");
		}

		if(buf2==NULL)
			strcpy(buf2, output);
		else	
			strcat(buf2, output);
	}
	else if(option_check==1)//ls -a
	{
		cnt=0; i=0;

		if(argc==1)
		{
			getcwd(getStr, 128);
			dp=opendir(getStr);
			while (d = readdir(dp)) 
			{
				strcpy(data[i], d->d_name);
				stat(data[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode))
					strcat(data[i], "/");
				cnt++; i++;
			}
			closedir(dp);k++;
		}
		else
		{
			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode)){}
				else
				{
					strcpy(data[j], argv[i]);
					cnt++;j++;
				}
			}

			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode))
				{
					strcpy(dir_data[j], argv[i]);
					strcat(dir_data[j], "/");
					j++; dir_cnt++;
				}
			}

			k=cnt; max=cnt+dir_cnt;
		}sort(cnt, data);
		row=cnt/5;

		remain=cnt%5;
		if(remain!=0)
			row++;

		for(i=0; i<row; i++)
		{
			if(remain==0)
			{
				for(j=0; j<5; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);
					strcat(output, "\t");
				}
			}
			else if(i==row-1)
			{
				for(j=0; j<remain; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);

						strcat(output, "\t\t");
				}
			}
			else
			{
				for(j=0; j<=remain; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*row+i]);
					else
						strcat(output, data[j*row+i]);
					strcat(output, "\t\t");
				}
				for(; j<5; j++)
				{
					if(output==NULL)
						strcpy(output, data[j*(row-1)+i+remain]);
					else
						strcat(output, data[j*(row-1)+i+remain]);
					strcat(output, "\t\t");
				}

				strcat(output, " ");
			}
			
			if(data==NULL)
				strcpy(output, "\n");
			else
				strcat(output, "\n");
		}

		if(buf2==NULL)
			strcpy(buf2, output);
		else	
			strcat(buf2, output);
	}
	else if(option_check==2)//ls -l
	{
		cnt=0; i=0;

		if(argc==1)
		{
			getcwd(getStr, 128);
			dp=opendir(getStr);
			while (d = readdir(dp)) 
			{
				if(strncmp(d->d_name, ".", 1)!=0)
				{
					strcpy(data[i], d->d_name);
					cnt++; i++;
				}
			}
			closedir(dp);k++;
		}
		else
		{
			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode)){}
				else
				{
					strcpy(data[j], argv[i]);
					cnt++;j++;
				}
			}

			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode))
				{
					strcpy(dir_data[j], argv[i]);
					strcat(dir_data[j], "/");
					j++; dir_cnt++;
				}
			}

			k=cnt; max=cnt+dir_cnt;
		}

		sort(cnt, data);

		for(i=0; i<cnt; i++)
		{
			stat(data[i], &buf_stat);

			if(S_ISDIR(buf_stat.st_mode))
				strcpy(output, "d");
			else
				strcpy(output, "-");

			if(S_IRUSR & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWUSR & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXUSR & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			if(S_IRGRP & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWGRP & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXGRP & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			if(S_IROTH & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWOTH & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXOTH & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			strcat(output, "\t");

			ui_temp=(unsigned int)buf_stat.st_nlink;
			uitoa(ui_temp, output);
			strcat(output, "\t");

			uid=getpwuid(buf_stat.st_uid);
			strcat(output, uid->pw_name);
			strcat(output, "\t");

			gid=getgrgid(buf_stat.st_uid);
			strcat(output, gid->gr_name);
			strcat(output, "\t");

			ui_temp=(unsigned int)buf_stat.st_size;
			uitoa(ui_temp, output);
			strcat(output, "\t");

			atime=gmtime(&buf_stat.st_atime);
			switch(atime->tm_mon)
			{
				case 0: strcat(output, "JAN "); break;
				case 1: strcat(output, "FEB "); break;
				case 2: strcat(output, "MAR "); break;
				case 3: strcat(output, "APR "); break;
				case 4: strcat(output, "MAY "); break;
				case 5: strcat(output, "JUN "); break;
				case 6: strcat(output, "JUL "); break;
				case 7: strcat(output, "AUG "); break;
				case 8: strcat(output, "SEP "); break;
				case 9: strcat(output, "OCT "); break;
				case 10: strcat(output, "NOV"); break;
				case 11: strcat(output, "DEC"); break;
			}
			uitoa(atime->tm_mday, output);
			strcat(output, " ");
			uitoa(atime->tm_hour, output);
			strcat(output, ":");
			uitoa(atime->tm_min, output);
			strcat(output, "\t");

			strcat(output, data[i]);
			if(S_ISDIR(buf_stat.st_mode))
				strcat(output, "/");
			strcat(output, "\n");

			if(buf2==NULL)
				strcpy(buf2, output);
			else	
				strcat(buf2, output);
		}
	}
	else//ls -al
	{
		cnt=0; i=0;

		if(argc==1)
		{
			getcwd(getStr, 128);
			dp=opendir(getStr);
			while (d = readdir(dp)) 
			{
				strcpy(data[i], d->d_name);
				cnt++; i++;
			}
			closedir(dp);k++;
		}
		else
		{
			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode)){}
				else
				{
					strcpy(data[j], argv[i]);
					cnt++;j++;
				}
			}

			j=0;
			for(i=1; i<argc; i++)
			{
				stat(argv[i], &buf_stat);
				if(S_ISDIR(buf_stat.st_mode))
				{
					strcpy(dir_data[j], argv[i]);
					strcat(dir_data[j], "/");
					j++; dir_cnt++;
				}
			}

			k=cnt; max=cnt+dir_cnt;
		}

		sort(cnt, data);

		for(i=0; i<cnt; i++)
		{
			stat(data[i], &buf_stat);

			if(S_ISDIR(buf_stat.st_mode))
				strcpy(output, "d");
			else
				strcpy(output, "-");

			if(S_IRUSR & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWUSR & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXUSR & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			if(S_IRGRP & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWGRP & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXGRP & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			if(S_IROTH & buf_stat.st_mode)
				strcat(output, "r");
			else
				strcat(output, "-");

			if(S_IWOTH & buf_stat.st_mode)
				strcat(output, "w");
			else
				strcat(output, "-");

			if(S_IXOTH & buf_stat.st_mode)
				strcat(output, "x");
			else
				strcat(output, "-");

			strcat(output, "\t");

			ui_temp=(unsigned int)buf_stat.st_nlink;
			uitoa(ui_temp, output);
			strcat(output, "\t");

			uid=getpwuid(buf_stat.st_uid);
			strcat(output, uid->pw_name);
			strcat(output, "\t");

			gid=getgrgid(buf_stat.st_uid);
			strcat(output, gid->gr_name);
			strcat(output, "\t");

			ui_temp=(unsigned int)buf_stat.st_size;
			uitoa(ui_temp, output);
			strcat(output, "\t");

			atime=gmtime(&buf_stat.st_atime);
			switch(atime->tm_mon)
			{
				case 0: strcat(output, "JAN "); break;
				case 1: strcat(output, "FEB "); break;
				case 2: strcat(output, "MAR "); break;
				case 3: strcat(output, "APR "); break;
				case 4: strcat(output, "MAY "); break;
				case 5: strcat(output, "JUN "); break;
				case 6: strcat(output, "JUL "); break;
				case 7: strcat(output, "AUG "); break;
				case 8: strcat(output, "SEP "); break;
				case 9: strcat(output, "OCT "); break;
				case 10: strcat(output, "NOV"); break;
				case 11: strcat(output, "DEC"); break;
			}
			uitoa(atime->tm_mday, output);
			strcat(output, " ");
			uitoa(atime->tm_hour, output);
			strcat(output, ":");
			uitoa(atime->tm_min, output);
			strcat(output, "\t");

			strcat(output, data[i]);
			if(S_ISDIR(buf_stat.st_mode))
				strcat(output, "/");
			strcat(output, "\n");

			if(buf2==NULL)
				strcpy(buf2, output);
			else	
				strcat(buf2, output);
		}
	}

		if(k==max || dir_cnt==0)
			break;
		else if(k==0 && cnt==0 && dir_cnt==1)
		{
			chdir(getStr_o);
			chdir(dir_data[1]);
		}
		else if(dir_cnt>0)
		{
			chdir(getStr_o);
			if(buf2==NULL)
				strcpy(buf2, "\n");
			else	
				strcat(buf2, "\n");
			memset(getStr, 0, sizeof(getStr));
			strncpy(getStr, dir_data[a], strlen(dir_data[a])-1);
			n=strlen(getStr);
			if(buf2==NULL)
				strcpy(buf2, getStr);
			else	
				strcat(buf2, getStr);
			strcat(buf2, ":\n");
			chdir(getStr); a++;
		} argc=1;
}
	
}

void PWD(char buf2[BUF_MAX_SIZE])
{
	char getStr[128];

	getcwd(getStr, 128);

	if(buf2==NULL)
		strcpy(buf2, getStr);
	else	
		strcat(buf2, getStr);
	strcat(buf2, "\n");
}

void CWD(char buf[BUF_MAX_SIZE])
{
	char *token;

	token=strtok(buf, "\t");
	token=strtok(NULL, "\n");

	chdir(token);
}

void CDUP()
{
	chdir("..");
}

void MKD(char buf[BUF_MAX_SIZE])
{
	char *token;

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		mkdir(token, 0775);
	}
}

void DELE(char buf[BUF_MAX_SIZE])
{
	char *token;
	struct stat exist_check;

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		stat(token, &exist_check);

		if(S_ISDIR(exist_check.st_mode))
			mkdir(token, 775);
		else
			unlink(token);
	}
}

void RMD(char buf[BUF_MAX_SIZE])
{
	char *token;

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		rmdir(token);
	}
}

void RNFR_RNTO(char buf[BUF_MAX_SIZE], char *dest)
{
	char *token, *token1;

	token1=strtok(dest, "\t");
	token1=strtok(NULL, "\t");

	token=strtok(buf, "\t");

	while(token=strtok(NULL, "\t\n"))
	{
		rename(token, token1);
	}
}

int make_argv(char buf[BUF_MAX_SIZE], char argv[1024][128])
{
	int argc=1;
	char *token;

	token=strtok(buf, "\t");
	strcpy(argv[0], token);

	while(token=strtok(NULL, "\t"))
	{
		strcpy(argv[argc], token);
		argc++;
	}

	return argc;
}

void sort(int argc, char argv[1024][128])
{
	int i, j;
	char temp[128];

	for(i=0; i<argc-1; i++)
	{
		for(j=0; j<argc-i-1; j++)
		{
			if(strcasecmp(argv[j], argv[j+1])>0)
			{
				strcpy(temp, argv[j]);
				strcpy(argv[j], argv[j+1]);
				strcpy(argv[j+1], temp);
			}
		}
	}
}

void uitoa(unsigned int in, char *output)
{
	char out[30]={0,};
	unsigned int temp;
	int i, check=1;

	temp=in;
	while(temp/=10)
	{
		check++;
	}

	for(i=check; i>0; i--)
	{
		out[i-1]=48+in%10;
		in/=10;
	}
	strcat(output, out);
}

