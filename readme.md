source code conventions:

1.  all identifiers are written in `snake_case`.
2.  except some macros and enum types, they can be in `CAPS_SNAKE` but also `snake_case`.
3.  additionally, types are prefixed with `t_`.
4.  type qualifiers follow the type they apply to (`char const *name`, `char *const name`).
5.  storage qualifiers are written first.
6.  typedefs are written in between the declared type and and declarant type.
7.  don't `int* i`, do `int *i` instead.
8.  opening curly bracket on the same line.
9.  prioritize conserving vertical space (don't overdo, if there's logical segmentation, put a blank line)
10. (weak rule) prefer `+=`/`-=` to post/pre increment/decrement
