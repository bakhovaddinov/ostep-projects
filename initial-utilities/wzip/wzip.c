#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("wzip: file1 [file2 ...]\n");
    return 1;
  }

  int current_char = 0;
  int run_length = 0;

  for (int i = 1; i < argc; i++) {
    FILE *file = fopen(argv[i], "r");
    if (!file) {
      fprintf(stderr, "wzip: cannot open file: %s\n", argv[i]);
      return 1;
    }

    int ch;
    while ((ch = fgetc(file)) != EOF) {
      if (run_length == 0) {
        current_char = ch;
        run_length = 1;
      } else if (ch == current_char) {
        run_length++;
      } else {
        fwrite(&run_length, sizeof(int), 1, stdout);
        fputc(current_char, stdout);
        current_char = ch;
        run_length = 1;
      }
    }

    fclose(file);
  }

  if (run_length > 0) {
    fwrite(&run_length, sizeof(int), 1, stdout);
    fputc(current_char, stdout);
  }

  return 0;
}
