sources = $(wildcard *.c)
objects = $(addsuffix .o, $(basename $(sources)))
flags = -g -W -Wall
target = libblade.a
# startflags = -m32

$(target) : $(objects)
	ar rcs $(target) $(objects)

%.o : %.c
	clang $(startflags) -c $(flags) $< -o $@
clean :
	rm $(target) $(objects)
