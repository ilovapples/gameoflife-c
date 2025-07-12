It's just Conway's Game of Life, but in C. 

I did try my best to make it cross-platform (that is, functional on Windows and POSIX-compliant OS's), but from my minimal benchmarking, it takes 10-100x longer to render each frame on the console on the Windows build, so be wary of that.

The only command line argument you need to be aware of is 'COLSxROWS'. You just input that as the first argument to change the size of the grid. The COLS and ROWS fields are both rounded up to the nearest multiple of 8 for technical reasons that I can't be bothered to fix otherwise (inspect the source code yourself if you'd like). For example, if I wanted a grid with 48 columns and 48 rows, I could run: 
```sh
$ ./life 48x48
```

# -- BUILD --
Build with make:
```sh
$ make
```
The Makefile will build both `cst` (or `cst.exe` on Windows) (see below) and `life` (or `life.exe` on Windows) and the executables will pop out in the `build/` folder.

# -- CST --
`cst` is a tool that converts files containing a textual representation of an initial grid state (and dimensions) to a binary file containing the same information.
So far, this tool is effectively useless, since the viewer (`life`) can't actually read the files into memory yet. That'll probably be the next commit, whenever I feel like writing it.
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
