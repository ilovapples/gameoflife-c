It's just Conway's Game of Life, but in C. 

I did try my best to make it cross-platform (that is, functional on Windows and POSIX-compliant OS's), but from my minimal benchmarking, it takes 10-100x longer to render each frame on the console on the Windows build, so be wary of that.

The only command line argument you need to be aware of is 'COLSxROWS'. You just input that as the first argument to change the size of the grid. The COLS and ROWS fields are both rounded up to the nearest multiple of 8 for technical reasons that I can't be bothered to fix otherwise (inspect the source code yourself if you'd like). For example, if I wanted a grid with 48 columns and 48 rows, I could run: 
```sh
$ ./life 48x48
```

# -- BUILD --
Build with nob.h:
```sh
$ gcc -o nob nob.c
$ ./nob
```
GCC figures out what platform to compile for automatically, and the executable will pop out in the `build/` folder.
