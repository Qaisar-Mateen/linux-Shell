#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<unistd.h>

void normal_comd(char comd[]);
void pipe_comd(char comd[], int p_no);
void redirect_comd(char comd[]);
int strocc(char str[], char ch, int* f);
void p_r_comd(char comd[], int p_no);
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
        fflush(stdin);
        fflush(stdout);

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
                redirect_comd(comd);

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

    if(strocc(comd, '<', NULL) > 0 || strocc(comd, '>', NULL) > 0) //if it also have redirection
    {
        p_r_comd(comd, p_no);
    }

    else
    {
        int j= 0, pipe_fd[p_no][2], k;
        char *sub_comd[p_no + 1], *r;

        for(j = 0; j < p_no + 1; j++) //extracts subcommands
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
            int i = 1, arg = strocc(sub_comd[k], ' ', NULL);
            char* rest = sub_comd[k], args[arg+1][15];
            strtok_r(sub_comd[k], " ", &rest);
            strncpy(args[0], sub_comd[k], 15);
            args[0][14] = '\0';

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
                    if(dup2(pipe_fd[k-1][0], 0) < 0){
                        perror("error in pipe input");
                        exit(1);          
                    }
                }

                if(k != p_no)
                {
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

        int f;
        for(f = 0; f < p_no; f++){
            close(pipe_fd[f][0]);
            close(pipe_fd[f][1]);
        }
        sleep(0.95);
    }
}//-------------------------------------------

void redirect_comd(char comd[])
{
    if(strocc(comd, '<', NULL) || strocc(comd, '>', NULL)) //checking if redirection exists
    {
        int comd1_size = 0, i = 1, flag = 0, arg, num = -1;
        for(comd1_size = 0; comd1_size < strlen(comd); comd1_size++) //calculating command size 
        {
            if(comd[comd1_size] == '<' || comd[comd1_size] == '>')
                break;
        }
        char op = comd[comd1_size];
        if(comd[comd1_size-1] == '1' || comd[comd1_size-1] == '2' || comd[comd1_size-1] == '0'){
            num = atoi(&comd[comd1_size-1]);
            comd1_size--;
        }
        char comd_r[comd1_size], *file = comd;
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
            
        while(i < arg+1) //tokenize
        {   
            strcpy(comd_r, rest);
            strtok_r(comd_r, " ", &rest);
            strncpy(args[i], comd_r, 15);
            args[i][14] = '\0';
            i++;  
        }
    
        if(fork()) //parrent process
            wait(NULL);

        else {
            int fd; 
            
            if(op == '>') {
                fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);

                if(num == -1 || num == 1){
                    if(fd == -1)
                        perror("Open Error Write");
                    dup2(fd, 1); //stdout
                }

                else if(num == 2){
                    if(fd == -1)
                        perror("Open Error Write");
                    dup2(fd, num); //stderr
                }
            }
    
            if(op == '<') {
                fd = open(file, O_RDONLY);
                if(fd == -1)
                    perror("Open Error Read");
                dup2(fd, 0); //stdin
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

int strocc(char str[], char ch, int* f) //find all the occurance of the character passed in string 
{
    int i, occ = 0;
    for(i = 0; i< strlen(str); i++)
    {
        if(str[i] == ch && str[i+1] != '&')
            occ++;
        if(f && str[i+1] == '&')
            *f = 1;
    }
    return occ;
}//-------------------------------------------

void p_r_comd(char comd[], int p_no)
{
    char cm[strlen(comd)+1];
    strcpy(cm,comd);
    cm[strlen(cm)] = '\0';

    int j= 0, pipe_fd[p_no][2], k;
    char *sub_comd[p_no + 1], *r;

    for(j = 0; j < p_no + 1; j++) //extracts subcommands
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

    for(j = 0; j < p_no; j++){
        close(pipe_fd[j][0]);
        close(pipe_fd[j][1]);
    }

    for(k  = 0; k <= p_no; k++)
    {
        if(strocc(sub_comd[k], '<', NULL) || strocc(sub_comd[k], '>', NULL))
        {
            if((k == 0 && strocc(sub_comd[k], '<', NULL)) || (k == p_no && strocc(sub_comd[k], '>', NULL)))
            {
                int cmd_s = 0, i = 1, flag = 0, arg, num = -1;
                for(cmd_s = 0; cmd_s < strlen(sub_comd[k]); cmd_s++) //calculating command size 
                {
                    if(sub_comd[k][cmd_s] == '<' || sub_comd[k][cmd_s] == '>')
                    break;
                }
                char op = sub_comd[k][cmd_s];
                if(sub_comd[k][cmd_s-1] == '1' || sub_comd[k][cmd_s-1] == '2' || sub_comd[k][cmd_s-1] == '0'){
                    num = atoi(&sub_comd[k][cmd_s-1]);
                    cmd_s--;
                }
                char comd_r[cmd_s], *file = sub_comd[k];
                strncpy(comd_r, sub_comd[k], cmd_s);
                comd_r[cmd_s-1] = '\0';
                strtok_r(sub_comd[k], &op, &file);
                file++;

                arg = strocc(comd_r, ' ', &flag);
                char* rest = comd_r, args[arg+1][15];
                strtok_r(comd_r, " ", &rest);
                strncpy(args[0], comd_r, 15);
                args[0][14] = '\0';    
                printf("\ncommand: %s ",  args[0]);
                arg > 0? printf("Arguments: %s  Total-Arguments: %d\n", rest, arg):printf("\n");
                printf("File: '%s' operator: '%c' cmd: '%s'\n\n", file, op, comd_r);
            
                while(i < arg+1) //tokenize
                {   
                    strcpy(comd_r, rest);
                    strtok_r(comd_r, " ", &rest);
                    strncpy(args[i], comd_r, 15);
                    args[i][14] = '\0';
                    i++;  
                }
                if(fork()) //parrent process
                    wait(0);

                else {
                    
                    char* argum[arg+2];
                    exit(0);
                    for(i = 0; i < arg + 1; i++)
                        argum[i] = args[i];
                    system(cm);                        
                    argum[arg+1] = NULL;

                    execvp(argum[0], argum);
                    printf("Error: Unknown Command!!\n");
                    exit(0);
                }
            }

            else
            {
                printf("Error: This combination of redirection is not possible with pipes!!\n");
                exit(1);
            }
        }
    }
}//-------------------------------------------

char** create_log(char **l, int s, char comd[]) //manages commands logs creation
{
    if(s == 0) //first command case
    {
        l = malloc(sizeof(char*));
        l[s] = malloc(sizeof(char)*(strlen(comd)+1));
        
        if(l[s] == NULL){
            perror("malloc failed");
            exit(1);
        }
        strcpy(l[s], comd);
        l[s][strlen(comd)] = '\0';

        return l;
    }

    char** new = malloc(sizeof(char*)*(s+1));
    int i = 0;
    for(i = 0; i < s; i++){
        new[i] = malloc(sizeof(char)*(strlen(l[i])+1));
        if(new[i] == NULL){
            perror("malloc failed");
            exit(1);
        }
        strcpy(new[i], l[i]);
        new[i][strlen(new[i])] = '\0';
    }

    new[s] = malloc(sizeof(char)*(strlen(comd)+1));
    strcpy(new[s], comd);
    new[s][strlen(comd)] = '\0';

    for(i = 0; i < s; i++)  //de-allocating previous log.
        free(l[i]);
    free(l);
    
    return new;
}//-------------------------------------------

void peek_log(char** l, int s) //prints atmost 10 recent logs
{
    if(s==0) {
        printf("\nError: No Commands in History \n"); 
        return;}
    
    int i, end = 0;
    printf("\n       ------------");
    printf("\n      /Recent LOGS/\n");
    printf("       ------------");

    end = (s <= 10 && s >= 0)?  0 : s-10; //to print only recent 10

        for(i = s-1; i >= end; i--)
            printf("\n-%d\t%s", i+1, l[i]);

    printf("\n");        
}//-------------------------------------------

void Q_shell_op(char** logs, int s, char* comd)
{
    if(logs && (strlen(comd) == 2 || strlen(comd) == 3))
    {
        if(!strcmp(comd, "!!")) //if command is !!
        {
            if(s == 0) {
                printf("\nError: No Commands in History \n"); 
                return;
            }

            comd[0] = '\0';
            strcpy(comd, logs[s-1]);
            comd[strlen(comd)] = '\0';
        }
        
        if(comd[0] == '!')
        {
            char num[3] = {comd[1], comd[2], '\0'};
            int ind = atoi(num);

            if(ind > 0 && (ind <= s && ind > s-10))
            {
                comd[0] = '\0';
                strcpy(comd, logs[ind-1]);
                comd[strlen(comd)] = '\0';
                if(strlen(comd) == 2 || strlen(comd) == 2)
                    Q_shell_op(logs, s, comd);
            }

            else{
                printf("\nCan't execute command(%d): %s", ind, comd);
                printf("\nError: Invalid Command no. \n");
                //strcpy(comd, "NULL");
                return;
            }
        }
    }
}//-------------------------------------------
