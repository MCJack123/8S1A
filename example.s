The assembler will ignore anything
that is put before the first section.
Note the data and bss sections must
be put before the text section.

section .data     ; named constants
    true 84       ; replace all instances of true with 84 (T)
    false 70      ; replace all instances of false with 70 (F)
section .bss      ; named variables in memory
    num 1,2       ; replace all instances of num with $1 and set it to 2
section .text     ; code
    add num,4     ; add num & 4
    ieq $0,6      ; if value 0 == 6
        out true  ; send value of true to the output
    end           ; end the if
    ine $0,6      ; if value 0 != 6
        out false ; send value of false to the output
    end           ; end the if
    ret           ; stop the program

The assembler will also ignore
anything after the ret instruction.
