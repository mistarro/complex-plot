# Complex Plot

Simple tool for visualizing complex algebraic functions using domain coloring.

## Build and run

Before building, make sure you have the following installed:

  1. C++ compiler with C++17 support (tested with Clang 6 and GCC 7.5.0)
  1. Qt 5 (user interface)
  1. LLVM 10 (LLJIT for function code generation)

Complex Plot uses CMake, to build:
```sh
$ mkdir build && cd build
$ cmake ..
$ make
```

This creates `complex-plot` binary.

## Usage

Algebraic function is specified as implicit function by providing polynomial in two variables:
  * `z` (argument) and
  * `w` (value).

For example `z*w^2 - 1` defines inverse of the square root of `z`, i.e.,

![formula](https://latex.codecogs.com/svg.latex?w%28z%29%20%3D%20%5Cfrac%7B1%7D%7B%5Csqrt%7Bz%7D%7D.)


Formal grammar for input formula is
```
<formula> ::= ["-"] <summand> (("-"|"+") <summand>)*
<summand> ::= <factor> ("*" <factor>)*
<factor>  ::= <base> ["^" <integer>]
<base>    ::= "(" <formula> ")" | "z" | "w" | "i" | <number>
<number>  ::= <digit>+ ["." <digit>* ["e" ["+"|"-"] <digit>+]]
<integer> ::= <digit>+
<digit>   ::= "0" | ... | "9"
```

By specifying seed and its value different branches of an algebraic function can be plotted.
Plotting algorithm proceeds from the seed pixel in BFS manner and uses seed value as a starting point to compute function's value.

To see how complex plane is colored, plot identity function (that is, use `w - z` as the function's formula).

