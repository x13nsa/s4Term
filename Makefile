objs  = s4t-b.o
optmz = -O0
std   = -std=c99
wNos  = -Wno-switch
flags = -Wall -Wextra -Wpedantic $(optmz) $(std) $(wNos)
exec  = s4t-b

all: $(exec)

$(exec): $(objs)
	gcc	-o $(exec) $(objs)
%.o: %.c
	gcc	-c $@ $< $(flags)
clean:
	rm	-f $(exec) $(objs)
