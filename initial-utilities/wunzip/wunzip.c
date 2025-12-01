#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("wunzip: file1 [file2 ...]\n");
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    FILE *file = fopen(argv[i], "r");
    if (!file) {
      fprintf(stderr, "wunzip: cannot open file: %s\n", argv[i]);
      return 1;
    }

    int run;
    char ch;

    while (fread(&run, sizeof(int), 1, file) == 1) {
      if (fread(&ch, sizeof(char), 1, file) != 1)
        break;
      for (int k = 0; k < run; k++)
        putchar(ch);
    }

    fclose(file);
  }

  return 0;
}
