#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char **read_lines(FILE *input, size_t *out_count) {
  size_t capacity = 16;
  size_t count = 0;
  char **lines = malloc(capacity * sizeof(char *));
  if (!lines)
    return NULL;

  char *line = NULL;
  size_t len = 0;
  while (getline(&line, &len, input) != -1) {
    if (count == capacity) {
      capacity *= 2;
      char **tmp = realloc(lines, capacity * sizeof(char *));
      if (!tmp) {
        for (size_t i = 0; i < count; i++)
          free(lines[i]);
        free(lines);
        free(line);
        return NULL;
      }
      lines = tmp;
    }
    lines[count] = strdup(line);
    if (!lines[count]) {
      for (size_t i = 0; i < count; i++)
        free(lines[i]);
      free(lines);
      free(line);
      return NULL;
    }
    count++;
  }
  free(line);
  *out_count = count;
  return lines;
}

void print_lines_reverse(char **lines, size_t count, FILE *out) {
  for (ssize_t i = count - 1; i >= 0; i--) {
    fprintf(out, "%s", lines[i]);
    free(lines[i]);
  }
  free(lines);
}

int main(int argc, char *argv[]) {
  if (argc > 3) {
    fprintf(stderr, "usage: reverse <input> <output>\n");
    return 1;
  }

  if (argc == 1) {
    size_t count;
    char **lines = read_lines(stdin, &count);
    if (!lines) {
      fprintf(stderr, "malloc failed\n");
      return 1;
    }
    print_lines_reverse(lines, count, stdout);
    return 0;
  }

  if (argc == 2) {
    FILE *input = fopen(argv[1], "r");
    if (!input) {
      fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
      return 1;
    }
    size_t count;
    char **lines = read_lines(input, &count);
    fclose(input);
    if (!lines) {
      fprintf(stderr, "malloc failed\n");
      return 1;
    }
    print_lines_reverse(lines, count, stdout);
    return 0;
  }

  if (argc == 3) {
    struct stat s1, s2;
    if (stat(argv[1], &s1) == 0 && stat(argv[2], &s2) == 0) {
      if (s1.st_ino == s2.st_ino && s1.st_dev == s2.st_dev) {
        fprintf(stderr, "reverse: input and output file must differ\n");
        return 1;
      }
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
      fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
      return 1;
    }
    size_t count;
    char **lines = read_lines(input, &count);
    fclose(input);
    if (!lines) {
      fprintf(stderr, "malloc failed\n");
      return 1;
    }

    FILE *output = fopen(argv[2], "w");
    if (!output) {
      fprintf(stderr, "reverse: cannot open file '%s'\n", argv[2]);
      for (size_t i = 0; i < count; i++)
        free(lines[i]);
      free(lines);
      return 1;
    }

    print_lines_reverse(lines, count, output);
    fclose(output);
    return 0;
  }

  return 0;
}
