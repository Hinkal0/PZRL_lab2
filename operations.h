#pragma once

typedef enum {
  OPERATION_SUFFIX,
  OPERATION_PREFIX,
  OPERATION_DELETE,
  OPERATION_REPLACE,
  OPERATION_UNKNOWN
} OperationType;

typedef struct {
  OperationType type;
  char* str1;
  char* str2;
} Operation;

Operation parseInput(const char* input);

void freeOperation(Operation operation);

int execute(Operation operation, const char* filename);
