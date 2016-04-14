
all: main.c
	gcc -pthread -Wno-cpp -std=c99 -o main main.c
	chmod 755 main

clean:
	-rm main
	
