# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -fPIC
LIB_NAME = libmemory_manager.so

# Source and Object Files
SRC = memory_manager.c linked_list.c test_memory_manager.c test_linked_list.c
OBJ = $(SRC:.c=.o)

# Default target
all: mmanager list test_mmanager test_list

# Rule to create the dynamic library
$(LIB_NAME): memory_manager.o
	$(CC) -shared -o $@ $^

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build the memory manager
mmanager: $(LIB_NAME)

# Build the linked list object
list: linked_list.o

# Test target to run the memory manager test program
test_mmanager: test_memory_manager.o $(LIB_NAME)
	$(CC) -o $@ $^ -L. -lmemory_manager

# Test target to run the linked list test program
test_list: test_linked_list.o $(LIB_NAME) linked_list.o
	$(CC) -o $@ $^ -L. -lmemory_manager
	
# Run tests
run_tests: run_test_mmanager run_test_list
	
# Run test cases for the memory manager
run_test_mmanager:
	./test_memory_manager

# Run test cases for the linked list
run_test_list:
	./test_linked_list

# Clean target to clean up build files
clean:
	rm -f $(OBJ) $(LIB_NAME) test_memory_manager test_linked_list
