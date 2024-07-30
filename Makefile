objs  = main.o error.o
std   = -std=c99
avoid = -Wno-switch
optmz = -O0
flags = -Wall -Wextra -Wpedantic $(std) $(avoid) $(optmz)
exec  = s4tb

all: $(exec)

$(exec): $(objs)
	gcc	-o $(exec) $(objs)
%.o: %.c
	gcc	-c $@ $< $(flags)
clean:
	rm	-r $(objs) $(exec)

