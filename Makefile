objs = exprns.o #main.o error.o exprns.o
std = -std=c99
opt = -O0
ign = -Wno-switch
flags = -Wall -Wextra -Wpedantic $(std) $(opt) $(ign)
exec = s4term

all: $(exec)

$(exec): $(objs)
	gcc	-o $(exec) $(objs)

%.o: %.c
	gcc	-c $@ $< $(flags)

clean:
	rm	-f $(objs) $(exec)
