all:
	gcc  -m32 -no-pie -nostdlib -o fib fib.c
	gcc  -m32 -no-pie -nostdlib -o sum sum.c
	gcc  -m32 -no-pie -nostdlib -o prime prime.c
	gcc -m32 -o loader newloader.c

clean:
	-@rm -f fib sum prime loader
