Introduction

This is x-lang (X language, x, x-language, whatever). The language created by me as a small C alternative. The language is aimed to support roughly the same feature set plus some additional features that are quite handy in everyone's everyday development. An example of such features would be slices. Unlike C, X doesn't have arrays. Assigning value list to slice will lead to static slice creation. Slices are also bounds checked.

1. Examples.

DECLARATIONS

Declaration syntax is different from most other languages.
  :int x; // declares x as int

The colon is 'declaration operator', so to say. It is only used in statements context. In struct decls (TODO), function argument decls &c., the colon isn't used.

Other types:
  :int$ x;             // pointer to int
  :int[] x;            // slice of int
  :int<-(bool, int) x; // function returning int, taking bool and int

The types cascade from left to right, from the inner-most to the outer most.
  :int$[] x;           // slice of pointer to int
  :int[]$ x;           // pointer of slice of int
  :int$<-(bool)[]      // slice of function returning pointer to int

A value can be optionally assigned upon declaration.
  :bool error = false;

OPERATIONS
The main operations in the language:
  - + * /              // binary/unary arithmetic
  < > <= >= == !=      // relational operators
  and or               // binary logical operators (add more?)
  $variable            // address of operator (unary, to the left)
  @pointer             // dereference operator
  slice[index]         // indexing operator
  slice[low:high]      // (sub-)slicing operator
  += -= *= /= =        // assignment
  print                // prints any value (to be removed?)

Operator precedence table (high to low):
1. Indexing, slicing operators
2. Unary minus, addressof, dereference operators
3. Multiplication/Division
4. Addition/subtraction
5. Relational operators
6. Binary logical operators

Then the expression:
  2 > 3 and 2+4*$x[3] == @array_ptr[index]

Will be parsed as:
  (2 > 3) and ((2 + (4 * (($x)[3])) == ((@array_ptr)[index]))

STATEMENTS
There's some sexy sugar for if and while statements. You don't have to parenthesize the condition.

(The angle braces aren't part of the syntax)

  if <condition> {}
  while <condition> {}

Although you _have_ to encapsulate the operators of the block inside curly braces. The braces in the final else statement are not required.

  if 2 < 3 {
    // ...
  }
  else if breadWroteHisLanguage() {
    // ...
  }
  else return false;

While loops support flow control operators such as break and continue.

FUNCTIONS

The lagnuage only has functions. Every function has a return value. If you want to create a procedure, create a function with int return type instead.
  :int<-() func {
     return;
  }

Inside functions a special implicit value "result" is initialized. By default it is zeroed for primitives and no action is done on complex types. In order to exit from function with return value, set the result value followed by the return statement:
  :int<-() main {
    result = -1;
    return;
  }

It is possible to write the same thing by putting a value into return statement:
  :int<-() main {
    return -1;
  }

This statement will set result variable's value and return it.

Source code conventions (mostly for me, not enforced):
1.  all identifiers are written in snake_case.
2.  except some macros and enum types, they can be in CAPS_SNAKE but also snake_case.
3.  additionally, types are prefixed with t_.
8.  opening curly bracket on the same line (bjarne stroustroup/1TBS).
9.  prioritize conserving vertical space (don't overdo, if there's logical segmentation, put a blank line)
