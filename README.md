Start C++ Programming by Implementing some Ideas about Solving a Sudoku

At this state compiling by make is for Linux only, see makefile.

usage: ./sudoku < file.sud

Where file.sud stands for a filename of a file containing 81 bytes for initialization of a Sudoku, e.g.: templates/schwer_01.sud.
The format of data in this file is: 81 ASCII characters '1'..'9' or ' ' (blank). 
The position of one character in the file correlates to the position in Sudoku matrix by following rule:
Starting at top left position of Sudoku matrix, filling a row with the next characters starting with first char of file, proceeding with next row of matrix and so on.

The output is pure ASCII-art on console, with luck a solved Sudoku (all blanks replaced by numbers).

Have fun
Stefan
11. November 2017
