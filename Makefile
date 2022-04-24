all: zmeyka.exe minitron.exe

%.exe:%.c
	@gcc $< -lncurses -lpthread -o $@

run: zmeyka.exe
	@./zmeyka.exe

clean:
	@rm *.exe
