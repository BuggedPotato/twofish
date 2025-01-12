srcDir = ./src
all: main
main: twofish.o utils.o $(srcDir)/main.c
	gcc -o main $(srcDir)/main.c twofish.o utils.o
twofish.o: $(srcDir)/twofish.c utils.o
	gcc -c $(srcDir)/twofish.c
utils.o: $(srcDir)/utils.c
	gcc -c $(srcDir)/utils.c
clean:
	rm *.o