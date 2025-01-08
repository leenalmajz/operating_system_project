# Define the compiler
CC = gcc

# Define the compiler flags
CFLAGS = -Wall -Wextra -O2

# Define the target executable name
TARGET = main

# Define the source files
SRC = main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Rule to run the executable
run: $(TARGET)
	./$(TARGET)

# Rule to clean up the build files
clean:
	rm -f $(TARGET)