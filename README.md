It's just Conway's Game of Life, but in C. 

I did try my best to make it cross-platform (that is, functional on Windows and POSIX-compliant OS's), but from my minimal benchmarking, it takes 10-100x longer to render each frame on the console on the Windows build, so be wary of that.

Command-line arguments for `life` should (hopefully) be self-explanatory from the help message displayed by the `--help` option.

# -- BUILD --
Build with make:
```sh
$ make
```
The Makefile will build both `cst` (or `cst.exe` on Windows) (see below) and `life` (or `life.exe` on Windows) and the executables will pop out in the `build/` folder.

# -- CST --
`cst` is a tool that converts files containing a textual representation of an initial grid state (and dimensions) to a binary file containing the same information.
~~So far, this tool is effectively useless, since the viewer (`life`) can't actually read the files into memory yet. That'll probably be the next commit, whenever I feel like writing it.~~ Update: functionality for reading binary files was added in the latest release.
`states/s0.txt` contains an example of the format for the input to `cst`. Here's a minimal example (the more astute among those reading this will recognize the 'glider' pattern):
```
8x8
------#-
-----#--
-----###
--------
--------
--------
--------
--------
```

The dead cells must be specified by a `-` character, while alive cells must be specified by a `#` character. The character between the two dimensions (col x row) may be `x` or `' '` (a space character, `'\x20'` in ASCII).
