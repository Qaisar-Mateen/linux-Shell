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
    printf("\n       --<><><><><><><><><><><><><( QM-SHELL )><><><><><><><><><><><><>--\n");
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

void pipe_comd(char comd[], int p_no)
{
    int j= 0, pipe_fd[p_no][2], k;
    char *sub_comd[p_no + 1], *r;

    for(j = 0; j < p_no + 1; j++)
    {
        if(j != p_no)
            pipe(pipe_fd[j]); //open pipe
        
        strtok_r(comd, "|", &r);
        if(*comd == ' ')
            comd++;
        sub_comd[j] = malloc(sizeof(char)*(strlen(comd)+1));
        strcpy(sub_comd[j],comd);
        sub_comd[j][strlen(comd)] = '\0';
        if(j != p_no)
            sub_comd[j][strlen(comd)-1] = '\0';

      //  printf("\nsub%d: %s", j+1,sub_comd[j]);
        strcpy(comd, r);
    }

    for(k = 0; k < p_no + 1; k++)
    {
        if(strocc(sub_comd[k], '<', NULL) > 0 || strocc(sub_comd[k], '>', NULL) > 0){
            if(strocc(sub_comd[k], '<', NULL) > 0)
                redirect_comd(sub_comd[k], NULL);
            else
                redirect_comd(sub_comd[k], &pipe_fd[k+1][1]);

        }

        else
        {
            int i = 1, arg = strocc(sub_comd[k], ' ', NULL);
            char* rest = sub_comd[k], args[arg+1][15];
            strtok_r(sub_comd[k], " ", &rest);
            strncpy(args[0], sub_comd[k], 15);
            args[0][14] = '\0';
        
            //printf("\nfor %d command: %s ", k+1, args[0]);
            //arg > 0? printf("Arguments: %s  Total-Arguments: %d\n\n", rest, arg):printf("\n\n");

            while(i < arg+1) //tokenize
            {   
                strcpy(sub_comd[k], rest);
                strtok_r(sub_comd[k], " ", &rest);
                strncpy(args[i], sub_comd[k], 15);
                args[i][14] = '\0';
                i++; 
            }

            if(fork()) //parrent process
            {
                if(k == p_no) //only wait for last sub command
                    wait(NULL);
            }

            else {  //child process
                if(k != 0)
                {
                //   printf("\n k:%d,child pipe%d[0](input)\n",k, k-1);
                    if(dup2(pipe_fd[k-1][0], 0) < 0){
                        perror("error in pipe input");
                        exit(1);          
                    }
                }

                if(k != p_no)
                {
                  //  printf("\n k:%d,child pipe%d[1](output)\n",k, k);
                    if(dup2(pipe_fd[k][1], 1) < 0){
                        perror("error in pipe output");
                        exit(1);          
                    }
                }
                int f;
                for(f = 0; f < p_no; f++){
                    close(pipe_fd[f][0]);
                    close(pipe_fd[f][1]);
                }
    
                char* argum[arg+2];
                for(f = 0; f < arg + 1; f++)
                {
                    argum[f] = args[f];
                }
                argum[arg+1] = NULL;

                execvp(args[0], argum);
                printf("Error: Unknown Command!!\n");
                exit(0);
            }
        }
    }
}//-------------------------------------------

void redirect_comd(char comd[], int *pipe_fd)
{
    if(strocc(comd, '<', NULL) || strocc(comd, '>', NULL)) //checking if redirection exists
    {
        int comd1_size = 0, i = 1, flag = 0, arg;
        for(comd1_size = 0; comd1_size < strlen(comd); comd1_size++) //calculating command size 
        {
            if(comd[comd1_size] == '<' || comd[comd1_size] == '>')
                break;
        }
        char comd_r[comd1_size], op = comd[comd1_size], *file = comd;
        strncpy(comd_r, comd, comd1_size);
        comd_r[comd1_size-1] = '\0';
        strtok_r(comd, &op, &file);
        file++;

        arg = strocc(comd_r, ' ', &flag);
        char* rest = comd_r, args[arg+1][15];
        strtok_r(comd_r, " ", &rest);
        strncpy(args[0], comd_r, 15);
        args[0][14] = '\0';    
        printf("\ncommand: %s ",  args[0]);
        arg > 0? printf("Arguments: %s  Total-Arguments: %d\n", rest, arg):printf("\n");
        printf("File: '%s' operator: '%c' cmd: '%s'\n\n", file, op, comd_r);
            
        while(i < arg+1)
        {   
            strcpy(comd_r, rest);
            strtok_r(comd_r, " ", &rest);
            if(strcmp(comd_r, "&") == 0)
            flag = 1; //concurrent execution flag

            else
            {
                strncpy(args[i], comd_r, 15);
                args[i][14] = '\0';
                i++;
            }   
        }
    
        if(fork()) //parrent process
            wait(NULL);

        else {
            int fd; 
            
            if(op == '>') {
                fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if(fd == -1){
                    perror("Open Error Write");
                    exit(1);
                }
                dup2(fd, 1); //stdout
                if(pipe_fd)
                    dup2(*pipe_fd, 0); //stdin
            }
    
            if(op == '<') {
                fd = open(file, O_RDONLY);
                if(fd == -1){
                    perror("Open Error Read");
                    exit(1);
                }
                dup2(fd, 0); //stdin
                if(pipe_fd)
                    dup2(*pipe_fd, 1); //stdout
            }
            char* argum[arg+2];
            for(i = 0; i < arg + 1; i++)
                argum[i] = args[i];
            argum[arg+1] = NULL;

            execvp(argum[0], argum);
            printf("Error: Unknown Command!!\n");
            exit(0);
        }
    }
}//-------------------------------------------

void redirect_comd(char comd[], int *pipe_fd)
{
    if(strocc(comd, '<', NULL) || strocc(comd, '>', NULL)) //checking if redirection exists
    {
        int comd1_size = 0, i = 1, flag = 0, arg;
        for(comd1_size = 0; comd1_size < strlen(comd); comd1_size++) //calculating command size 
        {
            if(comd[comd1_size] == '<' || comd[comd1_size] == '>')
                break;
        }
        char comd_r[comd1_size], op = comd[comd1_size], *file = comd;
        strncpy(comd_r, comd, comd1_size);
        comd_r[comd1_size-1] = '\0';
        strtok_r(comd, &op, &file);
        file++;

        arg = strocc(comd_r, ' ', &flag);
        char* rest = comd_r, args[arg+1][15];
        strtok_r(comd_r, " ", &rest);
        strncpy(args[0], comd_r, 15);
        args[0][14] = '\0';    
        printf("\ncommand: %s ",  args[0]);
        arg > 0? printf("Arguments: %s  Total-Arguments: %d\n", rest, arg):printf("\n");
        printf("File: '%s' operator: '%c' cmd: '%s'\n\n", file, op, comd_r);
            
        while(i < arg+1)
        {   
            strcpy(comd_r, rest);
            strtok_r(comd_r, " ", &rest);
            if(strcmp(comd_r, "&") == 0)
            flag = 1; //concurrent execution flag

            else
            {
                strncpy(args[i], comd_r, 15);
                args[i][14] = '\0';
                i++;
            }   
        }
    
        if(fork()) //parrent process
            wait(NULL);

        else {
            int fd; 
            
            if(op == '>') {
                fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if(fd == -1){
                    perror("Open Error Write");
                    exit(1);
                }
                dup2(fd, 1); //stdout
                if(pipe_fd)
                    dup2(*pipe_fd, 0); //stdin
            }
    
            if(op == '<') {
                fd = open(file, O_RDONLY);
                if(fd == -1){
                    perror("Open Error Read");
                    exit(1);
                }
                dup2(fd, 0); //stdin
                if(pipe_fd)
                    dup2(*pipe_fd, 1); //stdout
            }
            char* argum[arg+2];
            for(i = 0; i < arg + 1; i++)
                argum[i] = args[i];
            argum[arg+1] = NULL;

            execvp(argum[0], argum);
            printf("Error: Unknown Command!!\n");
            exit(0);
        }
    }
}//-------------------------------------------
