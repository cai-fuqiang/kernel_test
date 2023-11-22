all:main
src := $(wildcard *.c)
obj := $(src:.c=.o)

main:$(obj)
	gcc -o $@ $^  

$(obj):%.o:%.c
	gcc -c $< -o $@ -g

clean:
	rm -f *.o main
