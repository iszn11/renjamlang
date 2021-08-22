# Renjamlang (RJL)

Renjamlang (aka RJL) is a scripting language made in 48h in C++ for the first
langjam hosted at [langjam/jam0001](https://github.com/langjam/jam0001). The
theme was *First-class comments*.

In this language, comments can be attached to values and have string
interpolation ability, so you can substitute parts of your comments with
variables defined in the scope of a comment.

Most of the syntax uses *Reverse RPN* (prefix notation), because ~~it's easier
to parse and there's no operator precedence~~ it's superior and doesn't require
parentheses. Comments are old C-style `/* This is a comment. */`, but can
reference variables. For examples:

`/* The value of x is $x. */ = x 10`

This line sets the value of `x` to `10` and attaches a comment. When you later
type in `x` in REPL to print the value, you will get this output:

```
/* The value of x is 10. */
10
```

You get the comment with `$x` substituded for the actual value of `x` and the
value of `x` below.

Fun fact: this language doesn't use semicolons *and* doesn't care about
whitespace (spaces and newlines are treated equally), so you can have a program
that is just a giant single line.

# Building

You need `g++`. Run `./build.sh` or run directly:

```
g++ -std=c++17 -pedantic -Wall -Wextra -g -o rjl Common.cpp Main.cpp Lexer.cpp Parser.cpp Interpreter.cpp
```

# Running

After building run `./rjl` to get a REPL or `./rjl FILE` to read and execute a
script in `FILE`.

# Small examples

Some lines for you to try out in REPL.

# Examples

Examples available at [Example](./Examples) directory.

## Closures

**Command:**

```
./rjl Examples/closures.rjl
```

**Script:**

```
= random fn ()
  /*
  Chosen by fair dice roll.
  Guaranteed to be random.
  */
  return 4
end

= make_adder fn (a)
  /* Function that adds $a to a number. */
  return fn (x)
    /* Result of adding $a and $x. */
    return + a x
  end
end

= adder1 make_adder (1)
= adder4 make_adder (4)

random ()

adder1
adder4

adder1 (10)
adder4 (20)
make_adder (100) (200)
```

**Output:**

```
/*
Chosen by fair dice roll.
Guaranteed to be random.
*/
4
/* Function that adds 1 to a number. */
fn (x)
/* Function that adds 4 to a number. */
fn (x)
/* Result of adding 1 and 10. */
11
/* Result of adding 4 and 20. */
24
/* Result of adding 100 and 200. */
300
```

## Prime test

**Command:**

```
./rjl Examples/prime.rjl
```

**Script:**

```
= isPrime fn (a)
  if <= a 1
    return false
  end
  = i 2
  while <= * i i a
    if == % a i 0
      /* $a is not prime (divisible by $i). */
      return false
    end
    = i + i 1
  end
  /* $a is prime. */
  return true
end

= i 2 while <= i 10
  isPrime (i)
  = i + i 1
end
```

**Output:**

```
/* 2 is prime. */
true
/* 3 is prime. */
true
/* 4 is not prime (divisible by 2). */
false
/* 5 is prime. */
true
/* 6 is not prime (divisible by 2). */
false
/* 7 is prime. */
true
/* 8 is not prime (divisible by 2). */
false
/* 9 is not prime (divisible by 3). */
false
/* 10 is not prime (divisible by 2). */
false
```

## Sieve of Eratosthenes

**Command:**

```
./rjl Examples/sieve.rjl
```

**Script:**

```
= sieve fn (n)
  = A []
  = i 0
  while <= i n
    push A 1
    = i + i 1
  end
  = i 2
  while <= * i i n
    if == @ A i 1
      = j * i i
      while <= j n
        = @ A j 0
        = j + j i
      end
    end
    = i + i 1
  end
  = out []
  = i 2
  while <= i n
    if == @ A i 1 push out i end
    = i + i 1
  end
  /* Prime numbers from 2 to $n. */
  return out
end

sieve (100)
```

**Output:**

```
/* Prime numbers from 2 to 100. */
[2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97]
```
