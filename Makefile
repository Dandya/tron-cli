all: minitron.exe

%.exe:%.c
	@gcc $< -lncurses -lpthread -O2 -o $@

withoutcur:
	@gcc minitron.c -DWITHOUTCURSOR -lncurses -lpthread -O2 -o minitronwc.exe


run: minitron.exe
	@./minitron.exe

clean:
	@rm *.exe

update:
	@make clean
	@make 
