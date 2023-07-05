#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<unistd.h>

void normal_comd(char comd[]);
void pipe_comd(char comd[], int p_no);
void redirect_comd(char comd[], int* pipe_fd);
int strocc(char str[], char ch, int* f);
char** create_log(char** l, int s, char comd[]);
void peek_log(char** l, int s);
void Q_shell_op(char** logs, int s, char* comd);

int main(int argc, char* argv[])
{
    system("clear");
    printf("\n       --<><><><><><><><><><><><><( Q-SHELL )><><><><><><><><><><><><>--\n");
    char comd[50], **logs;
    int comd_no = 0;

    while(strcmp(comd, "exit") && strcmp(comd, "EXIT"))
    {
        printf("\nEnter command: ");
        scanf("\n%[^\n]%*c", comd); 

        comd[strlen(comd)] = '\0';
        
        if(strcmp(comd, "exit") != 0 && strcmp(comd, "EXIT") != 0)
        {
            Q_shell_op(logs, comd_no, comd);
            logs = create_log(logs, comd_no++, comd);

            int p_no = strocc(comd, '|', NULL);

            if(!strcmp(comd, "history") || !strcmp(comd, "HISTORY"))
                peek_log(logs, comd_no);

            else if(p_no > 0)
                pipe_comd(comd, p_no);

            else if(strocc(comd, '>', NULL) > 0 || strocc(comd, '<', NULL) > 0)
                redirect_comd(comd, NULL);

            else
                normal_comd(comd); 
        }
    }

    printf("\n\t\t\t   Terminating Q-Shell...\n\n");
    sleep(1.2);
    system("clear");
    return 0;
}

void normal_comd(char comd[])
{
    int i = 1, flag = 0, arg = strocc(comd, ' ', &flag);
    char* rest = comd, args[arg+1][15], *tok;
    tok = strtok_r(comd, " ", &rest);
    strncpy(args[0], tok, 15);
    args[0][14] = '\0';
            
    printf("\ncommand: %s ",  args[0]);
    arg > 0? printf("Arguments: %s  Total-Arguments: %d\n\n", rest, arg):printf("\n\n");
            
    while(i < arg+1)
    {   
        strcpy(comd, rest);
        tok = strtok_r(comd, " ", &rest);

        if(strcmp(tok, "&") == 0)
        flag = 1; //concurrent execution flag

        else
        {
            strncpy(args[i], tok, 15);
            args[i][14] = '\0';
            i++;
        }
    }

    if(fork()) //parrent process
    {
        if(flag == 0)
            wait(NULL);
    }

    else //child process
    {
        int j;
        char* argum[arg+2];
        for(j =0; j < arg + 1; j++)
        {
            argum[j] = args[j];
        }
        argum[arg+1] = NULL;
                
        if(flag == 1) {
            printf("Please wait, executing '%s' command!!\n\n", args[0]);
            sleep(2);
        }

        execvp(args[0], argum);
        printf("Error: Unknown Command!!\n");
        exit(0);
    }
}//-------------------------------------------
