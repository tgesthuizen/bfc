# BFC

This is a very minimal brainfuck compiler that targets arm-linux-eabi.
Everything that's armv5 or later should work.
Precisely, the compiler emits thumb code into an ELF relocatable file
that can be linked into an executable by your normal GNU ld or ELF
linker of your choice.

Adjust the Makefile to find a binutils installation targeting
arm-linux-eabi, invoke make and you should be able to run
`qemu-arm hello_world`.
It should print "Hello World".

This is a nice proof of concept of the following things:
- Compilers don't need to be complicated.
- It doesn't need to take weeks until the first executable rolls out
  of your compiler.
- Compilers can generate pretty horrible code (this one is especially
  well demonstrated).
