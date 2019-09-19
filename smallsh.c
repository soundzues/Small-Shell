//Name: Aditya Kothari
//Class: CS344
//Asignment: 3 - smallsh
//
//
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<signal.h>

#define SIGINT 2 //define Interrupt signal
#define SIGTSTP 20 //define stop sig

int STP_flag = 0; //stop flag, will be changed by
int stat = 0; //keeps track fo exit value

struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};//Initialize sig action structs

char argBuffer[2048]; //2048 char per line
char *arguments[512]; //512 arguments 

//stores process ID
int processID[3000]; //cuz I luv process 3000 (EMD GAME REF XD)
int process_count = 0;

//tracks flags 
struct flags
{
	//flag: 1 = set, 0 = not set
	int back;
	int input;
	int output;
	char *in;
	char *out;

};

//initialize flags
void flagInit(struct flags *cmd)
{
		cmd->back = 0;
		cmd->input = 0;
		cmd->output = 0;
		cmd->in = NULL;
		cmd->out = NULL;
}

//Kill all of it child process
//end execution 
int inBuiltExit()
{ 
	int i = -5;
	//USE KILLSIG TO KILL
	for(i = 0; i < process_count; i++)
	{
		kill(processID[process_count], SIGTERM);
	}
	
	//exit to end
	exit(0);
}

//retunrs status of the most recent executed process
int status(int *curStatus)
{
	return *curStatus;
}

//catch ctrl z
void catchSIGSTP(int signo)
{
	if(STP_flag == 0) //if STP flag not set
	{
		STP_flag = 1;
		char *message = "switching to FOREGROUND-ONLY mode\n";
		write(STDOUT_FILENO, message,35);//since write is non re-entrant
	}
	else//switch 
	{
		STP_flag = 0;
		char *message = "switching to NORMAL mode\n";
		write(STDOUT_FILENO, message,25);
	}
}


int main(int argc, char *argv[])
{
	int i = -5, j = -5;
	int count = 0; //keeps track of number of argguments
	struct flags cmd;	
	flagInit(&cmd); //set flags to 0
	
	//STOP signal (ctrl-z)
	SIGTSTP_action.sa_handler = catchSIGSTP;
	SIGTSTP_action.sa_flags = SA_RESTART;
	sigfillset(&SIGTSTP_action.sa_mask);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	//Ignore Intterupt signal in shell - CTRL C
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	sigaction(SIGINT, &SIGINT_action, NULL);

	//prompt loop
	while(1)
	{
		//retrive input 
		printf(": "); fflush(stdout);
		fgets(argBuffer, sizeof(argBuffer), stdin); //get input from 
		strtok(argBuffer, "\n"); //get rid of new  line
	
		//tokenize input
		char *token;
		
		token = strtok(argBuffer, " "); //tokenize by space
		
		//if NULL reached therfore we are at end of line
		int input_flag = 0;
		int output_flag = 0;

		count = 0;
		while(token != NULL)
		{
			//detecting if stdin redirect or not
			if(cmd.input == 1)
			{
				input_flag = 1;
			}
			
			//detectring if stdout redrirect or not
			if(cmd.output == 1)
			{
				output_flag = 1;
			}

			if(strstr(token, "&") != 0) //background
			{
//				printf("BACKGROUND \n" );
				cmd.back = 1;
							
				//if STP flag is set it will dissable baground
				//by re-setting stp back flag which was set on detection of &
				if(STP_flag == 1)
				{
					cmd.back = 0;
				}
			}
			else if(strstr(token, "<") != 0)  //stdin redirect 
			{

				cmd.input = 1;
			}
			else if(strstr(token, ">") != 0) //stdout redirect 
			{

				cmd.output = 1;
			
			}//built in commnads 
			else
			{
				if(cmd.input == 1 && input_flag == 1)
				{
					cmd.in = token;
	
					input_flag = 0;
					cmd.input = 0;
				}
			
				if(cmd.output == 1 && output_flag == 1)
				{
					cmd.out = token;
					output_flag = 0;
					cmd.output = 0;
				}
						
				if(cmd.in == NULL && cmd.out == NULL)
				{
					arguments[count] = token; //storing arguments 

					count++;
				}
			}
				token = strtok(NULL, " ");  

		}	

		//test stdin and stdout redirect files
/*
		printf("stdin file = %s \n", cmd.in);
		printf("stdout file = %s \n", cmd.out);
*/

//
//		printf("stdin flag = %d \n",cmd.input);
//		printf("stdout flag = %d \n",cmd.output);
	
//		printf("%d \n", count);

/*	
		//test print content
		for(i = 0; i < count; i++)
		{
			printf("%s \n", arguments[i]);	
		}
*/
	
		//$$ expansion
		int x = 0;
		int y = 0;
		char buff[2048];
		int pid_exp = getpid();	
		int length_string;		


	
		for ( x = 0; x < count; x++) //go through each string
		{
			length_string = strlen(arguments[x]);
  			for ( y = 0; y < length_string; y++) //look for $$
			{ 
				if ( arguments[x][y] == '$' && arguments[x][y+1] == '$') //if $$ found concequtively 
				{
					arguments[x][y] = '\0'; //replace them with null chars
					arguments[x][y+1] = '\0';
					snprintf(buff,sizeof(buff),"%s%d", arguments[x], pid_exp); //add pid
					arguments[x] = buff;
				}
			}
		}

		arguments[count+1] = NULL;
		//test arguments:
		for(i = 0; i < count; i++)
		{

			if(*arguments[0] == '\n') //if new line caught break
			{

				break;
			}
			if(strchr(arguments[i], 35) != 0) //catches comments
			{

				break;
			}
			else if(strcmp(arguments[i], "exit") == 0) //exit 
			{

				inBuiltExit();
			}
			else if(strcmp(arguments[i], "cd") == 0) //cd
			{
				char cwd[200];

				if(count == 1) //just cd by itself 
				{
				
					//Use home env var to jump to home directory
					chdir(getenv("HOME"));			
				
				}
				else
				{
					int check = chdir(arguments[1]);
					
					if(check == -1)
					{
		
						printf("Directory does not exist"); fflush(stdout);
					}	

					break;
				}
			}
			else if(strcmp(arguments[i], "status") == 0) //status
			{
				
				//Prints out the status of most recent exited child
				int myChildStats = status(&stat);
				printf("exit value: %d \n", myChildStats);
				break;
			}//other commands - not built in
			else
			{
				
				//exec stuff:
				int childExitMethod = -5;
			
				pid_t spawnPid = -5;
				spawnPid = fork(); //spawn new child
			
				//if failed to spawn a chidl throw error
				if(spawnPid == -1)
				{
					printf("did not fork"); fflush(stdout);
					exit(1);
					break;
				}
				//inside child:
				else if(spawnPid == 0)
				{
					
					//redirect input & output
					if(cmd.in != NULL)
					{
						int sourceFD = open(cmd.in, O_RDONLY); //open file for read only
						if(sourceFD == -1)
						{
							perror("source open()");
							exit(1);
						}
						dup2(sourceFD, 0);//redirect stdin to point to file pointer
					}
					if(cmd.out != NULL)
					{
						//printf("do stdout redirect \n");
						int targetFD = open(cmd.out, O_WRONLY | O_CREAT | O_TRUNC, 0644);//open file to write
						if(targetFD == -1)
						{
							perror("target open()");
							exit(1);
						}
						dup2(targetFD, 1);//redirect stdout to point to file pointer
					}
					//deal with background process
					if(cmd.back == 1 && cmd.in == NULL)
					{
						int devNull = open("/dev/null", O_WRONLY);//open dev null to wirte only
						dup2(devNull, 0); //redirect stdin to point to file pointer
					}			
				
					//if forground process -  dont ignore ctrl c
					if(cmd.back == 0)
					{
						//forground process dont ignore CTRLZ
						SIGINT_action.sa_handler = SIG_DFL;
						sigaction(SIGINT, &SIGINT_action, NULL);
					}

					//exec
					execvp(arguments[0], arguments);
					printf("exec failed \n"); fflush(stdout);
					exit(2);
				}
				else //parrent wait on child
				{

					if(spawnPid != -1 && cmd.back == 0) //if exec don't fail wait for child
					{
						int childStatus = -0;
						int childSig = 0;
		    				 stat = 0; //reset stat
						//Wait on child
						pid_t actualPid = waitpid(spawnPid, &childExitMethod, 0);
					
						//catch status:

						//check exit status 
						if(WIFEXITED(childExitMethod))
						{
							childStatus = WEXITSTATUS(childExitMethod); //non 0 value if error 
						}
					
						//check if signal caused child to exit
						if(WIFSIGNALED(childExitMethod))
						{
							childSig = WTERMSIG(childExitMethod); //non 0 value if error
				
						}

						//if child terminated or failed set stat to 1 - i.e exit value 1
						if(childSig != 0 || childStatus != 0)
						{
							stat = 1;
						}						
						
						status(&stat);
				

					}
					//Keep track of PID;
					if(spawnPid != -1)
					{
						processID[process_count] = spawnPid;

						process_count++;
					}

					//if backgrounded process print PID
					if(spawnPid != -1 && cmd.back == 1)
					{
						printf("PID of baground process is: %d \n", spawnPid); fflush(stdout);
					}

						break;
		
				}							
			}	
		}

		//Backgroudn process signal handeling and wait:

		int childExitMethodBack = -5;
		int test = 1;		
		int childStatus2 = 0;
		int childSig2 = 0;
		int stat2 = 0;
	
		do
		{
			test =	waitpid(-1, &childExitMethodBack, WNOHANG);//wait on any child no hang
						
		if(test > 0){
	
			if(WIFEXITED(childExitMethodBack) && (test > 0))
			{	
				printf("exit value background process = %d \n", stat2); fflush(stdout);		
			}
	
			if(WIFSIGNALED(childExitMethodBack) && (test > 0))
			{
				childSig2 = WTERMSIG(childExitMethodBack); //non 0 value if error
				printf("Process ID of dead child: %d & Killser signal is: %d", test, childSig2); fflush(stdout);
			}	
		}		
		}while((test != 0) && (test != -1)); //wait on child process if there are no libin amd dead childrem


	

		flagInit(&cmd); //set flags to 0

		//Null all the arguments
		for(i = 0; i < 512; i++)
		{
			arguments[i] = NULL;
		}
	
	}

	//printf("test \n");
}
