# Start C++ Programming by Implementing some Ideas about Solving a Sudoku


Build (tested with gcc 4.8.5 on CentOS 7 and mingw on Windows 7):

   ```g++ -std=c++11 -o sudoku sudoku.cpp```

You may also use the makefile:

   ```make```

... but it actually is bound to follwing environment:

   ```Linux CentOS 7 with devtoolset-4 installed```

Note: Default version of CentOS 7 is "g++ (GCC) 4.8.5"/C++11 where C++14 is not fully implemented, devtoolset-4 makes version "g++ (GCC) 5.3.1"/C++14 available in parallel. But actually there is no C++14 feature used in this project.


usage: ```./sudoku < file.sud```

Where file.sud stands for a filename of a file containing numbers for initialization of a Sudoku.

There are some samples in directory templates, e.g.: templates/x_01.sud.

The format of data in this file is: 

  - any number of bytes, 
  
  - only considering ASCII characters '1'..'9' as a number to initialize the corresponding cell to,
  
  - any other byte value is considered as empty (not yet solved) cell placeholder,
  
  - the byte position in the file corresponds to the cell position in the 9x9 Sudoku grid:
  
    ```
    byte  1 -> line 1, col 1
    
    byte  2 -> line 1, col 2
    
    [...]
    
    byte 10 -> line 2, col 1
    
    [...]
    
    byte 80 -> line 9, col 9
    ```
    
    - or as algorithm (zero based indices and fileposition):
  
    ```cell(row = int(filepos/9), col = filepos modulo 9)```
    
    - or prosaic:
  
    ```The position of one character in the file correlates to the position in Sudoku matrix by following rule:
    Starting at top left position of 9x9 Sudoku matrix, filling a row with the next characters
    starting with first char of file proceeding with next row of matrix and so on.```

The output is pure ASCII-art on console, with luck a solved Sudoku (all blanks replaced by numbers).

Have fun

Stefan

11. November 2017
