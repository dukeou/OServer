objfolder:=obj/
objs:=$(addprefix $(objfolder), $(patsubst %.c,%.o,$(wildcard *.c)))
objscpp:=$(addprefix $(objfolder), $(patsubst %.cpp,%.o,$(wildcard *.cpp)))
headers:=$(wildcard *.h)
binfolder:=bin/
bin:=$(binfolder)webserver
lib:=-lpthread
def:=-DO_SERVER_DEBUG
CPPFLAGS+=-Wall
all: $(bin)
	cp -f $(bin) ~/bin/
$(bin): $(objscpp) $(objs)
	@mkdir -p $(binfolder)
	$(CC) -o $@ $^ $(lib)
$(objs): $(objfolder)%.o: %.c
	@mkdir -p $(objfolder)
	$(CC) $(CPPFLAGS) $(def) -c $< -o $@
$(objscpp): $(objfolder)%.o: %.cpp
	@mkdir -p $(objfolder)
	$(CC) $(CPPFLAGS) $(def) -c $< -o $@

$(objs): $(headers)
$(objscpp): $(headers)

.PHONY: all clean
clean:
	rm -rf $(binfolder) $(objfolder)
