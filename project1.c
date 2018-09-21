#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>

char** addToken(char** instr, char* tok, int numTokens);
void printTokens(char** instr, int numTokens);

char *myUSER;
char *myPWD;
char *myMACHINE;
char *myHOME;
char *myROOT;

char **res_loop(char **args,int numtoks);//loop that picks what to resolve and how
char *PATHres(char *oldPath);//resolves variables that start with $PATH
char *absoluteRes(char *oldPath);//returns absolute path of input
char **strArr(char *path, char **strarr, char del);//allocates a 2d array that is null terminated
void freestrArr(char **strarr);//frees 2d array
int exists(char* path);//returns 1 if file exists 0 if it doesnt
char* addHOME(char* oldPath);//adds home to the front of the path
void remFirst(char* old);//removes first character
char* catPaths(char* first, char* second);//concatinates two dynamic allocated paths and stores it in the first
char* getdir(char* dir);//passes in . and returns directory
char* getparent(char* dir);//passes in .. and returns directory
int strarrlength(char** strarr);//returns number of strings in a 2d array
void my_execute(char **cmd); //executes 


int main( int argc, char *argv[] )
        {
	//Setting home directory
	myHOME = getenv("HOME");
	//myROOT = getenv("PWD");
	
	
	//The following code is the parser help code from canvas
	char token[256];	//holds instruction token
	char ** bucket;		//array that holds all instruction tokens
	char temp[256];		//used to split tokens containing special characters

	while(7){
		myUSER = getenv("USER");  //ensure everthing is right before running
        	myPWD = getenv("PWD");
        	myMACHINE = getenv("MACHINE");
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
		//Resolve paths and execute here
		printTokens(bucket, numI);
		res_loop(bucket, numI);
		if(strcmp(bucket[0],"exit")==0)
			{
			printf("Exiting...\n");
			int i;
			for(i = 0; i < numI; i++)
				free(bucket[i]);
			free(bucket);
			return 0;
			}

		else if(strcmp(bucket[0],"cd")==0)
			{
			}
		else
			my_execute(bucket); 

		printTokens(bucket, numI);		
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

void printTokens(char** instr, int numTokens){
	int i;
	printf("Tokens:\n");
	for(i = 0; i < numTokens; i++)
		printf("#%s#\n", instr[i]);
}

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
//			printf("%s is getting checked for . and ..",oldPath);
			if(strcmp(pathArr[i],".") ==0)
				{
				pathArr[i] = getdir(pathArr[i]); 
				printf("abs got %s\n",pathArr[i]);	
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
//		printf("total size to allocate is %d\n",total);
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
//		printf("new abs path is %s\n",temp);
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
//	printf("count is %d\n",count);
	strarr = (char**)calloc(count+1,sizeof(char*));//count plus 1 to allocate for null pointer in last string pointer

	for(i =0;i < count;i++)	//allocating space for each string
		strarr[i] = (char*)calloc(strlen(path),sizeof(char));//max size will be path
//	printf("%d strings allocated\n",count);
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
	printf("array values for %s are:\n",path);
	for(i = 0;i<count;i++)
		printf("%s ",strarr[i]);

	return strarr;		
	}

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

void remFirst(char* old)
	{
	int i=0;
	while(old[i] != '\0')
		{	
		old[i]=old[i+1];
		i++;
		}
	}

char* catPaths(char* first, char* second)
	{
	char *temp;
	temp = (char*)calloc(strlen(first)+strlen(second)+1,sizeof(char));
	strcpy(temp,first);
	strcat(temp, second);
	free(first);
	return temp;
	}

char* getdir(char* dir)//passes in . and returns directory
	{
	char* PWD;
	PWD = (char*)calloc(strlen(getenv("PWD"))+1,sizeof(char));	//gets Path to working directory
	strcpy(PWD,getenv("PWD"));
	free(dir);
	return PWD;


//old code that just got the directory so much work for nothing :(
/*
	char** PWDarr;			//holds string of strings
	remFirst(PWD);
	PWDarr = strArr(PWD,PWDarr,'/');
	int length = strarrlength(PWDarr);
	printf("strings in strarg is %d\n",length);
	printf("value in last is %s\n",PWDarr[length-1]);
	temp = PWDarr[length-1];	//gets last
	PWDarr[length-1] = NULL;	//negates old valuse
	
	free(PWD);
	free(dir);			// frees .
	freestrArr(PWDarr);
	printf("getdir is returning %s",temp);
	return temp;			//returns directory
*/	
	}

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

int strarrlength(char** strarr)
	{
	int i=0;
	while(strarr[i] != NULL)
		i++;
	return i;
	}

void my_execute(char **cmd) 
	{
	int status;
	pid_t pid = fork();
	if (pid == -1) 
		{
		//Error
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
