# Makefile

all: server client

CFLAGS = -w
# Compileaza server.c
server: server.cpp

# Compileaza client.c
client: client.cpp

.PHONY: clean run_server run_client

# Ruleaza serverul
run_server:
	./server

# Ruleaza clientul
run_client:
	./client
clean:
	rm -f server client
