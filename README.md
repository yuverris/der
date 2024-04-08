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
- [ ] generics support
- [ ] pointers support
- [x] struct support
- [X] for loop support
- [X] enums support
- [ ] better error messages
- [ ] constant types
- [x] subscript operator
- [x/2] instance handling
- [ ] use of struct/enums in struct types as well
- [ ] error handling (no idea how?????!!! but ill figure out)
- [ ] nested scopes
- [ ] named scopes
- [x] use of structs/enum in types
- [x] type check the return type with the actual return statement
- [ ] implement @skip_ir to skip the IR codegen and write C code instead
- [ ] file source code imports
- [ ] unary ops

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

# Language design
NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS NO IMPLICIT BS 