#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("wgrep: searchterm [file ...]\n");
    return 1;
  }

  char *search = argv[1];

  if (argc == 2) {
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, stdin) != -1) {
      if (strstr(line, search))
        printf("%s", line);
    }
    free(line);
    return 0;
  }

  for (int i = 2; i < argc; i++) {
    FILE *file = fopen(argv[i], "r");
    if (!file) {
      printf("wgrep: cannot open file\n");
      return 1;
    }

    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, file) != -1) {
      if (strstr(line, search))
        printf("%s", line);
    }
    free(line);
    fclose(file);
  }

  return 0;
}
