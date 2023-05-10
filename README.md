# chip-8

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT) ![C++ Solution](https://img.shields.io/badge/C++-Solutions-blue.svg?style=flat&logo=c%2B%2B)

A Chip-8 emulator written in modern C++.

[Chip-8](https://en.wikipedia.org/wiki/CHIP-8) is a simple, interpreted, programming language which was first used on some do-it-yourself computer systems in the late 1970s and early 1980s.

## Motivation

This is a project to write a Chip-8 emulator in modern C++. The main motivation is to learn lower level programming concepts.

Here are some of the concepts I learned while writing this program:

- The base system: specifically base 2 (binary), base 10 (decimal), base 16 (hexadecimal), how they interact with each other and the concept of abstract numbers in programming
- Bits, nibbles, bytes, ASCII encoding, and big and little endian values
- Bitwise operators - AND (`&`), OR (`|`), XOR (`^`), left shift (`<<`), right shift (`>>`) and how to use them for masking, setting, and testing values
- The concept of a raw data buffer and how to work with it, how to convert an 8-bit buffer to a 16-bit big endian array
- How a CPU can utilize memory, stack, program counters, stack pointers, memory addresses, and registers
- How a CPU implements fetch, decode, and execute
