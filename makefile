sniffd: sniffd.o ds
	gcc sniffd.o red_black_tree.o stack.o misc.o data_structure.o -o sniffd
sniffd.o: sniffd.c
	gcc -c sniffd.c -Wall
ds: red_blk
	gcc -c data_structure.c
red_blk: red_black_tree.c stack.c misc.c
	gcc -c red_black_tree.c stack.c misc.c
cli: cli.o
	gcc cli.o -o cli
cli.o: cli.c
	gcc -c cli.c -Wall
clean:
	rm *.o sniffd cli
