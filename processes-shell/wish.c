#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void print_error() {
  write(STDERR_FILENO, "An error has occurred\n",
        strlen("An error has occurred\n"));
}

char *find_executable(char *command, char **shell_path) {
  char full_path[1024];
  for (int i = 0; shell_path[i] != NULL; i++) {
    snprintf(full_path, sizeof(full_path), "%s/%s", shell_path[i], command);
    if (access(full_path, X_OK) == 0) {
      return strdup(full_path);
    }
  }
  return NULL;
}

int handle_builtins(char **args, char **shell_path) {
  if (strcmp(args[0], "exit") == 0) {
    if (args[1] == NULL) {
      exit(0);
    } else {
      print_error();
    }
    return 1;
  }

  if (strcmp(args[0], "cd") == 0) {
    if (args[1] != NULL && args[2] == NULL) {
      if (chdir(args[1]) == -1) {
        print_error();
      }
    } else {
      print_error();
    }
    return 1;
  }

  if (strcmp(args[0], "path") == 0) {
    int folder_index = 0;
    while (shell_path[folder_index] != NULL) {
      free(shell_path[folder_index]);
      shell_path[folder_index] = NULL;
      folder_index++;
    }

    if (args[1] == NULL)
      return 1;

    folder_index = 0;
    int arg_index = 1;
    while (args[arg_index] != NULL) {
      shell_path[folder_index] = strdup(args[arg_index]);
      folder_index++;
      arg_index++;
    }
    shell_path[folder_index] = NULL;
    return 1;
  }

  return 0;
}

void process_single_command(char *raw_cmd, char **shell_path) {
  char *redirect_file = NULL;
  char *ptr = raw_cmd;

  if (strchr(ptr, '>')) {
    char *command_side = ptr;
    char *file_side = ptr;

    command_side = strsep(&file_side, ">");

    if (file_side != NULL && strchr(file_side, '>')) {
      print_error();
      return;
    }

    char *file_token;
    int file_counter = 0;
    char *temp_file_side = file_side;

    while ((file_token = strsep(&temp_file_side, " \t")) != NULL) {
      if (*file_token != '\0') {
        if (file_counter == 0) {
          redirect_file = file_token;
        }
        file_counter++;
      }
    }

    if (file_counter != 1) {
      print_error();
      return;
    }

    ptr = command_side;
  }

  char *args[128];
  int i = 0;
  char *token;

  while ((token = strsep(&ptr, " \t")) != NULL) {
    if (*token != '\0') {
      args[i++] = token;
    }
  }
  args[i] = NULL;

  if (args[0] == NULL) {
    if (redirect_file != NULL) {
      print_error();
    }
    return;
  }

  if (handle_builtins(args, shell_path) == 1) {
    return;
  }

  char *executable_path = find_executable(args[0], shell_path);

  if (executable_path == NULL) {
    print_error();
    return;
  }

  pid_t pid = fork();

  if (pid == 0) {
    if (redirect_file != NULL) {
      int fd = open(redirect_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
      if (fd == -1) {
        print_error();
        exit(1);
      }
      dup2(fd, STDOUT_FILENO);
      dup2(fd, STDERR_FILENO);
      close(fd);
    }

    int err = execv(executable_path, args);
    if (err == -1) {
      print_error();
      exit(1);
    }
  } else if (pid > 0) {
    free(executable_path);
  } else {
    print_error();
    free(executable_path);
  }
}

int main(int argc, char *argv[]) {
  FILE *input_stream = stdin;

  if (argc > 2) {
    print_error();
    return 1;
  }

  if (argc == 2) {
    FILE *input = fopen(argv[1], "r");
    if (!input) {
      print_error();
      return 1;
    }
    input_stream = input;
  }

  char *line = NULL;
  size_t len = 0;

  char *shell_path[128];
  shell_path[0] = strdup("/bin");
  shell_path[1] = NULL;

  while (1) {
    if (argc == 1) {
      printf("wish> ");
    }

    ssize_t nread = getline(&line, &len, input_stream);

    if (nread == -1) {
      free(line);
      if (argc == 2) {
        fclose(input_stream);
      }
      break;
    }

    if (nread > 0 && line[nread - 1] == '\n') {
      line[nread - 1] = '\0';
    }

    char *raw_commands[128];
    int cmd_count = 0;
    char *ptr = line;
    char *command;

    while ((command = strsep(&ptr, "&")) != NULL) {
      if (*command != '\0') {
        raw_commands[cmd_count] = command;
        cmd_count++;
      }
    }

    for (int current_cmd = 0; current_cmd < cmd_count; current_cmd++) {
      process_single_command(raw_commands[current_cmd], shell_path);
    }

    while (wait(NULL) > 0)
      ;
  }

  return 0;
}