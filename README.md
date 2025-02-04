IVM (Inline Virtual Machine)
====

The *inline virtual machine* (IVM) allows you to write virtual instructions as structured, human-readable bytecode directly in C++ code, which is assembled at compile time. This eliminates the need for an external assembler. The VM processes the instructions sequentially or based on control flow (e.g., jumps). Each instruction corresponds to a custom operation, such as arithmetic, comparison, or branching.  Designed with a small form factor, the IVM is lightweight and easily embeddable, making it ideal for streamlined and efficient integration into C++ applications.


Language Overview
-----------------

### Registers, Loads, and Stores

IVM comes with 8 registers `R0`, `R1`, `R2`, `R3`, `R4`, `R5`, `R6`, `SP`. Registers can be used in the following formats:

* Instruction(Register, Register)
* Instruction(Register), Immediate value
```
{
    MOV(R0), 123,     	      // store immediate value '123' into R0
    MOV(R1, R0),     	      // load the value in R0 into R1
    LEA(R2, R1),              // load the address of R1 into R2
    MOV<int, int*>(R1, R2),   // load the byte from the memory location pointed to by R2 into R1
    MOV<int*, int>(R2, R0)    // store the byte in R0 into the memory location pointed to by R2
}
```
Note: `SP` is reserved for the stack pointer.

### Stack, Stack pointer

```
{
    PUSH(), 123,     	      // push the immediate value '123' onto the top of the stack
    MOV(R2), 456,             // store the immediate value '456' into R2 
    PUSH(R2),                 // push R2 onto the top of the stack
    SUB(SP), 8,               // subtract 8 from the stack pointer
    MOV<int*, int>(SP), 789,  // store the immediate value '789' into the SP memory location
    XOR(R2, R2),            
    XOR(R3, R3),
    MOV<int, int*>(R1, SP),   // load the value from the SP memory location into R1
    ADD(SP), 8,               // add 8 to the stack pointer
    POP(R2),                  // pop the value off the top of the stack into R2
    POP(R3)                   // pop the value off the top of the stack into R3
}
```

### Jumps, Loops

```
{
    MOV(R0), 4,               // store the immediate value '4' into R0
    JMP(R0),                  // jump to the location in R0
    RET(),
    ADD(R1), 1,               // add the immediate value '1' to R1
    CMP(R1), 100,             // compare R1 to the immediate value '100'
    JNE(), 0                  // jump to the start if R0 is not equal to 100
}
```
Note: Jumps use the program counter rather than a memory address.

### Calls, Native operations

```
{
    PUSH(), "Enter a string: ",   
    CALL(), printf,            
    ADD(SP), 8,
    SUB(SP), 100,             // allocate 100 bytes on the stack
    MOV(R0, SP),              // store the stack pointer
    PUSH(), stdin,            // push the file pointer
    PUSH(), 100,              // push the size of the buffer
    PUSH(R0),                 // push the stored buffer pointer
    CALL(), fgets,            // call fgets
    ADD(SP), 24,              // clean up stack (3 * 8 bytes)
    PUSH(R0),
    PUSH(), "You entered: %s",
    CALL(), printf,
    ADD(SP), 116,             // clean up stack (2 * 8 + 100 bytes)
}
```
Note: The call stack is 8-byte aligned and can hold 10 arguments.

Instruction Set Overview
-----------------

| **Instruction** | **Opcode** | **Description**                                                                          |
|-----------------|------------|------------------------------------------------------------------------------------------|
| `MOV`           | `1`        | Move a value into a register.						                                      |
| `ADD`           | `2`        | Add two values and store the result in a register.                                       |
| `SUB`           | `3`        | Subtract one value from another and store the result in a register.                      |
| `AND`           | `4`        | Perform a bitwise AND operation on two values and store the result.                      |
| `XOR`           | `5`        | Perform a bitwise XOR operation on two values and store the result.                      |
| `LEA`           | `6`        | Load the effective address of an operand into a register.                                |
| `CMP`           | `7`        | Compare two values, set the zero flag if equal, or clear it if not.                      |
| `JMP`           | `8`        | Jump to a specified location unconditionally.                                            |
| `JNE`           | `9`        | Jump to a specified location if the compared values are not equal (ZF = 0).              |
| `CALL`          | `10`       | Call a native address outside the virtual machine environment. 				          |
| `PUSH`          | `11`       | Push a value onto the top of the stack. 			                                      |
| `POP`           | `12`       | Pop the top value off the stack and store it in a register. 			                  |
| `RET`           | `13`       | Exit program.			     						                                      |

Requirements
-----------------

* **C++14** or newer *(e.g., GCC 5.0+, Clang 3.4+, MSVC 2015+)*
* **Optimization** (`-O1` and Above) for best results.

License
-----------------

IVM is licensed under the Apache License, Version 2.0