# AlpShell

AlpShell is a lightweight, fast, and extensible custom-built shell developed by students from IIT Jodhpur. Designed from scratch in C++, AlpShell aims to eliminate system bloat and improve shell performance without compromising essential functionality or user experience.


## Project Overview

AlpShell functions as a modern, minimalistic shell for Unix-like systems. It features a clean command-line interface that supports command execution, job control, file management, scripting, and extensibility — all with enhanced responsiveness and reduced system overhead.


## Directory Structure

```
├── bootloader/           # x86 bootloader (for future system-level integration)
├── shell/                # Core shell implementation
│   ├── src/              # Source files
│   ├── include/          # Header files
│   ├── main.cpp          # Entry point
│   └── Makefile/CMake    # Build scripts
├── parser_test/          # Lexer & parser testbed
├── userland/             # Placeholder for userland programs
└── README.md             # This file
```


## Features

### Core Functionalities
- Built-in command execution with argument parsing
- Pipelining (`|`), input/output redirection (`<`, `>`, `>>`)
- Command history and aliasing
- Foreground/background process handling (`&`, `kill`, `jobs`)

### File & Directory Management
- Support for `cd`, `ls`, `mkdir`, `rm`, etc.
- Directory stack (`pushd`, `popd`, `dirs`)
- Tilde (`~`) and shorthand directory navigation

### Scripting & Automation
- Shell script execution
- Support for variables, conditionals, loops
- User-defined functions (planned)

### User Experience
- Syntax highlighting and auto-suggestions
- Colored output for better readability
- Customizable shell prompt
- Error/output separation with formatting


## Architecture Highlights
- Modular design for maintainability
- Tokenizer and parser for command interpretation
- Readline integration for user input and history
- Lightweight and portable — minimal dependencies


## Build & Run Instructions

### Prerequisites
- g++ (C++11 or higher)
- CMake (v3.10+)
- readline development library

### Build
```bash
cd shell
mkdir build && cd build
cmake ..
make
```

## Authors
Aurindum Banerjee (B23CS1006) 
Lokesh Motwani (B23CS1033) 
Laksh Mendpara (B23CS1037) 
Aayush Bade (B23CS1084) 
Prashasta Srivastava (B23CS1096)

## References
[IBM AIX Shell Features](https://www.ibm.com/docs/en/aix/7.2?topic=concepts-shell-features)

[Fish Shell](https://www.google.com/search?client=firefox-b-d&q=main+feature+of+fish+shell)

[Why Zsh? – Joe Jag](https://code.joejag.com/2014/why-zsh.html#:~:text=Not%20only%20does%20it%20help,Zsh%20a%20joy%20to%20use.)

[O’Reilly – Learning the Bash Shell](https://www.oreilly.com/library/view/learning-the-bash/1565923472/pr01s02.html)
