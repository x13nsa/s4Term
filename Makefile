objs  = s4tb.o error.o
opt   = -O0
std   = -std=c99
flags = -Wall -Wextra -Wpedantic $(opt) $(std) -Wno-switch
exec  = s4tb

all: $(exec)

$(exec): $(objs)
	gcc	-o $(exec) $(objs)
%.o: %.c
	gcc	-c $@ $< $(flags)
clean:
	rm	-f $(objs) $(exec)
