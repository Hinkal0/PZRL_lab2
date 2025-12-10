CC = gcc
TARGET = sed_simplified
SOURCES = main.c operations.c
OBJECTS = $(SOURCES:.c=.o)
CFLAGS = -c -Wall -Wextra
TEST_FILE = test.txt

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	gcc $^ -o $@

%.o: %.c
	gcc $< $(CFLAGS) -o $@

clean:
	rm -rf *.o $(TARGET)

test: all
	cat $(TEST_FILE)
	./$(TARGET) $(TEST_FILE) 's/$$/ suffix1/'
	cat $(TEST_FILE)
	./$(TARGET) $(TEST_FILE) 's/^/prefix /'
	cat $(TEST_FILE)
	./$(TARGET) $(TEST_FILE) '/prefix /d'
	cat $(TEST_FILE)
	./$(TARGET) $(TEST_FILE) 's/suffix1/suffix2/'
	cat $(TEST_FILE)
	./$(TARGET) $(TEST_FILE) '/ suffix[0-9]/d'
	cat $(TEST_FILE)
