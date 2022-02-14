# Lisp Interpreter
This is a basic lisp interpreter written in C

## Install the compiler
`sudo apt-get install build-essential`

## Install readline and add_history
`sudo apt-get install libedit-dev`

## Install readline for debian and ubuntu
`sudo apt-get install libreadline-dev` 
`sudo apt install libeditline-dev`

## hello_world.c
Prints 'Hello World' to the console

### Compile
`cc -std=c00 -Wall hello_world.c -o hello_world`

## prompt.c
A basic prompt that repeats the user input back to the console

### Compile
`cc -std=c99 -Wall prompt.c -ledit -o prompt`

## parsing.c
Added support for parsing the user input so later we can do stuff with it

### Compile on Linux and Mac
`cc -std=c99 -Wall parsing.c mpc.c -ledit -lm -o parsing`

### Compile on Windows
`cc -std=c99 -Wall parsing.c mpc.c -o parsing`

#### FYI
I developed this using Ubuntu Linux, so the following examples will now only be for compiling on Ubuntu or Debian. If you are using Windows, simply omit the `-ledit -lm` flag from the compiler options from this point and  on.

## evaluation.c
Evaluates the user input using basic math functions

### Compile
`cc -std=c99 -Wall evaluation.c mpc.c -ledit -lm -o evaluation`

### Available Functions
* \+ 
* \- 
* \*
* /
* %
* ^
* add
* sub
* div
* mul
* rem
* pow
* min
* max
* leaves
* branches

#### Examples
```
lispy> + 1 1
2
lispy> - 2 1
1
lispy> * 2 2
4
lispy> / 4 2
2
lispy> % 10 4
2
lispy> ^ 3 2
9
lispy> add 1 2
3
lispy> sub 3 1
2
lispy> mul 2 3
6
lispy> div 10 5
2
lispy> rem 7 4
3
lispy> pow 10 10
100
lispy> min 7 18 4 32
4
lispy> max 7 18 4 32
32
```

## error_handling.c
Added some basic error handling. Dividing by zero no longer crashes the interpreter, instead the user receives an error. User is also notified if they use an invalid operator or invalid number.

### Compile
`cc -std=c99 -Wall error_handling.c mpc.c -ledit -lm -o error_handling`

### Functions
Same as evaluation.c

## s_expressions.c
Added support for S-Expressions. S-Expressions create support for nested operations

### Compile
`cc -std=c99 -Wall s_expressions.c mpc.c -ledit -lm -o s_expressions`

### Functions
Same as error_handling.c

### Example of S-Expressions
```
lispy> (+ 1 1)
2
```

## q_expressions.c
Added support for Q-Expressions, which are similar to lists. Also added functions for manipulating lists

### Compile
`cc -std=c99 -Wall q_expressions.c mpc.c -ledit -lm -o q_expressions`

### Functions
Same as s_expressions.c
Additionaly:
* cons
* len
* init
* head
* tail
* join
* list
* eval
* join

#### cons
takes a value and a Q-Expression and appends it to the front
```
lispy> cons 1 {2 3}
{1 2 3}
```

#### len
returns the number of elements in a Q-Expression
```
lispy> len {1 2 3}
3
```

#### init
returns all of a Q-Expression except the final element
```
lispy> init {1 2 3 4}
{1 2 3}
```

#### list
```
lispy> list 1 2 3 4
{1 2 3 4}
```

#### head
Returns the first element of a list
```
lispy> head (list 1 2 3 4)
{1}
```

#### tail
Returns the last element of a list
```
lispy> tail {1 2 3 4}
{4}
```
#### join
Conjoins one or more Q-Expressions and returns a Q-Expression

#### eval
Evaluates the expression as if it were an S-Expression
```
lispy> eval (tail {tail tail {5 6 7}})
{6 7}
```

## variables.c
Added support for creating variables using def. Also added an exit function to exit.

### Compile
`cc -std=c99 -Wall variables.c mpc.c -ledit -lm -o variables`

### Examples
```
lispy> def {x} 100
()
lispy> x
100
lispy> exit
Exiting...
```
