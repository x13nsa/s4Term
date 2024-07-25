objs	= main.o error.o
std	= -std=c99
opt	= -O0
avoid	= -Wno-switch
flags	= -Wall -Wextra -Wpedantic $(std) $(opt) $(avoid)
exec	= s4term

all:	$(exec)

$(exec): $(objs)
	gcc	-o $(exec) $(objs)
%.o: %.c
	gcc	-c $@ $< $(flags)
clean:
	rm	-f $(objs) $(exec)
