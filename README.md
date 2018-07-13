# 8S1A Assembler
This program takes in assembly code that you write and turns it into CPU instructions that you can run.

# How to use
First, write the code in a file (obviously), making sure to use the syntax below. Then run the assembler with the source file and the destination binary file. Then you can use any binary editor to view the output code and run it on the CPU.

# Syntax
The source file can be split into five sections. These are denoted with the syntax `section <name>`, where `<name>` is the name of the section. Any text that goes before the first section will be ignored when assembling.

## `.spec`
This section defines what instructions are available on the CPU. There are two types of entries: default specs and new instructions. The default spec is defined with `spec <8S1A|ROM>`. `ROM` adds the `jmp` and `ret` calls to the specification. New instructions are defined with `inst <name>,<val>`, where `<name>` is the name of the instruction and `<val>` is what the instruction will be assembled to.

## `.data`
This section creates constants in the file that act like C macros. Any instance of the constant name will be replaced with the value. The syntax is `<name> <value>`, where `<name>` is the name of the constant and `<value>` is what the assembler will replace it with.

## `.bss`
This section defines named variables that can be used in the program. They are similar to constants defined in `.data`, but they will be prefixed with `$` and can be modified in the program. The syntax is `<name> [addr[,<value>]]`, where `<name>` is the name of the variable, `[addr]` is the address of the variable (optional), and `[value]` is the default value of the variable (optional). If the address is omitted, the address will be incremented from the last one (1 if this is the first). The `rbx` variable is automatically defined as `$0`.

## `.comment`
This section holds comments that will not be compiled into the program.

## `.text`
This section is where all of the main code goes. This section must be present as the last section of the program. The syntax is `<inst> <arg1>[,<arg2>...]`, where `<inst>` is the instruction and `<arg*>` is the argument. Any argument can be prefixed with a `$`, which indicates to get the value from that position in memory. The section (and program) ends with the `ret` instruction at the end. After `ret`, assembly stops and text placed after it will not be checked. If the default spec is not `8S1A`, you can place `ret` instructions inside if statements.

For the list of instructions, go to https://mcjack123.github.io/8S1A/#instruction-set. The `CPU Name` column gives the assembly name.  
  
## Inline comments
You can write inline comments that will not be compiled into the program. These start with a `;` and continue until the end of the line.
