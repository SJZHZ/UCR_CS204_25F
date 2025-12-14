CC = g++
CFLAGS = -Wall -O2 -march=native
TARGET = EDC
SRC = $(TARGET).cpp utils.cpp Linear.cpp BigMod.cpp
SRC_CRC = CRC.cpp 			# Separate CRC implementation file
OBJ = $(SRC:.cpp=.o) $(SRC_CRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Linking complete. Cleaning up object files..."
	@rm -f $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

# CRC.o: CRC.cpp
# 	$(CC) $(CFLAGS) -c $< -o $@ -lz



clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean 