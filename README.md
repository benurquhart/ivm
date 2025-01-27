IVM (Inline Virtual Machine)
====

The *inline virtual machine* (IVM) allows you to write virtual instructions as structured, human-readable bytecode directly in C++ code, which is assembled at compile time. This eliminates the need for an external assembler. The VM processes the instructions sequentially or based on control flow (e.g., jumps). Each instruction corresponds to a custom operation, such as arithmetic, comparison, or branching.  Designed with a small form factor, the IVM is lightweight and easily embeddable, making it ideal for streamlined and efficient integration into C++ applications.


Language Overview
-----------------

### Registers, Loads, and Stores

IVM comes with 8 registers **R0**, **R1**, **R2**, **R3**, **R4**, **R5**, **R6**, **SP**. Registers can be used in the following formats:

* Instruction(Register, Register)
* Instruction(Register), Immediate value
```
{
    MOV(R0), 123,     	    // store immediate value '123' into R0
    MOV(R1, R0),     	    // load the value in R0 into R1
    LEA(R2, R1),            // load the address of R1 into R2
    MOV<uint8_t>(R1, R2),   // load the byte from the memory location pointed to by R2 into R1
    MOV<uint8_t*>(R2, R0)   // store the byte in R0 into the memory location pointed to by R2
}
```
Note: **SP** is reserved for the stack pointer.

### Stack, Stack pointer

```
{
    PUSH(), 123,     	    // push the immediate value '123' onto the top of the stack
    MOV(R2), 456,           // store the immediate value '456' into R2 
    PUSH(R2),               // push R2 onto the top of the stack
    SUB(SP), 8,             // subtract 8 from the stack pointer
    MOV<int*>(SP), 789,     // store the immediate value '789' into the SP memory location
    XOR(R2, R2),            
    XOR(R3, R3),
    MOV<int>(R1, SP),       // load the value from the SP memory location into R1
    ADD(SP), 8,             // add 8 to the stack pointer
    POP(R2),                // pop the value off the top of the stack into R2
    POP(R3)                 // pop the value off the top of the stack into R3
}
```

### Jumps, Loops

```
{
    MOV(R0), 4,             // store the immediate value '789' into R0
    JMP(R0),                // jump to the location in R0
    RET(),
    ADD(R1), 1,             // add the immediate value '1' to R1
    CMP(R1), 100,           // compare R1 to the immediate value '100'
    JNE(), 0                // jump to the start if R0 is not equal to 100
}
```
Note: Jumps use the program counter rather than a memory address.

### Instruction Set Overview

| **Instruction** | **Opcode** | **Description**                                      |
|-----------------|------------|------------------------------------------------------|
| `MOV`           | `1`        | Move a value into a register.						  |
| `ADD`           | `2`        | Add two values and store the result.                 |
| `SUB`           | `3`        | Subtract one value from another.                     |
| `AND`           | `4`        | Perform bitwise AND on two values.                   |
| `XOR`           | `5`        | Perform bitwise XOR on two values.                   |
| `LEA`           | `6`        | Load the effective address of an operand.            |
| `CMP`           | `7`        | Compare two values and set zero flag.                |
| `JMP`           | `8`        | Jump to a location unconditionally.                  |
| `JNE`           | `9`        | Jump to a location if the values are not equal.      |
| `CALL`          | `10`       | Call a native address outside the vm. 				  |
| `PUSH`          | `11`       | Push a value onto the top of the stack. 			  |
| `POP`           | `12`       | Pop a value off the top of the stack.  			  |
| `RET`           | `13`       | Exit program.			     						  |

### Requirements

* **C++14** or newer *(e.g., GCC 5.0+, Clang 3.4+, MSVC 2015+)*