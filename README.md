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
```c++
jbed std.enums;

dalaton add2<T: Kitzado>(x: T, y: T): T {
    rje3 x + y;
}


```


# TODO
- [ ] generics support
- [ ] pointers support
- [ ] struct support
- [ ] for/while support
- [ ] ranges support
- [ ] better error messages