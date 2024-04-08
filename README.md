# derija
low level statically typed prgoramming language that compiles to C, based on the Moroccan dialect darija.
> NOTE: the language is not memory safe and requires manual memory management.
# code example
```c++
jbed std.io;

dalaton main(): walo {
    io.kteb("Salam, 3alam !\n");
};
```


# TODO
## the language itself
### urgent shit
- [x] unary ops
- [X] for loop support
- [x] struct support
- [X] enums support
- [x] subscript operator
- [x] instance handling
- [x] use of struct/enums in struct types as well
- [x] use of structs/enum in types
- [ ] pointers support
- [ ] better error messages
- [ ] file source code imports
- [ ] C compatibility
- [ ] memory allocation

### not that urgent
- [ ] generics support
- [ ] constant types
- [ ] error handling (no idea how?????!!! but ill figure out prolly requires moving to something like LLVM)
- [ ] nested scopes
- [ ] named scopes
- [ ] implement @skip_ir to skip the IR codegen and write C code instead
- [ ] variadic args
- [ ] function overloading

### later???
- [ ] algebraic enums

## stdlib
- [ ] write it lmao
- [ ] memory allocation

# What could be improved
- [ ] use LLVM instead or own IR tm instead of C codegen.
- [ ] use a much more effecient type checking algorithm.
- [ ] Memory Safety, GC/Borrow Checker/RAII....?????!!!!!!.....
- [ ] Implement safety checks ???
- [ ] Optimizations

# Reminder
NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS, NO FOKING IMPLICIT CONVERSIONS, 