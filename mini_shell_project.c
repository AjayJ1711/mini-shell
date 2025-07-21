#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void executeCD(char *command, char *file_path, size_t path_size)
{
    char *arg = command + 2;
    while(*arg == ' ')
    {
        arg++;
    }

    if(strlen(arg) == 0)
    {
        char *home = getenv("HOME");
        if(home && chdir(home) == 0)
        {
            getcwd(file_path, path_size);
        }
        else
        {
            perror("chdir");
        }
    }
    else if(strcmp(arg, ".") == 0)
    {
        // Do nothing
    }
    else if(strcmp(arg, "..") == 0)
    {
        if(chdir("..") == 0)
        {
            getcwd(file_path, path_size);
        }
        else
        {
            perror("chdir");
        }
    }
    else
    {
        if(arg[0] != '/')
        {
            char temp[256];
            snprintf(temp, sizeof(temp), "%s/%s", file_path, arg);
            arg = temp;
        }

        if(chdir(arg) == 0)
        {
            getcwd(file_path, path_size);
        }
        else
        {
            perror("chdir");
        }
    }
}

void executeCommand(char *command)
{
    if(strstr(command, ">") || strstr(command, ">>") || strstr(command, "<"))
    {
        int redirect_type = -1;
        if(strstr(command, ">>"))
        {
            redirect_type = 2;
        }
        else if(strstr(command, ">"))
        {
            redirect_type = 1;
        }
        else if(strstr(command, "<"))
        {
            redirect_type = 3;
        }

        char *com = strtok(command, "><");
        char *file_name = strtok(NULL, "><");

        if(!file_name)
        {
            fprintf(stderr, "Redirection error.\n");
            exit(1);
        }

        while(*file_name == ' ')
        {
            file_name++;
        }
        file_name[strcspn(file_name, "\n")] = '\0';

        int fd;
        if(redirect_type == 2)
        {
            fd = open(file_name, O_CREAT | O_WRONLY | O_APPEND, 0644);
        }
        else if(redirect_type == 1)
        {
            fd = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        }
        else if(redirect_type == 3)
        {
            fd = open(file_name, O_RDONLY);
        }
        else
        {
            fprintf(stderr, "Unknown redirection type.\n");
            exit(1);
        }

        if(fd < 0)
        {
            perror("open");
            exit(1);
        }

        if(redirect_type == 3)
        {
            dup2(fd, STDIN_FILENO);
        }
        else
        {
            dup2(fd, STDOUT_FILENO);
        }
        close(fd);

        char *argv[10];
        int k = 0;
        char *tok = strtok(com, " ");

        while(tok != NULL)
        {
            argv[k++] = tok;
            tok = strtok(NULL, " ");
        }
        argv[k] = NULL;

        execvp(argv[0], argv);
        perror("command failed");
        exit(1);
    }
    else
    {
        char *argv[10];
        int k = 0;
        char *tok = strtok(command, " ");

        while(tok != NULL)
        {
            argv[k++] = tok;
            tok = strtok(NULL, " ");
        }
        argv[k] = NULL;

        execvp(argv[0], argv);
        perror("command failed");
        exit(1);
    }
}

int main()
{
    char file_path[128];
    if(getcwd(file_path, sizeof(file_path)) == NULL)
    {
        perror("getcwd");
        return 1;
    }

    while(1)
    {
        printf("%s\nmysh> ", file_path);
        fflush(stdout);

        char command[256];
        if(fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }

        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n')
        {
            command[len - 1] = '\0';
        }

        if(strcmp(command, "exit") == 0)
        {
            break;
        }

        if(strncmp(command, "cd", 2) == 0)
        {
            executeCD(command, file_path, sizeof(file_path));
            continue;
        }

        if(strstr(command, "|"))
        {
            char *commands[10];
            int k = 0;
            char *tok = strtok(command, "|");

            while(tok != NULL && k < 10)
            {
                while(*tok == ' ')
                {
                    tok++;
                }
                commands[k++] = tok;
                tok = strtok(NULL, "|");
            }

            int prev_fd = -1;
            for(int i = 0; i < k; i++)
            {
                int pipefd[2];
                if(i != k - 1)
                {
                    if(pipe(pipefd) == -1)
                    {
                        perror("pipe");
                        exit(1);
                    }
                }

                pid_t pid = fork();
                if(pid < 0)
                {
                    perror("fork");
                    exit(1);
                }

                if(pid == 0)
                {
                    if(prev_fd != -1)
                    {
                        dup2(prev_fd, STDIN_FILENO);
                        close(prev_fd);
                    }

                    if(i != k - 1)
                    {
                        dup2(pipefd[1], STDOUT_FILENO);
                        close(pipefd[0]);
                        close(pipefd[1]);
                    }

                    executeCommand(commands[i]);
                }

                if(prev_fd != -1)
                {
                    close(prev_fd);
                }
                if(i != k - 1)
                {
                    close(pipefd[1]);
                    prev_fd = pipefd[0];
                }
                wait(NULL);
            }
        }
        else
        {
            pid_t pid = fork();
            if(pid < 0)
            {
                perror("fork");
                continue;
            }

            if(pid == 0)
            {
                executeCommand(command);
            }
            else
            {
                wait(NULL);
            }
        }
    }
    return 0;
}