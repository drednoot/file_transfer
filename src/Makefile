CC=gcc -std=c++17
CXXFLAGS=-Wall -Werror -Wextra -lstdc++ -g
BUILD_DIR=build

all: server client

server: server.pro
	qmake -o server.mk server.pro
	make -f server.mk all

client: client.pro
	qmake -o client.mk client.pro
	make -f client.mk all

uninstall: clean
	rm -rf $(BUILD_DIR)

run_server: server
	$(BUILD_DIR)/server

run_client: client
	$(BUILD_DIR)/client

clean:
	-rm -rf $(BUILD_DIR)/moc $(BUILD_DIR)/obj *.o *.gcno *.gcda *.info *.gcov *.a *.dSYM report *.out *.mk
