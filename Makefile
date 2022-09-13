ARM_TOOLS := /home/tibbe/src/micros/env/bin

all: myfirstexe

bfc: main.cpp
	g++ -Og -g3 -std=c++17 -fno-exceptions main.cpp -o bfc

crt0.o: crt0.s
	$(ARM_TOOLS)/arm-none-eabi-as $< -o $@

t1.o: t1.bf bfc
	./bfc $< $@

myfirstexe: t1.o crt0.o
	$(ARM_TOOLS)/arm-none-eabi-ld -static $^ -o $@
