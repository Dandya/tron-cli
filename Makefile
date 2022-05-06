all: zmeyka.exe minitron.exe

%.exe:%.c
	@gcc $< -lncurses -lpthread -o $@

run: minitron.exe
	@./minitron.exe

clean:
	@rm *.exe

update:
	make clean
	make 
