all: minitron.exe

%.exe:%.c
	@gcc $< -lncurses -lpthread -O2 -o $@

run: minitron.exe
	@./minitron.exe

clean:
	@rm *.exe

update:
	make clean
	make 
