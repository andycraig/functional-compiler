# Little Functional Language Compiler

[![CircleCI](https://circleci.com/gh/andycraig/functional-compiler/tree/master.svg?style=svg)](https://circleci.com/gh/andycraig/functional-compiler/tree/master)

A compiler for LFL ('Little Functional Language'), an original functional programming language. LFL has first-class functions and closures and supports recursion, and has Lisp-style syntax. The compiler is hand-written in C and Assembly.

I created LFL as a learning exercise, after wondering how anonymous functions might be implemented.

[Overview](#overview) | [Building](#building) | [Usage example](#usage-example) | [Feature showcase](#feature-showcase) | [Keywords](#keywords) | [Built-in functions](#built-in-functions) | [Limitations](#limitations) | [References](#references) | [Testing](#testing)

## Overview

The compiler takes in a file of LFL code, processes it and outputs the corresponding Assembly code. 

Here's an example of an LFL program, which creates an anonymous function that adds 1 to its argument, creates an alias `inc` for that function, and applies it to 1 to yield 2:

```
(let inc (λ x (plus x 1))
    (inc 1))
```

The compiler converts this to:

```
; Assembly code generated by compiler
	global main
	extern printf, malloc                ; C functions
	extern make_closure, call_closure    ; built-in functions
	extern plus, minus, equals           ; standard library functions

	section .text
_f0:
	push rbp
	mov rbp, rsp
	sub rsp, 40        ; memory for local variables
	mov rax, rdi       ; pointer to vector of arguments

... ET CETERA

	call printf
	mov rax, 0            ; exit code 0
	leave
	ret

	section .data
message: db "%d", 10, 0        ; 10 is newline, 0 is end-of-string
```

The steps the compiler goes through are:

1. **Tokenising:** The text file of LFL code is read in and converted into a list of symbols.
2. **Parsing:** An Abstract Syntax Tree (AST) of function calls, constants and variable names is generated from the list of symbols.
3. **Processing special forms:** Nodes in the AST with keywords (e.g.,  `λ`/`lambda`, `if`) are converted into special AST nodes.
4. **Processing lambdas:** The bodies of lambda expressions in the AST are pulled up to the global level and named, and the lambda expressions themselves are replaced with calls to a special function `make_closure`.
5. **Emitting Assembly code:** The AST is traversed, and at each node the corresponding Assembly code is written to the output file.

## Building

Build with CMake (requires NASM to build `libstandard.a`):

```
mkdir build
cd build
cmake ..
make # Creates binary 'bin/compile' and static libraries 'lib/libclosure.a', 'lib/libstandard.a'
```

## Usage example

Let's use [`examples/example.code`](examples/example.code). Compile it to Assembly with:

```
bin/compile examples/example.code example.asm
```

It produces a file `example.asm` of Assembly. To turn this Assembly code into something that can be run, it needs to be further processed with the NASM Assembly compiler and linked:

```
nasm -f elf64 example.asm
gcc -no-pie -o example example.o lib/libclosure.a lib/libstandard.a
```

Now we can run it:

```
./example # Should print '2'
```

## Feature showcase

Here's [an example program](examples/example_first_class.code) that shows closures and first-class functions in action:

```
(let make-adder 
    (λ x                       ; A function that returns a function
        (λ y (plus x y)))      ; Creates a closure over x
    (let add5 (make-adder 5)
        (add5 3)))             ; Prints '8'
```

[This program](examples/example_mult.code) uses recursion to do integer multiplication:

```
(defrec mult_aux                                       ; Use 'defrec' to define a recursive function
    (lambda x y acc
        (if 
            (equals x 1)
            acc
            (mult_aux (minus x 1) y (plus acc y)))))   ; Recursion happens here

(def mult                                              ; Wrapper for recursive function
    (lambda x y (mult_aux x y y)))          
    
(mult 3 4)                                             ; Prints '12'
```

## Keywords

### `λ` (aka `lambda`)

Syntax is `(λ ARG_1 ... ARG_N RESULT)`. Example:

```
(λ x y (plus x y)) ; Just adds x and y
```

### `if`

Syntax is `(if PREDICATE TRUE-CASE FALSE-CASE)`. Example:

```
(if (equals x 0) 0 (plus x 1)) ; Adds 1 to x unless it was 0
```

### `let`/`letrec`

Syntax is `(let ARG DEFINITION BODY)`. Example:

```
(let x 7
    (plus x 1)) ; Prints '8'
```

`letrec` must be used when creating a recursive function.

### `def`/`defrec`

Syntactic sugar for a `let/letrec` wrapped around the last expression in the file. Example:

```
(def double (λ x (plus x x )))

(double 2) ; Prints '4'
```

## Built-in functions

[`standard.asm`](src/standard.asm) defines the following functions:

- `plus` (e.g.: `(plus 3 2)`, which returns 5)
- `minus` (e.g.: `(minus 3 2)`, which returns 1)
- `equals` (e.g.: `(equals 3 3)`, which returns 1)

... and that's it.

## Limitations

- Functions can have up to four arguments, and up to three of these can be free variables captured by closures. (These free variables include other functions produced by `let`/`letrec`/`def`/`defrec`.) This restriction is because the compiler uses the fastcall calling convention, which requires using named registers for the first few arguments and the stack after that, and I didn't implement passing arguments using the stack.
- Integers (and functions) are the only data types. No floats, no strings, no lists ... You name it, it's not implemented.
- Register use is about as inefficient as it could be: registers other than `rax` are almost unused, except when passing arguments.
- No way of getting input from the user.
- Only form of output beyond the automatic printing of the last expression.
- Rampant memory leaks (both the compiler and the Assembly code it outputs).
- No run-time checking to verify that calls are made only on functions.

## References

I found these resources useful when learning how to implement closures:

- [Closure conversion: How to compile lambda](http://matt.might.net/articles/closure-conversion/)
- [Lecture 11: First-class Functions](https://course.ccs.neu.edu/cs4410/lec_lambdas_notes.html) of Northeastern University's course CS 4410/6410: Compiler Design

## Testing

That the [code examples](examples) produce the expected results can be verified by installing the [Bash Automated Testing System (BATS)](https://github.com/bats-core/bats-core) and running 

```
bats test/test.bats
```
