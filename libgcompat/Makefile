
# Compiler and flags
CXX = gcc
FLAG= 
CFLAGS = -static

# Target names
STATIC_L = gcompat.a
TARGET = ld-linux-x86-64.so.2

# Source and headers
ASM = $(shell find . -maxdepth 1 -type f -name "*.S" | sort)
SRC = $(shell find . -maxdepth 1 -type f -name "*.c" | sort) $(ASM)
OBJ = $(SRC:.c=.o)
HDR = $(shell find . -maxdepth 1 -type f -name "*.h")
.PHONY: all clean 
all: $(TARGET) 

$(TARGET): $(OBJ)
	ar rcs -o $(STATIC_L) $^ /usr/lib/libc.a
	# gcc -shared -fPIC $(STATIC_L) -o $@

%.o: %.c $(HDR)
	$(CXX)  -c $< -o $@ $(CFLAGS)

# Clean all builds
clean:
	rm -f $(OBJ) $(TARGET) $(STATIC_L) *.h.gch 


