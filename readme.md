# The Ape Programming Language

## About
Ape is an easy to use programming language and library written in C. It's an offspring of [Monkey](https://monkeylang.org) language (from [Writing An Interpreter In Go](https://interpreterbook.com) and [Writing A Compiler In Go](https://compilerbook.com) books by [Thosten Ball](https://thorstenball.com)), but it evolved to be more procedural with variables, loops, and more.

## Current state
It's under development so everything in the language and the api might change.

## Example
```
fn contains_item(to_find, items) {
    for (item in items) {
        if (item == to_find) {
            return true
        }
    }
    return false
}

const cities = ["Warszawa", "Rabka", "Szczecin"]
if (contains_item("Warszawa", cities)) {
    println("found!")
}
```

## Embedding
Add ape.h and ape.c to your project and compile ape.c with a C compiler before linking.

```c
#include "ape.h"

int main() {
    ape_t *ape = ape_make();
    ape_execute(ape, "println(\"hello world\")");
    ape_destroy(ape);
    return 0;
}
```

An example that shows how to call Ape functions from C code and vice versa can be found [here](examples/api.c).

## Language

Ape is a dynamically typed language with mark and sweep garbage collection. It's compiled to bytecode and executed on internal VM. It's fairly fast for simple numeric operations and not very heavy on allocations (custom allocators can be configured).

### Basic types
```bool```, ```string```, ```number``` (double precision float), ```array```, ```map```, ```function```, ```error```

### Operators
```
Math:
+ - * /

Logical:
! < > <= >= == != && ||

Assignment:
= += -= *= /=
```

### Defining constants and variables
```
const constant = 2
constant = 1 // fail
var variable = 3
variable = 7 // ok
```

### Arrays
```
const arr = [1, 2, 3]
arr[0] // -> 1
```

### Maps
```
const map = {"lorem": 1, 'ipsum': 2, dolor: 3}
map.lorem // -> 1, dot is a syntactic sugar for [""]
map["ipsum"] // -> 2
map['dolor'] // -> 3
```

### Conditional statements
```
if (a) {
    // a
} else if (b) {
    // b
} else {
    // c
}
```

### Loops
```
while (true) {
    // body
}

var items = [1, 2, 3]
for (item in items) {
    if (item == 2) {
        break
    } else {
        continue
    }
}

for (var i = 0; i < 10; i += 1) {
    // body
}
```

### Functions
```
const add_1 = fn(a, b) { return a + b }

fn add_2(a, b) {
    return a + b
}

fn map_items(items, map_fn) {
    const res = []
    for (item in items) {
        append(res, map_fn(item))
    }
    return res
}

map_items([1, 2, 3], fn(x){ return x + 1 })

fn make_person(name) {
    const person = {}
    person.name = name
    person.greet = fn() {
        println("Hello, I'm " + name)
    }
    return person
}
```

### Errors
```
const err = error("something bad happened)
if (is_error(err)) {
    println(err)
}
```

### Modules
```
import "foo" // imports "foo.bn" and load global symbols prefixed with foo::

foo::bar()
```

## Splitting and joining

ape.c can be split into separate files by running utils/split.py:

```
utils/split.py --input ape.c --output-path ape
```

It can be joined back into a single file with utils/join.py:

```
utils/join.py --template utils/ape.c.templ --path ape --output ape.c
```

## License
[The MIT License (MIT)](http://opensource.org/licenses/mit-license.php)