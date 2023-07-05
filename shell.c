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
