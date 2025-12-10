#include "operations.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

static int deleteRegex(Operation op, const char* filename, char* buf, long size);
static int replaceRegex(Operation op, const char* filename, char* buf, long size);

Operation parseInput(const char* input) {
  Operation op = {OPERATION_UNKNOWN, NULL, NULL};
  if (!input) {
    return op;
  }

  long len = strlen(input);

  const char* start = strchr(input, '/');
  if (!start || start-input == len-1) {
    printf("Неверный формат команды!\n");
    return op;
  }
  ++start;

  const char* middle = strchr(start, '/');
  if (!middle || middle-input == len-1) {
    printf("Неверный формат команды!\n");
    return op;
  }
  ++middle;

  const char* end;

  switch (*start) {
  case '$':
    end = strchr(middle, '/');
    if (!end || end - input != len-1 || middle - start != 2) {
      printf("Неверный формат команды!\n");
      return op;
    }
    
    op.type = OPERATION_SUFFIX;
    op.str1 = calloc(end-middle+1, sizeof(char));
    memcpy(op.str1, middle, (end-middle)*sizeof(char));
    break;
  case '^':
    end = strchr(middle, '/');
    if (!end || end - input != len-1 || middle - start != 2) {
      printf("Неверный формат команды!\n");
      return op;
    }
    
    op.type = OPERATION_PREFIX;
    op.str1 = calloc(end-middle+1, sizeof(char));
    memcpy(op.str1, middle, (end-middle)*sizeof(char));
    break;
  default:
    if (*input == 's') {
      end = strchr(middle, '/');
      if (!end || end - input != len-1 || middle - start == 1) {
        printf("Неверный формат команды!\n");
        return op;
      }

      op.type = OPERATION_REPLACE;
      op.str1 = calloc(middle-start, sizeof(char));
      memcpy(op.str1, start, (middle-start-1)*sizeof(char));
      op.str2 = calloc(end-middle+1, sizeof(char));
      memcpy(op.str2, middle, (end-middle)*sizeof(char));
    } else if (input+1 == start) {
      if (*middle != 'd' || middle-input != len-1) {
        printf("Неверный формат команды!\n");
        return op;
      }
    
      op.type = OPERATION_DELETE;
      op.str1 = calloc(middle-start, sizeof(char));
      memcpy(op.str1, start, (middle-start-1)*sizeof(char));
    } else {
      printf("Неверный формат команды!\n");
      return op;
    }
    break;
  }

  return op;
}

void freeOperation(Operation operation) {
  free(operation.str1);
  free(operation.str2);
}

int execute(Operation operation, const char* filename) {
  if (!filename) {
    return 1;
  }

  FILE* file = fopen(filename, "rb");
  if (!file) {
    printf("Не удалось открыть файл %s для чтения!\n", filename);
    return 1;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);

  char* buf = malloc((size+1)*sizeof(char));
  fread(buf, sizeof(char), size, file);
  buf[size] = '\0';
  fclose(file);
  
  if (operation.type == OPERATION_PREFIX || operation.type == OPERATION_SUFFIX) {
    file = fopen(filename, "w");
    if (!file) {
      printf("Не удалось открыть файл %s для записи!\n", filename);
      free(buf);
      return 1;
    }
  }
  
  char* i = buf;
  char* j = buf;
  switch (operation.type) {
  case OPERATION_SUFFIX:
    while (1) {
      for (; j-buf < size && *j != '\n'; ++j);

      if (j-buf == size) {
        if (i-buf != size) {
          fputs(i, file);
          fputs(operation.str1, file);
        }
        break;
      }

      *j = '\0';
      fputs(i, file);
      fputs(operation.str1, file);
      fputc('\n', file);
      i = ++j;
    }
    fclose(file);
    break;
  case OPERATION_PREFIX:
    while (1) {
      for (; j-buf < size && *j != '\n'; ++j);

      if (j-buf == size) {
        if (i-buf != size) {
          fputs(operation.str1, file);
          fputs(i, file);
        }
        break;
      }

      *j = '\0';
      fputs(operation.str1, file);
      fputs(i, file);
      fputc('\n', file);
      i = ++j;
    }
    fclose(file);
    break;
  case OPERATION_DELETE:
    if (deleteRegex(operation, filename, buf, size)) {
      free(buf);
      return 1;
    }
    break;
  default:
    if (replaceRegex(operation, filename, buf, size)) {
      free(buf);
      return 1;
    }
    break;
  }

  free(buf);
  return 0;
}

int deleteRegex(Operation op, const char* filename, char* buf, long size) {
  FILE* file = fopen(filename, "w");
  if (!file) {
    printf("Не удалось открыть файл %s для записи!\n", filename);
    return 1;
  }

  regex_t regex;
  if (regcomp(&regex, op.str1, REG_EXTENDED)) {
    printf("Неверное регулярное выражение!\n");
    fclose(file);
    return 1;
  }
 
  char* i = buf;
  char* j = buf;
  regmatch_t match;
  while (i-buf < size && !regexec(&regex, i, 1, &match, 0)) {
    if (match.rm_eo == match.rm_so) {
      ++i;
      continue;
    }
    fwrite(j, sizeof(char), match.rm_so+i-j, file);
    i += match.rm_eo;
    j = i;
  }

  if (j-buf < size) {
    fwrite(j, sizeof(char), size-(j-buf), file);
  }

  regfree(&regex);
  fclose(file);
  return 0;
}

int replaceRegex(Operation op, const char* filename, char* buf, long size) {
  FILE* file = fopen(filename, "w");
  if (!file) {
    printf("Не удалось открыть файл %s для записи!\n", filename);
    return 1;
  }

  regex_t regex;
  if (regcomp(&regex, op.str1, REG_EXTENDED)) {
    printf("Неверное регулярное выражение!\n");
    return 1;
  }
 
  char* i = buf;
  char* j = buf;
  regmatch_t match;
  while (i-buf < size && !regexec(&regex, i, 1, &match, 0)) {
    if (match.rm_eo == match.rm_so) {
      ++i;
      continue;
    }
    fwrite(j, sizeof(char), match.rm_so+i-j, file);
    fputs(op.str2, file);
    i += match.rm_eo;
    j = i;
  }

  if (j-buf < size) {
    fwrite(i, sizeof(char), size-(j-buf), file);
  }

  regfree(&regex);
  return 0;
}
