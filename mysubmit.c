#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <assert.h>
/*  mysubmic.c by Caroline Whitman
	3/16/16
	Bugs: * isnt implemented 
*/

typedef enum { false, true } bool;

//prototypes
char* getSubDirectories(char *cnamedir, char *hdir);
char* getHomeDirName(const char *hdir);
char* uploadFilestodir(char *userpath);
bool checkDirectory(char *currpath, char *dirname);
void displayCurrDir(char *endFilePath);
bool prefix(const char *pre, const char *str);
void copyFilesToDirectory(char *fileName, char *endDirectory);
char** str_split(char* a_str, const char a_delim);

int main()
{
	char courseName[20];
	struct stat fileName; 
	struct passwd *pw;
	int count = 0; 

	printf("Please enter the name of your course. (ex: cos350) \n");
	scanf("%s", courseName);

	//Determines if directory exists 
	struct stat s;
	int err = stat(courseName, &s);
	if(-1 == err)
	{
		if(ENOENT == err){	/* does not exist */ } 
		else 
		{
			perror("stat");
			exit(1);
		}
	} 
	else 
	{
		if(S_ISDIR(s.st_mode)) { /* it's a dir */} 
    	else {/* exists but is no dir */}
	}

	//Gets user's name from the system
	pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	//printf("%s is the home directory \n", homedir);
	char *pname;
	pname = getHomeDirName(homedir);

	//get sub directories
	char *userPath;
	userPath = getSubDirectories(courseName, pname);

	//allows user to upload files
	char *finalPath;
	finalPath = uploadFilestodir(userPath);
	
	displayCurrDir(finalPath);

	//User submit 
    char filesToBeSubmitted[250];
    printf("\nEnter the names of the files you would like to submit separated by ',' or * for all files:\n");
	scanf("%s", filesToBeSubmitted);
	char** tokens;
	char cwd[250];
	getcwd(cwd, sizeof(cwd));

    tokens = str_split(filesToBeSubmitted, ',');

    if (tokens)
    {
        int i;
        for (i = 0; *(tokens + i); i++)
        {
            copyFilesToDirectory( *(tokens + i) , finalPath);
            chdir(cwd);
            free(*(tokens + i));
        }
        printf("\n");
        free(tokens);
    }

    chdir(finalPath);
    displayCurrDir(cwd);

	return 0;
}

//Checks the subdirectories of the class for user's name
// and if it doesn't exist, creates it. 
char* getSubDirectories(char *cnamedir, char *hdir)
{	
	//could handle determining if directory exists here
	bool subdirExistsFlag = checkDirectory(cnamedir, hdir);

	char *dirpath = malloc(50);
	strcat(dirpath,cnamedir);
	strcat(dirpath,"/");
	strcat(dirpath,hdir);

	if(subdirExistsFlag == false){
		mkdir(dirpath, S_IRUSR | S_IWUSR | S_IXUSR);
	}

  	return dirpath;
}

char* getHomeDirName(const char *hdir)
{
	char *homedirstring = (char *)hdir;
	char *token;
	char *temp;
	char delims[] = "/";
	token = strtok (homedirstring, delims);

  	while (token != NULL)
  	{	
  		temp = token;
    	token = strtok (NULL, delims);
  	}
  	return temp;
}

//Ask user for name of assignment, create subdirectory if it doesnt exist
char* uploadFilestodir(char *userpath)
{
	char assignmentName[10];
	printf("Please enter the name of your assignment. (ex: prog1) \n");
	scanf("%s", assignmentName);

	bool progdirExistsFlag = checkDirectory(userpath, assignmentName);


	char *progpath = malloc(50);
	strcat(progpath,userpath);
	strcat(progpath,"/");
	strcat(progpath,assignmentName);

	if(progdirExistsFlag == false){
		mkdir(progpath, S_IRUSR | S_IWUSR | S_IXUSR);
	}

	return progpath;
}

bool checkDirectory(char *currpath, char *dirname)
{
	DIR *currdir = opendir(currpath);
	struct dirent *dir_info;
	char fullpath[250];
	bool dirExistsFlag = false;

	//to show directories
	while((dir_info = readdir(currdir)) !=0)
	{
		if(dirname == dir_info->d_name)
		{ 
			dirExistsFlag = true; 
			break;
		}
        strcpy (fullpath, dirname);
        strcat (fullpath, "/");
        strcat (fullpath, dir_info->d_name);
	}

	return dirExistsFlag;
}
// Show user a list of files in their current directory (alphabetically)
void displayCurrDir(char *endFilePath)
{
	struct dirent **entry_list;
	char cwd[250];
	getcwd(cwd, sizeof(cwd));
	
	int i;
	int count;

	count = scandir(cwd, &entry_list, NULL, alphasort);

	 if (count < 0) {
        perror("scandir");
        exit(1);
    }
    struct stat fileInfo; 


 	printf("The files in your current directory are: \n");
    for (i = 0; i < count; i++) {
        struct dirent *entry;
        entry = entry_list[i];

        if(prefix("." , entry->d_name)==0)
        {
        	int holder = stat(entry->d_name, &fileInfo);
    		printf("\t%s  \t%lld\t%s ", entry->d_name, fileInfo.st_size, ctime(&fileInfo.st_mtime));        
        }
        free(entry);
    }

    free(entry_list);

}

void copyFilesToDirectory(char *fileName, char *endDirectory)
{
	int in_fd, out_fd, n_chars; 
	char buf[4096];
	char path[250];
	strcpy(path, endDirectory);
	strcat(path,"/");
	strcat(path, fileName);

	/* open files */
	if ( (in_fd=open(fileName, O_RDONLY)) == -1 ) printf("Cant copy \n");
	chdir(endDirectory);
	chmod (path, 0777);

	if ( (out_fd=creat(fileName, 0644)) == -1 ) printf("Cant paste \n");

	/* copy the file */
	while ( (n_chars = read(in_fd , buf, 4096)) > 0 ) 
		if ( write( out_fd, buf, n_chars ) != n_chars )
		printf("Error \n");
		if ( n_chars == -1 )
		printf("Error \n");
		/* close files */
		if ( close(in_fd) == -1 || close(out_fd) == -1 ) printf("Error closing files \n");
}


char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

