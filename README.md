# 1bitvm.c - 1bitvm implemetation in C
This is an implemetation of the 1bit processor as described by [adimineman](https://github.com/adimineman).
Being that it's written in C, it performes about 232x better, and because of the lower level and lower overhead it provides better compatability with software written for the 1bit architecture.



## Installation
### Dependacies
This software only depens on the GNU C Library, GCC and GNU Make
### Building
    make

## Usage 
    ./1bitvm infile
### Options
All described options are settable in `config.h`
 - `DEBUG` - Debugging is availiable through four debug levels
 - `LIMIT` - Maximum number of executed instructions, -1 to disable
 - `EXIT_ON_EOF` - If set, the vm exits imediately after EOF is input, if not set, the vm is allowed to continue - IN will always be 1
 - `STDIN_PROMPT` - if set, the vm will prompt the user with the character `>`, if not set, the character will not be shown
 - `STDIN_PROMPT_STREAM` - sets the stream to which the prompt character may be printed

### Return values
    0 - Exited by PC non increment - graceful
    1 - Execution limit reached
    2 - EOF on stdin
## Example files
get example files [here](https://github.com/adimineman/1bitvm)
### Compilation
    ./asm.py *.asm

## License
    1bitvm.c - C Implementation of the 1bit architecture
    Copyright (C) 2023 Oliver Wagner

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
