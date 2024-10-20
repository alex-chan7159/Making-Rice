CC = clang++
CFLAGS = -g -Wall
BIN = FreeRice

all: $(BIN)

%: %.cpp
	$(CC) $(CFLAGS) $< -o $@ -lcurl

clean:
	$(RM) -rf $(BIN) *.dSYM