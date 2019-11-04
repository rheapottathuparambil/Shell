/*
Name: Rhea Pottathuparambil
ID: 1001542798
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11    // Mav shell only supports eleven arguments


static void handle_signal(int sig) // Overwrtiting handler
{
  int status;
  if(sig == SIGCHLD)
    waitpid(-1, &status, WNOHANG);
}

int main()
{

  char **hist  = (char**)malloc(15*sizeof(char*));
  
  int hist_index = 0;
  int i;
  for(i=0;i<15;i++)
    hist[i] = (char*)malloc(MAX_COMMAND_SIZE);
  
  int pid_index = 0;
  pid_t pid[15];

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  sigset_t newmask;
  sigemptyset(&newmask);
  sigaddset(&newmask,SIGINT);
  sigaddset(&newmask,SIGTSTP);

  struct sigaction act;
  memset (&act, '\0',sizeof(act));
  act.sa_handler = &handle_signal;

  if (sigaction(SIGCHLD , &act, NULL) < 0) 
  {
    perror ("sigaction: Error ");
    return 1;
  }

  while( 1 )
  {
    /*if(hist_index!=0)
    {
      if((cmd_str[0] == '!') && strcmp(hist[hist_index-1],"history")==0)
      {
        int i; 
        char n[5];
        
        strncpy(n, cmd_str+0, 2);
        n[2] = '\0';
        
        i = (int) n[2];

        if (i < hist_index)
        {
          strcpy(cmd_str,hist[i]);
        }
        else
        {
          printf("Not in history.\n");
        }
      }
    }*/

    if(sigprocmask(SIG_BLOCK, &newmask, NULL)<0)
    {
      perror("sigprocmask ");
    }  
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
    
    

      if(cmd_str[0] == '!')
      {
        char command2[10];
        strcpy(command2,cmd_str);
        int cmd = atoi(strtok(&command2[1]," "));
        //printf("cmd is %d, hist_index = %d\n",cmd,hist_index);
        if((cmd)>hist_index)
        {
          printf("Command not in history.\n");
          continue;
        }
        strcpy(cmd_str,hist[cmd]);
      }


    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    /*int token_index  = 0;
    for( token_index = 0; token_index < token_count; token_index ++ ) 
    {
      printf("token[%d] = %s\n", token_index, token[token_index] );  
    }*/

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
       
    
    pid_t child_pid;
    int status;
    if (token[0]!=NULL)
    {
        
        if(strcmp(token[0],"history")==0)
        {
          for(i=0;i<hist_index;i++)
            printf("%d: %s",i,hist[i]);
        }        
        
        else if ((strcmp(token[0],"quit")==0)||(strcmp(token[0],"exit")==0))
        { 
          exit(0);
        }

        else if(strcmp(token[0],"bg")==0)
        {
          int flag =0;
          for(i=pid_index-1;i>=0;i--)
          {
            int value = kill(pid[i],SIGCONT);
            if(value == 0)
            {
              flag =-1;
              break;
            }
          }
          if(flag == 0)
            printf("bg : no process to background\n");
        }

        else if (strcmp(token[0],"cd")==0)
        {
          if(chdir(token[1])!=0)
          {
            perror("chdir() failed: ");
          } 
        }
        
        else if(strcmp(token[0],"listpids")==0)
        {
          for(i=0;i<pid_index;i++)
          {
            printf("%d: %i\n",i,pid[i]);
          }
        }

        else 
        {
            child_pid = fork();
            if(pid_index == 15)
            {  
              for(i=1; i<15;i++)
              {
                pid[i-1] = pid[i];
              }
              pid[14] = child_pid;
            }
            else
            {
              pid[pid_index] = child_pid;
              pid_index++;
            }
            if (child_pid == 0)
            {
                sigprocmask(SIG_UNBLOCK,&newmask,NULL);
                char string[20];
                
                execvp(token[0],token);
                sleep(20);
                strcpy(string,"/usr/local/bin/");
                strcat(string,token[0]);
                execvp(string,token);
                
                strcpy(string,"/usr/bin/");
                strcat(string,token[0]);
                execvp(string,token);

                strcpy(string,"/bin");
                strcat(string,token[0]);
                execvp(string,token);

                printf("%s : Command not found.\n\n",token[0]);
                exit(0);
            }
            sigprocmask(SIG_BLOCK,&newmask,NULL);
            waitpid(child_pid, &status, 0);
        } 
        if(strcmp(cmd_str,"")!=0)
        {
          if(hist_index == 15)
          {  
            for(i=1; i<15;i++)
            {
              strcpy(hist[i-1],hist[i]);
            }
            strcpy(hist[14],cmd_str);
          }
          else
          {
            strcpy(hist[hist_index],cmd_str);
            hist_index++;
          }
        }
    }
  free( working_root );
  }
return 0;
}
