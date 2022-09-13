ARM_TOOLS := /home/tibbe/src/micros/env/bin
PROGRAMS := t1 t2

all: $(PROGRAMS)

bfc: main.cpp
	g++ -Og -g3 -std=c++17 -fno-exceptions main.cpp -o bfc

crt0.o: crt0.s
	$(ARM_TOOLS)/arm-none-eabi-as -mthumb-interwork $< -o $@

%.o: %.bf bfc
	./bfc $< $@

$(PROGRAMS): %: %.o crt0.o
	$(ARM_TOOLS)/arm-none-eabi-ld -static $^ -o $@
