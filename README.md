# derija
low level statically typed programming language that compiles to C, based on the Moroccan dialect darija.
> NOTE: the language is not inherently safe by any means and everything is left up to the programmer (in the time being).
# code example
<img src="https://i.imgur.com/PEgIPIJ.png">

it was a very stupid and bad idea to use C as a backend, biggest mistake of the century

# language
## types
there are 4 primite types:
- ra9m which signifies a 32 bit int
- ktba equivalent to a string (const char*)
- harf equivalent to a char
- bool
- [<type>; N] a static array
- *<type> a pointer
## literals
- `79465` -> ra9m
- `"salam 3alam!"` -> ktba
- `'a'` -> harf
- `sa7i7` or `khata2` -> bool
- `[1,4,7,9,8]` -> array of ra9m [ra9m; 5]
- `&x` pointer to x
- `*x` dereference the pointer x

## variables
```cpp
dir x: ra9m = 7;
```
## arrays
```cpp
dir arr: [ra9m; 3] = [1, 7, 8];
dir r: ra9m = arr[0];
```
## pipe operator
the pipe operator is nothing but a syntax sugar used to pass left hand of the op to the right hand as an argument
```cpp
x |> foo();
```
## if statements
```cpp
ila x < 8 {
    ...;
} awla {
    ...;
};
```
## one lined if statements
```cpp
x == 7 ?? ...; 
```
## for loops
```cpp
lkola x: 1...7 {
    ...;
};
```
break/continue statements are not yet supported.
## functions
```cpp
dalaton zid2(a: ra9m, b: ra9m): ra9m {
    rje3 a + b,
}
```
optional arguments are not yet supported.
## structs
```cpp
jism No9ta {
    x: ra9m;
    y: ra9m;
};
dir y: No9ta = jadid No9ta{x: 1, y: 2};
dyr z: ra9m = y.x;
```
## enums
```cpp
ti3dad Cmp {
    EQ, LT, GT
};
dir abc: Cmp = Cmp.EQ;
```

# TODO
- [x] unary ops
- [X] for loop support
- [x] struct support
- [X] enums support
- [x] subscript operator
- [x] instance handling
- [x] use of struct/enums in struct types as well
- [x] use of structs/enum in types
- [x] pointers support
- [x] array as arguments/returns

- [ ] move to a runtime instead !!
- [ ] pointer casting
- [ ] variadic args
- [ ] better error messages
- [ ] file source code imports
- [ ] C compatibility
- [ ] memory allocation 
- [ ] generics support
- [ ] constant types
- [ ] error handling
- [ ] nested scopes
- [ ] named scopes
- [ ] function overloading
- [ ] algebraic enums
- [ ] standard library
- [ ] Memory Safety, GC/Borrow Checker/RAII....?????!!!!!!.....
- [ ] Implement safety checks ???
- [ ] Optimizations
