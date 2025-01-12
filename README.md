IVM (Inline Virtual Machine)
====

The *inline virtual machine* (IVM) allows you to write virtual instructions as structured, human-readable bytecode directly in C++ code. The VM processes the instructions sequentially or based on control flow (e.g., jumps). Each instruction corresponds to a custom operation, such as arithmetic, comparison, or branching.  Designed with a small form factor, the IVM is lightweight and easily embeddable, making it ideal for streamlined and efficient integration into C++ applications.


Language Overview
-----------------

### Registers, Immediate Values

IVM comes with 6 registers **R0**, **R1**, **R2**, **R3**, **R4**, **R5**. Registers can be used in the following formats:

* Instruction(Register, Register)
* Instruction(Register), Immediate value
```
{
	MOV(R0), 123     		// store immediate value '123' into R0
	MOV(R1, R0)     		// load the value in R0 into R1
	MOV<uint8_t>(R1, R0),		// load the byte from the memory location pointed to by R0 into R1
	MOV<uint8_t*>(R1, R0),		// store the byte in R0 into the memory location pointed to by R1
}
```

### Instruction Set Overview

| **Instruction** | **Opcode** | **Description**                                       |
|------------------|------------|------------------------------------------------------|
| `MOV`           | `1`        | Move a value into a register.						   |
| `ADD`           | `2`        | Add two values and store the result.                  |
| `SUB`           | `3`        | Subtract one value from another.                      |
| `AND`           | `4`        | Perform bitwise AND on two values.                    |
| `XOR`           | `5`        | Perform bitwise XOR on two values.                    |
| `LEA`           | `6`        | Load the effective address of an operand.             |
| `CMP`           | `7`        | Compare two values and set flags.                     |
| `JNE`           | `8`        | Jump to a location if the values are not equal.       |
| `CNA`           | `9`        | Call a native address.      						   |