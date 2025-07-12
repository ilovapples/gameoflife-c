# TODO
[ ] First of all fix any bugs I've found so far (issues on GitHub)
[X] add a way to convert a text file to a binary that the program can read as a state of the cells (oh hey I did that)
[ ] Write logic in the viewer to read a binary created by the 'cst' tool and load it as cell state
[ ] Allow row and column values that aren't multiples of 2 or 8 (row ends might be in the middle of a byte, with the next row starting within the same byte)
[ ] Add a movable viewport to allow the grid to be absolutely gargantuan while keeping a small-ish viewport (obviously the whole grid will be updated every generation).
