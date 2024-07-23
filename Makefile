objs = main.o error.o
exec = s4Term

optmz = -O0
avoid = -Wno-switch
std = -std=c99
flags = -Wall -Wextra -Wpedantic $(optmz) $(avoid) $(std)

all: $(exec)

$(exec): $(objs)
	gcc	-o $(exec) $(objs)
%.o: %.c
	gcc	-c $@ $< $(flags)
clean:
	rm	-f $(objs) $(exec)
