#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include "strmap.h"

int file_flag[25]={0};//set flag for 25 files
pthread_mutex_t mutex;
pthread_mutex_t map_lock;
int ip_num;
StrMap *map;

int check_file(char filename[])
{
  pthread_mutex_lock(&mutex);//add lock
  char buf[2];
  buf[0]=filename[6];
  if(filename[7]!='.')
     {buf[1]=filename[7];}
  int tmp=atoi(buf);
  if(file_flag[tmp-1]==0)
  {
    file_flag[tmp-1]=1;
    pthread_mutex_unlock(&mutex);//unlock
    return 1;
  }
  else 
  {
   pthread_mutex_unlock(&mutex);//unlock
   return 0;
  }
};

void *open_dir_read_file(void* arg)
{ 
        char* open_add=(char *)arg;
        DIR *dir;	//directory stream
	      FILE *file;	//file stream
	      struct dirent *ent;	// directory entry structure
        char *line = NULL;	// pointer to 
        size_t len = 1000;	//the length of bytes getline will allocate
        size_t read;
        long lines=0;
        char* ip=NULL;
	      char full_filename[256];	//will hold the entire file name to read
        // try to open the directory given by the argument
	if ((dir = opendir (open_add)) != NULL) 
	{
	  	/* print all the files and directories within directory */
	  	while ((ent = readdir (dir)) != NULL) 
		{  
	    	 	//printf ("%s\n", ent->d_name);
			// Check if the list is a regular file
			if(ent->d_type == DT_REG)
			{
				// Create the absolute path of the filename
				snprintf(full_filename, sizeof full_filename, "./%s%s\0", open_add, ent->d_name);				
				// open the file
                              if(check_file(ent->d_name)==1)//check if this file was read and use mutex
                                {
                                    int tmp_num=0;
                                    
                                    FILE* file = fopen(full_filename, "r");
                                   // file was not able to be open
                                   if (file != NULL)
                                   {
	                              // Print out each line in the file
	                                     while ((read = getline(&line, &len, file)) != -1) 				
                                        {
                                             lines++;
	                                     ip = strtok (line, " ");
	                                     pthread_mutex_lock(&map_lock); // lock the mutex associated with minimum_value and update the variable as required
	                                     int flag = sm_exists(map, ip); // sm_get
	                                     if (flag == 0) 
                                                  { // Not found, add ip into map
	                                                sm_put(map, ip, "");
	                                                ip_num++; // Increase the global ip num
                                                  tmp_num++;
	                                                }
	                                     pthread_mutex_unlock(&map_lock); // unlock the mutex
		                                    }
                                       printf("%s  this file's adding num: %d\n", ent->d_name,tmp_num);
	                                     fclose(file);    
	                                 }
                                     
                                 }	
			}
	  }
		// Close the directory structure
	  	closedir (dir);
	} 
	else 
	{
	  	/* could not open directory */
	  	perror ("");
  		//return -1;
	}
};


int main(int argc, char *argv[])
{
	// check the arguments
	if(argc < 2)
	{
		printf("Not enough arguments supplied\n");
		return -1;
	}

	if(argc > 2)
	{
		printf("Too many arguments supplied\n");
		return -1;
	}
        map = sm_new(300);
        printf("%s\n",argv[1]);
        pthread_mutex_init(&mutex,NULL);
        pthread_mutex_init(&map_lock, NULL);
        int ret_1,ret_2;
        pthread_t tid_1,tid_2;
        ret_1=pthread_create(&tid_1,NULL,open_dir_read_file,(void *)argv[1]);
        ret_2=pthread_create(&tid_2,NULL,open_dir_read_file,(void *)argv[1]);
        pthread_join(tid_1,NULL);
        pthread_join(tid_2,NULL);
        printf("ip numbers: %d\n",ip_num);
        pthread_exit(NULL);
        return 0;
}
