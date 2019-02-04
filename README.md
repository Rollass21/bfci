# bfci
BrainFuck C Interpreter

# Why?
Recently I found out about this excellent, magnificent and most of all easy to understand programming language called [Brainfuck](https://en.wikipedia.org/wiki/Brainfuck).
As I observed other people trying to create the smallest and the most efficient interpreters, a bright idea came to my mind.
I, on the other hand, figured that trying to over-engineer, read as "italian pasta-ify", the heck out of something like this could help me learn something new and would be much more fun than just a little cute fewliner!.

TLDR: Learning purposes.

Keep in mind this is just a little hobby project and is not intended to be used as serious development/debugging tool ( I feel sorry for everyone who is in need for a profesional grade BF interpreter ).
Every kind of helpful feedback will be greatly appreciated.

:)

---

# Files
## *.c files
### [main.c](main.c)
- Actual implementation of BrainFuck executable interpreter.
### [bfci.c](bfci.c)
- Main bfci engine functions taking care of execution. Function `interpret()` is a go to if you want to know how it all works!
### [insset.c](insset.c)
- Contains BrainFuck's instruction set with respective functions.
### [inssetf.c](inssetf.c)
- Helper functions for [instruction set](insset.c). 
### [presets.c](presets.c)
- Contains custom preset settings to configure contexts.

## *.h files
### [bfci.h](bfci.h)
- Main bfci engine header file, containing macros, data type defs API functions.
### [tcolors.h](tcolors.h)
- Contains terminal color definitions.

## *.md files
### [ideas.md](ideas.md)
- Contains ideas for future additions to bfci, can be considered a lazy version of a TODO list.


---

# How to run
1. Compile with
`make`
1. Execution
- Execute a BF source file: (you can find some test BF files in [testsrcs/](testsrcs/))
`./bfci -t <file destination>`
- Execute a string containing BF source:
`./bfci -i '<string>'`

You can always view help prompt by typing `./bfci -h`.
