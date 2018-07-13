# 8S1A CPU
![CPU](https://github.com/MCJack123/8S1A/raw/gh-pages/2018-07-13_00.26.11.png)
The 8S1A is an 8-bit microprocessor for Minecraft. It includes 128 bytes of memory built-in, and includes 9 instructions. 

# IMPORTANT
This world was made in 1.13. Since 1.13 adds a new world layout system, it is impossible to load this world in 1.12 or below. This also means that you cannot copy the CPU into a pre-existing world without copying in the region file manually. **Do NOT try to load the world in 1.12, or you WILL experience data loss.** (You can always download a new copy of the CPU, though.)

# Interface
![CPU pins](https://mcjack123.github.io/8S1A/pinout.png)  
  
Going from left to right, the pins are:  

Instruction | Instruction Done | Output Bits | Input Bits | Clock In | Clock Out | Memory Clear |  
-----------:|:----------------:|:-----------:|:----------:|:--------:|:---------:|:------------:|  
**Type**    | Pulse            | Steady      | Steady     | Pulse    | Pulse     | Pulse        |  
**Input/Output**| Output           | Output      | Input      | Input    | Output    | Input        |  
**Symbol**  | `IND`            | `OUT`       | `INP`      | `CKI`    | `CKO`     | `MAC`        |

Each pulse is expected to be at least 4 ticks long.

# How to Use
First, make sure to connect `INP`, `OUT`, `CKI`, and `CKO` to wherever they are going to, such as levers and lamps. (A good way to send parallel lines for `INP` and `OUT` is to alternate {block, redstone, block, repeater}. This will keep the delay at 1 tick/4 blocks and doesn't cause any interference.) Make sure that the `CKI` wire has the same delay as the input, or you may end up sending a clock before the input is ready.  
Next, send the input instructions over `INP`. Once the required number is going through the input, send a pulse of 4 ticks to `CKI`. Maintain the signal over `INP` until a pulse is sent over `CKO`. Once `CKO` pulses, the CPU is ready to accept another byte of data.  
When the CPU is done executing a full instruction, a pulse will be sent over `IND`. This is not necessary to use the CPU, but it can be helpful when creating instruction extensions for the CPU. (More on that later.) This signal indicates that the CPU is ready to accept another instruction.
If you want to reset the memory of the CPU, send a pulse over `MAC`. This will clear the entire 128 bytes of memory, as well as `OUT` and the result buffer. Note that no pulse will be sent over `CKO` when done, but if `OUT` is not zero it will change. You can use that as an unreliable way to detect when it is done if needed. But to be safe, wait at least 30 ticks to send more input.  

# Instruction Set

| Value | CPU Name | Full Name   | Arguments   | Description      | Caveats |
| ----- | -------- | ----------- | ----------- | ---------------- | ------- |
| 0x00  | `ADD`    | Addition    | `<a1> <a2>` | Adds a1 and a2 together and stores the output in the result buffer (`0x00`). | None |
| 0x01  | `SUB`    | Subtraction | `<m> <s>`   | Subtracts s from m and stores the output in the result buffer (`0x00`). | None |
| 0x02  | `SET`    | Memory Set  | `<addr> <val>` | Sets the memory address at addr to val. | You cannot set address `0x00` (result buffer) because this is the default address, and buggy things may happen. Also, you cannot copy values directly in memory (except for address `0x00`), but you can replicate it by adding 0 to the value and setting from the result buffer: `ADD 0x7E <addr1> SET <addr2> 0x7E 0x00`. |
| 0x03  | `IEQ`    | If Equal    | `<n1> <n2> <instr...> 0x7D` | If `n1` and `n2` are equal, anything in `instr` is executed. If not, does nothing until it recieves `0x7D`. | Make sure to pass `0x7D` at the end of the if unless you are 100% sure that `n1` == `n2`. (But in that case, why are you using if at all?) |
| 0x04  | `INE`    | If Not Equal| `<n1> <n2> <instr...> 0x7D` | If `n1` and `n2` are not equal, anything in `instr` is executed. If not, waits until it gets `0x7D`. | Same as `IEQ` |
| 0x05  | `IGT`    | If Greater Than | `<n1> <n2> <instr...> 0x7D` | If `n1` is greater than `n2`, anything in `instr` is executed. If not, waits until it gets `0x7D`. | Same as `IEQ` |
| 0x06  | `ILT`    | If Less Than| `<n1> <n2> <instr...> 0x7D` | If `n1` is less than `n2`, anything in `instr` is executed. If not, waits until it gets `0x7D`. | Same as `IEQ` |
| 0x07  | `OUT`    | Output      | `<num>`     | Sends `num` to the output. | When reading from memory, there is a very precise timing on the output, so it may not work on slower (real) computers. |
| 0x7E  | `$`*     | Memory Read | `<addr>`    | This instruction can be used anywhere in a program. It will replace whatever argument it is in the place of with the value of `addr`. | Cannot be used in conjunction with `SET` because both are accessing the memory unless `0x00` is being read. |

\* Symbol used in assembly  

# For experts
## Extending Instructions
You can easily extend the instruction set of the CPU. By adding a circuit to detect new instructions, you can easily extend the functionality of the CPU. I am planning to release a memory extension unit which will add a 16-bit address bus and four new commands allowing the CPU to run instructions from memory.  
Here's an outline of how you can create your own instructions:
1. Create an XOR equality checker inline with the input to check if the number is the same.  
2. Attach the XOR to an RS-NOR latch to only look for instructions when the CPU is waiting for instructions.  
3. Use the `IND` wire to turn off the RS-NOR, and turn on when an instruction is sent.  
4. Set up a way to allow the signal to go to the instruction only when the instruction is selected, and disable this when it is not.  
You can look at the main selection circuitry inside the CPU to see how it is done inside.

## Writing assembly code
I have written an assembler for the 8S1A (technically, it's for the memory expansion, but it still works) in the `assembler` branch of this repository. I have included build instructions there to be able to write programs.  

## ROM chip
I will be writing instructions on how to build your own ROM from assembled code soon. The ROM will be coming before the memory unit.  

# Distribution
You are allowed to freely copy and redistribute this processor as long as you do not try to say that it's your creation. Honestly, I have no way to really enforce any licensing policy, but it's the good thing to do. :)
