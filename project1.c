#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

char** addToken(char** instr, char* tok, int numTokens);
void printTokens(char** instr, int numTokens);

char *myUSER;
char *myPWD;
char *myMACHINE;
char *myHOME;
char *myROOT;
time_t sessionTime;

typedef struct
	{
	int queueNum;
	pid_t PID;
	char cmd[255];	
	}Back;

char **res_loop(char **args,int numtoks);//loop that picks what to resolve and how
char *PATHres(char *oldPath);//resolves variables that start with $PATH
char *absoluteRes(char *oldPath);//returns absolute path of input
char **strArr(char *path, char **strarr, char del);//allocates a 2d array that is null terminated
void freestrArr(char **strarr);//frees 2d array
int strarrlength(char** strarr);//returns number of strings in a 2d array

int exists(char* path);//returns 1 if file exists 0 if it doesnt
char* addHOME(char* oldPath);//adds home to the front of the path
void remFirst(char* old);//removes first character
char* catPaths(char* first, char* second);//concatinates two dynamic allocated paths and stores it in the first

char* getdir(char* dir);//passes in . and returns directory
char* getparent(char* dir);//passes in .. and returns directory

void my_execute(char **cmd); //executes 
void my_backExecute(char **cmd, Back queue);

int contchar(char** instr, int numTokens, char cont);
char *newinstr(char **buck, int numTokens);

void echo(char *args);


int main( int argc, char *argv[] )
        {
	//Setting home directory
	myHOME = getenv("HOME");
	//myROOT = getenv("PWD");
	Back Ground;	
	
	//The following code is the parser help code from canvas
	char token[256];	//holds instruction token
	char ** bucket;		//array that holds all instruction tokens
	char temp[256];		//used to split tokens containing special characters

	while(7){
		myUSER = getenv("USER");  //ensure everthing is right before running
        	myPWD = getenv("PWD");
        	myMACHINE = getenv("MACHINE");

		sessionTime = time(NULL);

        	//check to make sure they all got valid data
        	if (myUSER != NULL && myPWD != NULL && myMACHINE != NULL)
                	printf("%s@%s :: %s -> ", myUSER, myMACHINE, myPWD);
        	else
                	printf("get env returned null");

		int numI = 0;	//number of tokens in instruction
		do{			//reads character sequences separated by whitespace
			scanf("%s", token);
			int i;
			int start;
			start = 0;

			for(i = 0; i < strlen(token); i++){
				if(token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&'){
					if(i-start > 0){
						memcpy(temp, token + start, i - start);
						temp[i-start] = '\0';
						bucket = addToken(bucket, temp, numI);
						numI++;
					}
	
					char specialChar[2];
					specialChar[0] = token[i];
					specialChar[1] = '\0';

					bucket = addToken(bucket, specialChar, numI);
					numI++;

					start = i + 1;
				}
			}
			if(start < strlen(token)){
				memcpy(temp, token + start, strlen(token) - start);
				temp[i-start] = '\0';
				bucket = addToken(bucket, temp, numI);
				numI++;
			}
		}while('\n' != getchar());	//until end of line is reached
//Resolve paths 
		res_loop(bucket, numI);
//execution
		if(strcmp(bucket[0],"exit")==0)
			{
			if(waitpid(Ground.PID, NULL, WNOHANG) == 0)
				printf("Error cant exit because background process running\n");
			else
				{
				sessionTime = time(NULL) - sessionTime;
				double time = (double) sessionTime;
				printf("Session Time: %fs\n", time);
				printf("Exiting...\n");
				free(bucket);
				return 0;
				}
			}

		else if(strcmp(bucket[0],"cd")==0)
			{
			if(exists(bucket[1])==1 && strcmp(bucket[1],"/") != 0)//makes sure bucket is valid and new location isnt the root
				{
				printf(" in cd \n");
				chdir(bucket[1]);
				setenv("PWD",bucket[1],1);
				}
			else
				printf("invalid location\n");
			}

		else if(strcmp(bucket[0], "echo") == 0){
			int i;
			for(i = 1; i < numI; i++){
				echo(bucket[i]);
				printf(" ");
			}
			printf("\n");
		}

//input redirection
		else if(contchar(bucket,numI,'<') > 0)
			{
				
			if(strcmp(bucket[0], "<")==0){
				printf("Invalid input\n");
				}
			else if((strcmp(bucket[1], "<") == 0) && numI == 2){					
				printf("Inavlid input\n");
				}
			else
				{
				char *path;
				char **arr;
				int fd,status;
				char inputStream[150];
				strcpy(inputStream, bucket[2]);
				
				pid_t pid = fork();
				if((fd = open(inputStream, O_RDONLY, 0)) < 0)
					{
					printf("ERROR: Unable to open file%s\n", inputStream);
					}
				if(pid == 0)
					{
					close(STDIN_FILENO);
					//dup2(fd, STDIN_FILENO);
					dup(fd);
					close(fd);
						
					path = newinstr(bucket,numI);

					int i = 0;
					while(path[i] != '<')
						{
						i++;
						}
					path[i-1] = '\0';
					arr = strArr(path, arr, ' ');
					my_execute(arr);

					freestrArr(arr);
					free(path);
					exit(0);
					}
				else
					{
					waitpid(pid,&status,0);
					close(fd);
					}
				
					
				}
			}
//output redirection
		else if(contchar(bucket,numI,'>') > 0){
			if(strcmp(bucket[0], ">")==0)
				{
				printf("Invalid input\n");
				}
			else if(strcmp(bucket[1], ">") == 0 && numI == 2)
				{
				printf("Invalid input\n");
				}
			else
				{
				char *path;
				int status, fd;
				char **arr;
				char outputStream[150];
				strcpy(outputStream, bucket[2]);

				pid_t pid = fork();
				if((fd = creat(outputStream, 0644)) < 0)
					{
					printf("ERROR: Unable to open file%s\n", outputStream);
					}
				if(pid == 0)
					{
					close(STDOUT_FILENO);
					//dup2(fd, STDOUT_FILENO);
					dup(fd);
					close(fd);
					path = newinstr(bucket,numI);
					int i = 0;
					while(path[i] != '>')
						{
						i++;
						}

					path[i-1] = '\0';
					arr = strArr(path, arr, ' ');
                                        my_execute(arr);

                                        freestrArr(arr);
                                        free(path);
                                        exit(0);

					}
				else
					{
					waitpid(pid, &status, 0);
					close(fd);
					}
				}
			}			
				
//background execution
		else if(contchar(bucket,numI,'&') > 0 && contchar(bucket,numI,'&') == numI-1)//basically if & is the last character run the process in the background
			{
			//run in background
			//next need to remove & because it will mess up exec call
			char *fullPath;
			char **execArr;
			fullPath = newinstr(bucket,numI);
			int i = 0;			//gets rid of &
			while(fullPath[i] != '&')
				{
				i++;
				}
			fullPath[i-1] = '\0';//minus 1 because needs to get rid of space
			execArr = strArr(fullPath, execArr, ' ');		
			my_backExecute(execArr,Ground);
//execute in background here

			freestrArr(execArr);
			free(fullPath);						
			}

		else
			my_execute(bucket); 


	}	//until "exit" is read in

	return 777;	//Jackpot bb
}

//reallocates instr array to holds another token. Returns new pointer to instr array
//Taken from parser help on canvas
char** addToken(char** instr, char* tok, int numTokens){
	int i;

	char** new_arr;
	new_arr = (char**)malloc((numTokens+1) * sizeof(char*));

	//copy values into new array
	for(i = 0; i < numTokens; i++){
		new_arr[i] = (char*)malloc((strlen(tok)+1) * sizeof(char));
		strcpy(new_arr[i], instr[i]);
	}

	//add new token
	new_arr[numTokens] = (char*)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(new_arr[numTokens], tok);

	if(numTokens > 0)
		free(instr);

	return new_arr;
}
//takes an existing bucket and makes a string out of it
char *newinstr(char **buck, int numTokens)
	{
	char *temp;
	int i;
	int total = 0;
	//get length of bucket
	for(i = 0; i < numTokens; i++)	//gets size of new path
		{
		total = strlen(buck[i])+total;
		}
	//allocate space for spaces too
	temp = (char*)calloc(total+numTokens+1,sizeof(char));//allocates space for new path
	//copy over bucket
	for(i = 0; i < numTokens; i++)
		{
		temp = catPaths(temp,buck[i]);
		temp = catPaths(temp," ");
		}
	temp[total+numTokens] = '\0';
	return temp;
	}

void printTokens(char** instr, int numTokens){
	int i;
	printf("Tokens:\n");
	for(i = 0; i < numTokens; i++)
		printf("#%s#\n", instr[i]);
}

//determines what resolve and what to not
char** res_loop(char **args, int numtokes)
	{
	if(strcmp(args[0],"exit") == 0 || strcmp(args[0],"echo") == 0 || strcmp(args[0],"io") == 0)
		{}//no resolution
	else if(strcmp(args[0],"cd") == 0)
		{
		printf("its a cd\n");
		args[1]=absoluteRes(args[1]);
		}
	else 
		{
		//its another command try $PATH resolution
		args[0] = PATHres(args[0]);
		}
	}

char* PATHres(char* oldPath)
	{
	//takes adds each element of PATH to oldPath and checks if it is a valid location	
	char** PATHStr;
	char* temp;
	PATHStr = strArr(getenv("PATH"),PATHStr,':');
	int i=0;
	while(PATHStr[i] != NULL)
		{
		PATHStr[i]= absoluteRes(PATHStr[i]);
		PATHStr[i]= catPaths(PATHStr[i],"/");		
		PATHStr[i]= catPaths(PATHStr[i],oldPath);		
		if(exists(PATHStr[i]) ==1)
			{
			temp = PATHStr[i];
			PATHStr[i]=NULL;	
			freestrArr(PATHStr);
			return temp;
			}
		i++;
		}
	return oldPath;
	}


char *absoluteRes(char *oldPath)
	{
	//puts path into a array goes to each string  
	char *temp;
	int homeflag =0;
	if(oldPath[0] == '~')	//if starts with home resolve
		{
		oldPath= addHOME(oldPath);
		homeflag =1;
		}
	if(homeflag ==0)
		{
		char** pathArr;		//strarr to hold path 
		pathArr= strArr(oldPath,pathArr,'/');	//put strarr in pathArr
		int i =0;
		while(pathArr[i] != NULL)		//goes to each string fixes if . or ..
			{	
			if(strcmp(pathArr[i],".") ==0)
				{
				pathArr[i] = getdir(pathArr[i]); 
				}
			if(strcmp(pathArr[i],"..")== 0)
				pathArr[i] = getparent(pathArr[i]); 
			i++;
			}
		i= 0;
		int total = 0;
		while(pathArr[i] != NULL)
			{
			total = total + strlen(pathArr[i]);
			i++;
			}
		temp = (char*)calloc(total +1,sizeof(char));
		i = 0;
		strcpy(temp,pathArr[i]);
		i++;
		while(pathArr[i] != NULL)
			{
			strcat(temp,"/");
			strcat(temp,pathArr[i]);
		
			i++;
			}
		temp[total+1] = '\0';
		freestrArr(pathArr);
		free(oldPath);
	
		return temp;
		}
	else
		return oldPath;
	}
char **strArr(char *path, char** strarr, char del)
	{
	//takes in a string and a string array and returns a string array filled with the paths of value
	int i=0;		//for loop
	int count = 0;		//amount of array pointers to allocate
	while(path[i] != '\0')
		{
		if(path[i] ==del)
			count++;
		i++;
		}
	count++;//last one for null character
	strarr = (char**)calloc(count+1,sizeof(char*));//count plus 1 to allocate for null pointer in last string pointer

	for(i =0;i < count;i++)	//allocating space for each string
		strarr[i] = (char*)calloc(strlen(path),sizeof(char));//max size will be path
	strarr[i]= NULL;	//last string points to null
	int j = 0; //which string to put info in
	int k = 0; //how far in string you are
	for(i = 0;i<strlen(path);i++)
		{
		if(path[i] != del)
			{
			strarr[j][k]= path[i];
			k++;//progress through each string
			}
		else 
			{
			strarr[j][k] = '\0';//end that string
			j++;			//go to next
			k = 0;			//start at bottome of next
			}
		}
	strarr[j][k] = '\0';//final null character
	//array is populated

	return strarr;		
	}

//frees array created in strArr
void freestrArr(char **strarr)
	{
	int i = 0;
	while(strarr[i] != NULL)
		{
		free(strarr[i]);
		i++;
		}
	free(strarr[i]);	//free last
	free(strarr);
	}

//sees if path exists
int exists(char* path)
	{	
	if( access( path, F_OK ) != -1 ) 
		{
		return 1;
		} 	
	else 
		{
		return 0;
		}
	}

//adds $HOME to whatever path it is given
char* addHOME(char* oldPath)
	{
	char* temp;
	char* HOME = getenv("HOME");
	temp = (char*)calloc(strlen(HOME)+strlen(oldPath)+1,sizeof(char));
	strcpy(temp,HOME);
	remFirst(oldPath);
	strcat(temp,oldPath);
	free(oldPath);
	return temp;

	}

//removes first character of a c string
void remFirst(char* old)
	{
	int i=0;
	while(old[i] != '\0')
		{	
		old[i]=old[i+1];
		i++;
		}
	}

//concatenates two strings
char* catPaths(char* first, char* second)
	{
	char *temp;
	temp = (char*)calloc(strlen(first)+strlen(second)+1,sizeof(char));
	strcpy(temp,first);
	strcat(temp, second);
	free(first);
	return temp;
	}

//returns value inPWD 
char* getdir(char* dir)//passes in . and returns directory
	{
	char* PWD;
	PWD = (char*)calloc(strlen(getenv("PWD"))+1,sizeof(char));	//gets Path to working directory
	strcpy(PWD,getenv("PWD"));
	free(dir);
	return PWD;


	}

//gets PWD then removes last directory off of it
char* getparent(char* dir)//passes in . and returns directory
	{
	char* temp;
	char* PWD ;	//gets Path to working directory
	PWD = (char*)calloc(strlen(getenv("PWD"))+1,sizeof(char));	//gets Path to working directory
	strcpy(PWD,getenv("PWD"));	
	int i = strlen(PWD)-1;
	while(PWD[i] != '/')
		{
		i--;
		}
	PWD[i] = '\0';
	free(dir);
	return PWD;	
	}

//returns number of tokens in a strArr
int strarrlength(char** strarr)
	{
	int i=0;
	while(strarr[i] != NULL)
		i++;
	return i;
	}

//executes basic functions
void my_execute(char **cmd) 
	{
	int status;
	pid_t pid = fork();
	if (pid == -1) 
		{
		//Error
		printf("error in execute\n");
		exit(1);
		}
	else if (pid == 0) 
		{
		//Child
		execv(cmd[0], cmd);
		printf("Problem executing %s\n", cmd[0]);
		exit(1);
		}
	else 
		{
		//Parent
		waitpid(pid, &status, 0);
		}
	}

//returns place of a character in a string
int contchar(char** instr, int numTokens, char cont)
        {
        int check = 0;
        int i;
        for(i = 0; i < numTokens; i++)
                {
                if(instr[i][0] == cont)
                        check = i;
                }
        return check;
        }


//executes a command in the background
void my_backExecute(char **cmd, Back queue) 
	{
	int status;
	pid_t pid = fork();
	if (pid == -1) 
		{
		//Error
		printf("error in execute\n");
		exit(1);
		}
	else if (pid == 0) 
		{
		//Child
		execv(cmd[0], cmd);
		printf("Problem executing %s\n", cmd[0]);
		exit(1);
		}
	else 
		{
		//Parent
		//wait stuff happens outside to keep things simple
		if(waitpid(pid, &status, WNOHANG) ==0)
			{
			queue.queueNum = 0;
			queue.PID = pid;
			strcpy(queue.cmd,cmd[0]);			
			printf("[%d]\t[%d]\n",queue.queueNum,queue.PID);
			}
		}
	}

//does echo 
void echo(char *args){
	if(args[0] != '$'){
		printf("%s", args);
	}
	else{
		char *envVar;
		remFirst(args);
		envVar = getenv(args);
		if(envVar == NULL){
			printf("[Undefined] ");
		}
		else{
			printf("%s ", envVar);
		}
	}
}
