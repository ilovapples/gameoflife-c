It's just Conway's Game of Life, but in C. 

I did try my best to make it cross-platform (that is, functional on Windows and POSIX-compliant OS's), but from my minimal benchmarking, it takes 10-100x longer to render each frame on the console on the Windows build, so be wary of that.


# -- BUILD --
Build with nob.h:
```sh
$ gcc -o nob nob.c
$ ./nob
```
GCC figures out what platform to compile for automatically, and the executable will pop out in the `build/` folder.
