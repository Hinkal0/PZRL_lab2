#include <stdio.h>
#include <stdlib.h>

#include "operations.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Неверное число аргументов!\n");
    return EXIT_FAILURE;
  }

  Operation op = parseInput(argv[2]);
  if (op.type == OPERATION_UNKNOWN) {
    printf("Не удалось распознать команду!\n");
    return EXIT_FAILURE;
  }

  if (execute(op, argv[1]) != EXIT_SUCCESS) {
    printf("Не удалось выполнить операцию!\n");
    freeOperation(op);
    return EXIT_FAILURE;
  }

  freeOperation(op);

  return EXIT_SUCCESS;
}
