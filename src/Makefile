CC=gcc -std=c++17
CXXFLAGS=-Wall -Werror -Wextra -lstdc++ -g
BUILD_DIR=build

all: build_server build_client

build_server: server.pro
	qmake -o server.mk server.pro
	make -f server.mk all

build_client: client.pro
	qmake -o client.mk client.pro
	make -f client.mk all

uninstall: clean
	rm -rf $(BUILD_DIR)

run_server: build_server
	$(BUILD_DIR)/server/server

run_client: build_client
	$(BUILD_DIR)/client/client

clean:
	-rm -rf $(BUILD_DIR)/*/moc $(BUILD_DIR)/*/obj *.o *.gcno *.gcda *.info *.gcov *.a *.dSYM report *.out *.mk files
