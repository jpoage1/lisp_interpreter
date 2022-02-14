# Lisp Interpreter
This is a basic lisp interpreter written in C

## Install the compiler
sudo apt-get install build-essential

## Install readline and add_history
sudo apt-get install libedit-dev

## Install readline for debian and ubuntu
sudo apt-get install libreadline-dev 
sudo apt install libeditline-dev 

## Compile hello_world.c
cc -std=c00 -Wall hello_world.c -o hello_world

## Compile prompt.c
cc -std=c99 -Wall prompt.c -ledit -o prompt

## Compile parsing.c on Linux and Mac
cc -std=c99 -Wall parsing.c mpc.c -ledit -lm -o parsing
## Compile parsing.c on Windows
cc -std=c99 -Wall parsing.c mpc.c -o parsing

##Compile evaluation.c
cc -std=c99 -Wall evaluation.c mpc.c -ledit -lm -o evaluation

Has functions: + - * / % ^ add sub div mul rem pow min max leaves branches

## Compile error_handling.c
cc -std=c99 -Wall error_handling.c mpc.c -ledit -lm -o error_handling

## Compile s_expressions.c
cc -std=c99 -Wall s_expressions.c mpc.c -ledit -lm -o s_expressions

## Compile q_expressions.c
cc -std=c99 -Wall q_expressions.c mpc.c -ledit -lm -o q_expressions

## Compile variables.c
cc -std=c99 -Wall variables.c mpc.c -ledit -lm -o variables
