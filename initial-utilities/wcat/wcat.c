#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("wcat: file1 [file2 ...]\n");
    return 0;
  }

  for (int i = 1; i < argc; i++) {
    FILE *file = fopen(argv[i], "r");
    if (!file) {
      printf("wcat: cannot open file\n");
      return 1;
    }

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), file)) {
      printf("%s", buffer);
    }

    fclose(file);
  }

  return 0;
}
