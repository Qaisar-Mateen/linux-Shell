# linux-Shell
A Linux Shell (QM-Shell), coded in C language 
it can run all linux Shell commands including some custom commands. It also handels shell pipes and input/output and error redirection and can also run their combination sucha as "command1 | command 2 | command3 > file.txt"

CUSTOM COMMANDS:
Following are the custom commands supported.
$ History
ths command will display the most recent 10 commands run on my shell.

As an example, assume that the history consists of the commands (from most to least recent):
ps, ls -I, top, cal, who, date
The command history will output:

6: ps
5: ls -l
4: top
3: cal
2: who
1: date

and it supports two techniques for retrieving commands from the command history:
1. When the user enters !!, the most recent command in the history is executed.
2. When the user enters a single! followed by an integer N, the Nth command in the history is
executed. (!4 will run top command)
