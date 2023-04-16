sources = $(wildcard *.c)
objects = $(addsuffix .o, $(basename $(sources)))
flags = -g -W -Wall
target = bladeenc
# startflags = -m32

$(target) : $(objects)
	clang $(startflags) -o $(target) $(objects)

%.o : %.c
	clang $(startflags) -c $(flags) $< -o $@
clean :
	rm $(target) $(objects)
