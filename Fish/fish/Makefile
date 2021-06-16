CC=gcc
CFLAGS = -Wall -std=c99 -o2 -g
LDFLAGS = -g
TARGET1=fish
TARGET2=cmdline_test

all : $(TARGET1) $(TARGET2)

# target 1 fish
$(TARGET1) : fish.o libcmdline.so
	$(CC) $(LDFLAGS) -L${PWD} $< -Wl,-rpath=${PWD} -lcmdline -o $@

# target 2 cmdtest
$(TARGET2) :  cmdline_test.o libcmdline.so
	$(CC) $(LDFLAGS) -L${PWD} $< -Wl,-rpath=${PWD} -lcmdline -o $@

libcmdline.so : cmdline.o
		$(CC) -shared $^ -o $@

# library
cmdline.o : cmdline.c cmdline.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

cmdline_test.o : cmdline_test.c cmdline.h
	$(CC) $(CFLAGS) -c $< -o $@

fish.o : fish.c fish.h
	$(CC) $(CFLAGS) -D_DEFAULT_SOURCE -c $< -o $@

clean :
	rm -f *.o

mrproper : clean
	rm -f $(TARGET1) $(TARGET2)
