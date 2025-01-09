srcDir = ./src
all: encrypt decrypt
encrypt: twofish.o utils.o $(srcDir)/encrypt.c
	gcc -o encrypt $(srcDir)/encrypt.c twofish.o utils.o
decrypt:
twofish.o: $(srcDir)/twofish.c utils.o
	gcc -c $(srcDir)/twofish.c
utils.o: $(srcDir)/utils.c
	gcc -c $(srcDir)/utils.c
clean:
	rm *.o