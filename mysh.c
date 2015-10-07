#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define MAX_LINE		512 /* 512 chars per line, per command, should be enough. */
#define MAX_COMMANDS	5 /* size of history */
// Prototypes
int Setup(char inputBuffer[], char *args[],int *background);

int command_count = 0;

int main(void)
{
	char inputBuffer[MAX_LINE]; 	/* buffer to hold the command entered */
	int background;             	/* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2 + 1];		/* command line (of 512) has max of 40 arguments */
	int status;           			/* result from execvp system call*/
	int shouldrun = 1;
	pid_t child;            		/* process id of the child process */
	char error_message[30] = "An error has occurred\n";
	int i, upper;
	/* Program terminates normally inside Setup */
    while (shouldrun)
	{            		
		background = 0;
		
		shouldrun = Setup(inputBuffer,args,&background);       /* get next command */
		if (strncmp(inputBuffer, "exit", 4) == 0)
			return 0;
				
		// 1) Fork child process using fork()
		if (shouldrun) 
		{
			child = fork();          /* creates a duplicate process! */
				// 2) the child process invoke execvp()
			if(child<0)			
				write(STDERR_FILENO, error_message, strlen(error_message));

			else if(child==0)
			{
				if (strncmp(inputBuffer, "ls",2) == 0)
				{
					execvp("ls",args);
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(127);
				}
	                        else if (strncmp(inputBuffer, "pwd", 3) == 0)

        	                {
                	                char * dir=malloc(40);
                        	        getcwd(dir,get_current_dir_name());
                                	printf("%s\n",dir);
					free(dir);
                              		exit(0); 
                        	}

	                       	else if (strncmp(inputBuffer,"cd",2)==0 && args[1] ==NULL)
				{
                           		printf("%s\n", getenv("HOME"));
					exit(0);
				}
				else if (strncmp(inputBuffer,"cd",2)==0 && args[1] !=NULL)
				{
					int ret;
					ret=chdir(args[1]);
					if(ret==-1)
					{
						write(STDERR_FILENO, error_message, strlen(error_message));
						exit(127);
					}
					else
					{
			        	        printf("%s\n", getenv("PWD"));
						exit(0);
					}
				}
				else
				{
					write(STDERR_FILENO, error_message, strlen(error_message));
					exit(127);
				}
			}
			else 
			{
				wait(NULL);
			

			}
		}
    }// end of while 
	
	return 0;
}

/** 
 * The Setup function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings. 
 */
int Setup(char inputBuffer[], char *args[],int *background)
{
	int length,			/* # of characters in the command line */
		i,j,				/* loop index for accessing inputBuffer array */
		start,			/* index where beginning of next command parameter is */
		ct,				/* index of where to place the next parameter into args[] */
		command_number;	/* index of requested command number */
	char temp[MAX_LINE];
	ct = 0;
	
	/* read what the user enters on the command line */
	do 
	{
		printf("mysh>");
		fflush(stdout);
		length = read(STDIN_FILENO,inputBuffer,MAX_LINE); 
	}
	while (inputBuffer[0] == '\n'); /* swallow newline characters */

	/**
	 *  0 is the system predefined file descriptor for stdin (standard input),
	 *  which is the user's screen in this case. inputBuffer by itself is the
	 *  same as &inputBuffer[0], i.e. the starting address of where to store
	 *  the command that is read, and length holds the number of characters
	 *  read in. inputBuffer is not a null terminated C-string. 
	 */


	/** 
	 * the <control><d> signal interrupted the read system call 
	 * if the process is in the read() system call, read returns -1
	 * However, if this occurs, errno is set to EINTR. We can check this  value
	 * and disregard the -1 value 
	 */
	if ( (length < 0) && (errno != EINTR) ) 
	{
		perror("error reading the command");
		exit(-1);           /* terminate with error code of -1 */
	}

	/**
	 * Parse the contents of inputBuffer
	 */
	start =0;
	int end =0;
	if (inputBuffer[length-1]='\n')
		inputBuffer[length-1]=0;
	for (i=0;i<length;i++) 
	{ 
		if((int)inputBuffer[i]=='&')
		{
			args[ct++]=strdup("&");	 		
			background =0;
		}
		/* examine every character in the inputBuffer */
		if((int)inputBuffer[i]==' '|| (int)inputBuffer[i]=='>')
		{
			
			if((int)inputBuffer[i]=='>')
				args[ct++]=strdup(">");
			else
			{
				
				for(j=0;j<end-start;j++)
					temp[j]=inputBuffer[start+j];
				temp[end-start]=0;	
				args[ct++]=strndup(temp,(end-start));
				
				start=end;
			}
		}


		end++;
	}    /* end of for */

	for(j=0;j<end-start;j++)  //for the last element
       		temp[j]=inputBuffer[start+j];	
	temp[end-start]=0; // Null terminate
	
	args[ct++]=strndup(temp,(end-start));
	
	args[ct]=NULL;
	
	/**
	 * If we get &, don't enter it in the args array
	 */

	if (*background)
		args[--ct] = NULL;

//	args[ct] = NULL; /* just in case the input line was > 512 */

	return 1;

} /* end of Setup routine */
