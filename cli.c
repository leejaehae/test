/////////////////////////////////////////////////////////////////////// 
// File Name : cli.c // 
// Date : 2014/05/22 // 
// Os : Ubuntu 12.04 LTS 64bits // 
// Author : Lee Jae Hae // 
// Student ID : 2009720151 // 
// ----------------------------------------------------------------- // 
// Title : System Programming Assignment #2 // 
// Description :  // 
///////////////////////////////////////////////////////////////////////
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#define BUF_MAX_SIZE 8192

void sort(int argc, char argv[1024][128]);//sort fucntion
int wildcard(int argc, char argv[1024][128]);//whilcard check function
void strrev(char *s);//string reverse function
char *path_up(char *argv);//input data exist check function
char *uitoa_(unsigned int in);//unsigned int -> char*
int make_argv(char *buf, char argv[1024][128]);
void convert(int argc, char argv[1024][128], char buf[BUF_MAX_SIZE]);
void sh_quit(int signum);

int sockfd;

int main(int argc, char **argv)
{
	char buff[BUF_MAX_SIZE];
	char *temp;
	int n, i;
	
	int m_argc;
	char m_argv[1024][128];
	struct sockaddr_in serv_addr;

	signal(SIGINT, sh_quit);

	sockfd = socket(AF_INET, SOCK_STREAM,0);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)); 

	while(1) 
	{ 
		memset(buff, 0, sizeof(buff));
		write(STDOUT_FILENO, "> ", 2); 
		read(STDIN_FILENO, buff, BUF_MAX_SIZE); 

		m_argc = make_argv(buff, m_argv);

		memset(buff, 0, sizeof(buff));
		convert(m_argc, m_argv, buff);

		if(write(sockfd, buff, BUF_MAX_SIZE) > 0)
		{ 
			if(read(sockfd, buff, BUF_MAX_SIZE) > 0) 
				if(strncmp(buff, "QUIT", 4)==0)
					sh_quit(0);
				else
					write(STDOUT_FILENO, buff, BUF_MAX_SIZE); 
			else
				break; 
		}
		else 
			break;
	} 
	close(sockfd); 

	return 0; 
}

void sh_quit(int signum) {
	write(sockfd, "QUIT", 4);
	close(sockfd);
	exit(1);
}

void convert(int argc, char argv[1024][128], char buf[BUF_MAX_SIZE])
{
	struct stat exist_check, exist_check1;
	
	int rename_ck=0;
	int n;
	int i, j, k;
	int ck=0, cnt=0;

	if(strcmp(argv[1], "ls") == 0)
	{
		for(i=2; i<argc; i++)
		{
			if(argv[i][0]!='*' && argv[i][strlen(argv[i])-1]!='*' && argv[i][0]!='-' && stat(argv[i], &exist_check) == -1)
			{
				if(ck==0)
				{
					strcpy(buf, "ls: cannot access ");
					ck=1;
				}
				else
					strcat(buf, "ls: cannot access ");

				strcat(buf, argv[i]);
				strcat(buf, ": No such file or directory\n");

				cnt++;
			}
		}

		if(argc==2 || (argc>2 && cnt+2!=argc))
		{
			if(ck==0)
				strcpy(buf, "NLST");
			else
				strcat(buf, "NLST");

			ck=0;
		}

		argc=wildcard(argc, argv);
		sort(argc, argv);

		for(i=2; i<argc; i++)
		{
			if(argv[i][0]=='-' && strcmp(argv[i], "-a")!=0 && strcmp(argv[i], "-l")!=0
			&& strcmp(argv[i], "-al")!=0 && strcmp(argv[i], "-la")!=0)
			{
				strcpy(buf, "ls: invalid option -- '");
				strncat(buf, argv[i]+1, 1);
				strcat(buf, "'\nls: Available options: a l\n");
				ck=1;
				break;
			}
			else if(stat(argv[i], &exist_check) == 0 || strcmp(argv[i], "-a")==0 || strcmp(argv[i], "-l")==0
			|| strcmp(argv[i], "-al")==0 || strcmp(argv[i], "-la")==0 || argv[i][0]=='*' || argv[i][strlen(argv[i])-1]=='*')
			{
				strcat(buf, "\t");
				strcat(buf, argv[i]);
			}
		}

		if(ck==0 && cnt+2!=argc)
			strcat(buf, "\n");
	}
	else if(strcmp(argv[1], "dir") == 0)
	{
		for(i=2; i<argc; i++)
		{
			if(argv[i][0]!='*' && argv[i][strlen(argv[i])-1]!='*' && argv[i][0]!='-' && stat(argv[i], &exist_check) == -1)
			{
				if(ck==0)
				{
					strcpy(buf, "dir: cannot access ");
					ck=1;
				}
				else
					strcat(buf, "dir: cannot access ");

				strcat(buf, argv[i]);
				strcat(buf, ": No such file or directory\n");

				cnt++;
			}
		}

		if(argc==2 || (argc>2 && cnt+2!=argc))
		{
			if(ck==0)
				strcpy(buf, "LIST");
			else
				strcat(buf, "LIST");

			ck=0;
		}

		sort(argc, argv);

		for(i=2; i<argc; i++)
		{
			if(argv[i][0]=='-')
			{
				strcpy(buf, "bash: dir: ");
				strncat(buf, argv[i]+1, 1);
				strcat(buf, ": invalid option\ndir: usage: dir [Arguments]\n");
				ck=1;
				break;
			}
			else if(stat(argv[i], &exist_check) == 0 || argv[i][0]=='*' || argv[i][strlen(argv[i])-1]=='*')
			{
				strcat(buf, "\t");
				strcat(buf, argv[i]);
			}
		}

		if(ck==0 && cnt+2!=argc)
			strcat(buf, "\n");
	}
	else if(strcmp(argv[1], "pwd") == 0)
	{
		if(argc>2 && argv[2][0]=='-')
		{
			strcpy(buf, "bash: pwd: ");
			strncat(buf, argv[i]+1, 1);
			strcat(buf, ": invalid option\npwd: usage: pwd\n");
		}
		else
		{
			strcpy(buf, "PWD");
			strcat(buf, "\n");
		}
	}
	else if(strcmp(argv[1], "cd") == 0)
	{
		if(argc<3)
			strcpy(buf, "Error: another argument is required\n");
		else if(strcmp(argv[2], "..")==0)
		{
			strcpy(buf, "CDUP\n");
			chdir("..");
		}
		else
		{
			if(argv[2][0]=='-')
			{
				strcpy(buf, "bash: cd: ");
				strncat(buf, argv[i]+1, 1);
				strcat(buf, ": invalid option\ncd: usage: cd [Arguments]\n");
			}
			else if(stat(argv[2], &exist_check) == 0)
			{
				if(S_ISDIR(exist_check.st_mode))
				{
					strcpy(buf, "CWD");
					strcat(buf, "\t");
					strcat(buf, argv[2]);
					strcat(buf, "\n");
					chdir(argv[2]);
				}
				else
				{
					strcpy(buf, "bash: cd: ");
					strcat(buf, argv[2]);
					strcat(buf, ": Not a directory\n");
				}
			}
			else
			{
				strcpy(buf, "bash: cd: ");
				strcat(buf, argv[2]);
				strcat(buf, ": No such file or directory\n");
			}
		}
	}
	else if(strcmp(argv[1], "mkdir") == 0)
	{
		if(argc < 3)
			strcpy(buf, "mkdir: missing operand\nmkdir: usage: mkdir [Arguments]\n");
		else
		{
			for(i=2; i<argc; i++)
			{
				if(argv[i][0]!='-' && stat(argv[i], &exist_check) == 0)
				{
					if(ck==0)
					{
						strcpy(buf, "mkdir: cannot create directory '");
						ck=1;
					}
					else
						strcat(buf, "mkdir: cannot create directory '");
	
					strcat(buf, argv[i]);
					strcat(buf, "': File exists\n");
	
					cnt++;
				}
				else
				{
					for(j=i-1; j>1; j--)
					{
						if(strcmp(argv[i], argv[j])==0)
						{
							if(ck==0)
							{
								strcpy(buf, "mkdir: cannot create directory '");
								ck=1;
							}
							else
								strcat(buf, "mkdir: cannot create directory '");
			
							strcat(buf, argv[i]);
							strcat(buf, "': File exists\n");
	
							for(k=i; k<argc-1; k++)
							{
								memset(argv[k], 0, sizeof(*argv[k]));
								strcpy(argv[k], argv[k+1]);
							}
	
							argc--;
							i--;
						}
					}
				}
			}
	
			if(argc==2 || (argc>2 && cnt+2!=argc))
			{
				if(ck==0)
					strcpy(buf, "MKD");
				else
					strcat(buf, "MKD");

				ck=0;
			}

			sort(argc, argv);

			for(i=2; i<argc; i++)
			{
				if(argv[i][0]=='-')
				{
					strcpy(buf, "bash: mkdir: ");
					strncat(buf, argv[i]+1, 1);
					strcat(buf, ": invalid option\nmkdir: usage: mkdir [Arguments]\n");
					ck=1;
					break;
				}
				else if(stat(argv[i], &exist_check) == -1)
				{
					strcat(buf, "\t");
					strcat(buf, argv[i]);
				}
			}

			if(ck==0 && cnt+2!=argc)
				strcat(buf, "\n");
		}
	}
	else if(strcmp(argv[1], "delete") == 0)
	{
		sort(argc, argv);

		if(argc < 3)
			strcpy(buf, "delete: missing operand\ndelete: usage: delete [Arguments]\n");
		else
		{
			for(i=2; i<argc; i++)
			{
				if(argv[i][0]!='-' && stat(argv[i], &exist_check) == -1)
				{
					if(ck==0)
					{
						strcpy(buf, "delete: failed to remove '");
						ck=1;
					}
					else
						strcat(buf, "delete: failed to remove '");
	
					strcat(buf, argv[i]);
					strcat(buf, "': No such file or directory\n");
	
					cnt++;
				}
				else if(S_ISDIR(exist_check.st_mode))
				{
					if(ck==0)
					{
						strcpy(buf, "delete: cannot remove '");
						ck=1;
					}
					else
						strcat(buf, "delete: cannot remove '");
	
					strcat(buf, argv[i]);
					strcat(buf, "': Is a directory\n");
	
					cnt++;
				}
				else
				{
					for(j=i-1; j>1; j--)
					{
						stat(argv[i], &exist_check);
						stat(argv[j], &exist_check1);

						if(exist_check.st_ino==exist_check1.st_ino)
						{
							if(ck==0)
							{
								strcpy(buf, "delete: failed to remove '");
								ck=1;
							}
							else
								strcat(buf, "delete: failed to remove '");
			
							strcat(buf, argv[i]);
							strcat(buf, "': No such file or directory\n");
	
							for(k=i; k<argc-1; k++)
							{
								memset(argv[k], 0, sizeof(*argv[k]));
								strcpy(argv[k], argv[k+1]);
							}
	
							argc--;
							i--;
						}
					}
				}
			}
	
			if(argc==2 || (argc>2 && cnt+2!=argc))
			{
				if(ck==0)
					strcpy(buf, "DELE");
				else
					strcat(buf, "DELE");

				ck=0;
			}

			sort(argc, argv);

			for(i=2; i<argc; i++)
			{
				if(argv[i][0]=='-')
				{
					strcpy(buf, "bash: delete: ");
					strncat(buf, argv[i]+1, 1);
					strcat(buf, ": invalid option\ndelete: usage: delete [Arguments]\n");
					ck=1;
					break;
				}
				else if(stat(argv[i], &exist_check) == 0 && !S_ISDIR(exist_check.st_mode))
				{
					strcat(buf, "\t");
					strcat(buf, argv[i]);
				}
			}

			if(ck==0 && cnt+2!=argc)
				strcat(buf, "\n");
		}
	}
	else if(strcmp(argv[1], "rmdir") == 0)
	{
		if(argc < 3)
			strcpy(buf, "rmdir: missing operand\nrmdir: Usage: rmdir [Arguments]\n");
		else
		{
			for(i=2; i<argc; i++)
			{
				if(argv[i][0]!='-' && stat(argv[i], &exist_check) == -1)
				{
					if(ck==0)
					{
						strcpy(buf, "rmdir: failed to remove '");
						ck=1;
					}
					else
						strcat(buf, "rmdir: failed to remove '");
	
					strcat(buf, argv[i]);
					strcat(buf, "': No such file or directory\n");
	
					cnt++;
				}
				else if(!S_ISDIR(exist_check.st_mode))
				{
					if(ck==0)
					{
						strcpy(buf, "rmdir: failed to remove '");
						ck=1;
					}
					else
						strcat(buf, "rmdir: failed to remove '");
	
					strcat(buf, argv[i]);
					strcat(buf, "': Not a directory\n");
	
					cnt++;
				}
				else
				{
					for(j=i-1; j>1; j--)
					{
						stat(argv[i], &exist_check);
						stat(argv[j], &exist_check1);

						if(exist_check.st_ino==exist_check1.st_ino)
						{
							if(ck==0)
							{
								strcpy(buf, "rmdir: failed to remove '");
								ck=1;
							}
							else
								strcat(buf, "rmdir: failed to remove '");
			
							strcat(buf, argv[i]);
							strcat(buf, "': No such file or directory\n");
	
							for(k=i; k<argc-1; k++)
							{
								memset(argv[k], 0, sizeof(*argv[k]));
								strcpy(argv[k], argv[k+1]);
							}
	
							argc--;
							i--;
						}
					}
				}
			}

			if(argc==2 || (argc>2 && cnt+2!=argc))
			{
				if(ck==0)
					strcpy(buf, "RMD");
				else
					strcat(buf, "RMD");

				ck=0;
			}

			sort(argc, argv);

			for(i=2; i<argc; i++)
			{
				if(argv[i][0]=='-')
				{
					strcpy(buf, "bash: rmdir: ");
					strncat(buf, argv[i]+1, 1);
					strcat(buf, ": invalid option\nrmdir: usage: rmdir [Arguments]\n");
					ck=1;
					break;
				}
				else if(stat(argv[i], &exist_check) == 0 && S_ISDIR(exist_check.st_mode)!=0)
				{
					strcat(buf, "\t");
					strcat(buf, argv[i]);
				}
			}

			if(ck==0 && cnt+2!=argc)
				strcat(buf, "\n");
		}
	}
	else if(strcmp(argv[1], "rename") == 0)
	{
		if(argc < 4)
		{
			strcpy(buf, "rename: missing destination file operand after '");
			strcat(buf, argv[2]);
			strcat(buf, "'\nUsage: rename [Arguments] [DEST]\n");
		}
		else if(argc ==4)
		{
			if(argv[2][0]=='-')
			{
				strcpy(buf, "bash: rename: ");
				strncat(buf, argv[2]+1, 1);
				strcat(buf, ": invalid option\nrename: usage: rename [Arguments] [DEST]\n");
			}
			else if(argv[3][0]=='-')
			{
				strcpy(buf, "bash: rename: ");
				strncat(buf, argv[3]+1, 1);
				strcat(buf, ": invalid option\nrename: usage: rename [Arguments] [DEST]\n");
			}
			else if(stat(argv[2], &exist_check) == 0)
			{
				stat(argv[2], &exist_check);
				stat(argv[3], &exist_check1);

				if(exist_check.st_ino==exist_check1.st_ino)
				{
					strcpy(buf, "rename: cannot move `");
					strcat(buf, argv[2]);
					strcat(buf, "' to a subdirectory of itself, `");
					strcat(buf, argv[2]);
					strcat(buf, "/");
					strcat(buf, argv[2]);
					strcat(buf, "'\n");
				}
				else //if(stat(argv[3], &exist_check)==0 || stat(path_up(argv[3]), &exist_check)==0)
				{
					strcpy(buf, "RNFR\t");
					strcat(buf, argv[2]);
					strcat(buf, "\nRNTO\t");
					strcat(buf, argv[3]);
					strcat(buf, "\n");
				}
				/*else
				{
					strcpy(buf, "rename: cannot move `");
					strcat(buf, argv[2]);
					strcat(buf, "' to '");
					strcat(buf, argv[3]);
					strcat(buf, ": No such file or directory\n");
				}*/
			}
			else
			{
				strcpy(buf, "rename: cannot stat '");
				strcat(buf, argv[2]);
				strcat(buf, "': No such file or directory\n");
			}
		}
		else
		{	
			stat(argv[argc-1], &exist_check);
			stat(path_up(argv[argc-1]), &exist_check1);

			if(stat(path_up(argv[argc-1]), &exist_check)==-1 && exist_check.st_ino==exist_check1.st_ino)
			{
				strcpy(buf, "rename: target '");
				strcat(buf, argv[argc-1]);
				strcat(buf, "' is not a directory\n");
			}
			else
			{
				for(i=2; i<argc-1; i++)
				{
					stat(argv[argc-1], &exist_check1);

					if(argv[i][0]!='-' && stat(argv[i], &exist_check) == -1)
					{
						if(rename_ck==0 && ck==0)
						{
							strcpy(buf, "rename: cannot stat '");
							ck=1;
						}
						else
							strcat(buf, "rename: cannot stat '");
		
						strncat(buf, argv[i]+1, 1);
						strcat(buf, "': No such file or directory\n");
		
						cnt++;
					}
					else if(exist_check.st_ino==exist_check1.st_ino)
					{
						if(rename_ck==0)
						{
							strcpy(buf, "rename: cannot move '");
							strcat(buf, argv[i]);
							strcat(buf, "' to a subdirectory of itself, '");
							strcat(buf, argv[i]);
							strcat(buf, "/");
							strcat(buf, argv[i]);
							strcat(buf, "'\n");
							rename_ck=1;
						}
						else
						{
							strcat(buf, "rename: will not create hard link '");
							strcat(buf, argv[i]);
							strcat(buf, "/");
							strcat(buf, argv[i]);
							strcat(buf, "' to directory '");
							strcat(buf, argv[i]);
							strcat(buf, "/");
							strcat(buf, argv[i]);
							strcat(buf, "'\n");
						}

						cnt++;
					}
					else
					{
						for(j=i-1; j>1; j--)
						{
							stat(argv[i], &exist_check);
							stat(argv[j], &exist_check1);

							if(exist_check.st_ino==exist_check1.st_ino)
							{
								if(ck==0)
								{
									strcpy(buf, "rename: cannot stat '");
									ck=1;
								}
								else
									strcat(buf, "rename: cannot stat '");
				
								strcat(buf, argv[i]);
								strcat(buf, "': No such file or directory\n");
		
								for(k=i; k<argc-1; k++)
								{
									memset(argv[k], 0, sizeof(*argv[k]));
									strcpy(argv[k], argv[k+1]);
								}
		
								argc--;
								i--;
							}
						}
					}
				}
	
				if(argc==2 || (argc>2 && cnt+3!=argc))
				{
					if(rename_ck==0 && ck==0)
						strcpy(buf, "RNFR");
					else
						strcat(buf, "RNFR");
	
					ck=0;
					sort(argc-1, argv);
					for(i=2; i<argc-1; i++)
					{
						if(argv[i][0]=='-')
						{
							strcpy(buf, "bash: rename: ");
							strncat(buf, argv[i]+1, 1);
							strcat(buf, ": invalid option\nrename: usage: rename [Arguments] [DEST]\n");
							ck=1;
							break;
						}
						else if(stat(argv[i], &exist_check) == 0 && strcmp(argv[i], argv[argc-1])!=0)
						{
							strcat(buf, "\t");
							strcat(buf, argv[i]);
						}
					}
		
					if(ck==0)
					{
						strcat(buf, "\nRNTO\t");
						strcat(buf, argv[i]);
						strcat(buf, "\n");
					}
				}
			}
		}
	}
	else if(strcmp(argv[1], "quit") == 0)
	{
		if(argc>2 && argv[2][0]=='-')
		{
			strcpy(buf, "bash: quit: ");
			strncat(buf, argv[i]+1, 1);
			strcat(buf, ": invalid option\nquit: usage: quit\n");
		}
		else
		{
			strcpy(buf, "QUIT");
			strcat(buf, "\n");
		}
		
	}
	else
	{
		strcpy(buf, argv[1]);
		strcat(buf, ": command not found\n");
	}

	/*n = strlen(buf);

	if(write(STDOUT_FILENO, buf, n) != n)
		printf("write error\n");

	exit(0);*/
}

void sort(int argc, char argv[1024][128])
{
	int i, j;
	char temp[128];

	for(i=2; i<argc-1; i++)
	{
		for(j=2; j<argc-i-1; j++)
		{
			if(strcasecmp(argv[j], argv[j+1])>0)
			{
				memset(temp, 0, sizeof(temp));
				strcpy(temp, argv[j]);
				memset(argv[j], 0, sizeof(argv[j]));
				strcpy(argv[j], argv[j+1]);
				memset(argv[j+1], 0, sizeof(argv[j+1]));
				strcpy(argv[j+1], temp);
			}
		}
	}
}

int wildcard(int argc, char argv[1024][128])
{
	int cnt=2;
	int i;
	char getStr[128];
	char temp[128];
	char buf[2048][128];

	struct dirent *d;
	DIR *dp;

	for(i=2; i<argc; i++)
	{
		if(argv[i][0]=='*' && strlen(argv[i])==1)
		{
			dp=opendir(getcwd(getStr, 128));

			while(d=readdir(dp))
			{
				if(strncmp(d->d_name, ".", 1)!=0)
				{
					strcpy(buf[cnt],d->d_name);
					cnt++;
				}
			}

			closedir(dp);
		}
		else if(argv[i][0]=='*')
		{
			strrev(argv[i]);
			argv[i][strlen(argv[i])-1]='\0';

			dp=opendir(getcwd(getStr, 128));

			while(d=readdir(dp))
			{
				strcpy(temp,d->d_name);
				strrev(temp);

				if(strncmp(argv[i], temp, strlen(argv[i]))==0)
				{
					strcpy(buf[cnt], d->d_name);
					cnt++;
				}
			}

			closedir(dp);
		}
		else if(argv[i][strlen(argv[i])-1]=='*')
		{
			dp=opendir(getcwd(getStr, 128));

			while(d=readdir(dp))
			{
				if(strncmp(argv[i], d->d_name, strlen(argv[i])-1)==0)
				{
					strcpy(buf[cnt], d->d_name);
					cnt++;
				}
			}

			closedir(dp);
		}
		else
		{
			strcpy(buf[cnt], argv[i]);
			cnt++;
		}
	}

	cnt++;

	for(i=2; i<cnt; i++)
	{
		memset(argv[i], 0, sizeof(argv[i]));
		strcpy(argv[i], buf[i]);
	}

	return cnt;
}

void strrev(char *s)
{
	int i;
	char temp;

	for(i=0; i<strlen(s)/2; i++)
	{
		temp=*(s+i);
		*(s+i)=*(s+strlen(s)-1-i);
		*(s+strlen(s)-1-i)=temp;
	}
}

char *path_up(char *argv)
{
	int n;
	char temp[128];
	char *token;

	if(argv[0]!='/')
		return argv;

	strcpy(temp, argv);
	strrev(temp);

	if(*temp=='/')
		strcpy(temp, temp+1);

	token=strtok(temp, "/");

	n=strlen(argv)-strlen(temp)-1;

	strncpy(temp, argv, n);

	token=temp;

	return token;
}

int make_argv(char *buf, char argv[1024][128])
{
	int argc=1;
	char *token;

	token=strtok(buf, " \n");
	if(token!=NULL)
	{
		strcpy(argv[1], token);
		argc++;
	}

	while(token=strtok(NULL, " \n"))
	{
		strcpy(argv[argc], token);
		argc++;
	}

	return argc;
}
