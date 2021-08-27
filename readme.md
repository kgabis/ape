# The Ape Programming Language

## Try Ape in your browser on [Ape Playground](https://kgabis.github.io/apeplay/).

## About
Ape is an easy to use programming language and library written in C. It's an offspring of [Monkey](https://monkeylang.org) language (from [Writing An Interpreter In Go](https://interpreterbook.com) and [Writing A Compiler In Go](https://compilerbook.com) books by [Thorsten Ball](https://thorstenball.com)), but it evolved to be more procedural with variables, loops, operator overloading, modules, and more.

## Current state
It's under development so everything in the language and the api might change.

## Example
```javascript
fn contains_item(to_find, items) {
    for (item in items) {
        if (item == to_find) {
            return true
        }
    }
    return false
}

const cities = ["Warszawa", "Rabka", "Szczecin"]
const city = "Warszawa"
if (contains_item(city, cities)) {
    println(`found ${city}!`)
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

Ape is a dynamically typed language with mark and sweep garbage collection. It's compiled to bytecode and executed on internal VM. It's fairly fast for simple numeric operations and not very heavy on allocations (custom allocators can be configured). More documentation can be found [here](documentation.md).

### Basic types
```bool```, ```string```, ```number``` (double precision float), ```array```, ```map```, ```function```, ```error```

### Operators
```
Math:
+ - * / %

Binary:
^ | & << >>

Logical:
! < > <= >= == != && ||

Assignment:
= += -= *= /= %= ^= |= &= <<= >>=
```

### Defining constants and variables
```javascript
const constant = 2
constant = 1 // fail
var variable = 3
variable = 7 // ok
```

## Strings
```javascript
const str1 = "a string"
const str2 = 'also a string'
const str3 = `a template string, it can contain expressions: ${2 + 2}, ${str1}`
```

### Arrays
```javascript
const arr = [1, 2, 3]
arr[0] // -> 1
```

### Maps
```javascript
const map = {"lorem": 1, 'ipsum': 2, dolor: 3}
map.lorem // -> 1, dot is a syntactic sugar for [""]
map["ipsum"] // -> 2
map['dolor'] // -> 3
```

### Conditional statements
```javascript
if (a) {
    // a
} else if (b) {
    // b
} else {
    // c
}
```

### Loops
```javascript
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

for (var i = 0; i < 10; i++) {
    // body
}
```

### Functions
```javascript
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
    return {
        name: name,
        greet: fn() {
            println(`Hello, I'm ${this.name}`)
        },
    }
}
```

### Errors
```javascript
const err = error("something bad happened")
if (is_error(err)) {
    println(err)
}

fn() {
    recover (e) { // e is a runtime error wrapped in error
        return null
    }
    crash("something bad happened") // crashes are recovered with "recover" statement
}
```

### Modules
```javascript
import "foo" // import "foo.ape" and load global symbols prefixed with foo::

foo::bar()

import "bar/baz" // import "bar/baz.ape" and load global symbols prefixed with baz::
baz::foo()
```

### Operator overloading
```javascript
fn vec2(x, y) {
    return {
        x: x,
        y: y,
        __operator_add__: fn(a, b) { return vec2(a.x + b.x, a.y + b.y)},
        __operator_sub__: fn(a, b) { return vec2(a.x - b.x, a.y - b.y)},
        __operator_minus__: fn(a) { return vec2(-a.x, -a.y) },
        __operator_mul__: fn(a, b) {
            if (is_number(a)) {
                return vec2(b.x * a, b.y * a)
            } else if (is_number(b)) {
                return vec2(a.x * b, a.y * b)
            } else {
                return vec2(a.x * b.x, a.y * b.y)
            }
        },
    }
}
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

## Visual Studio Code extension

A Visual Studio Code extension can be found [here](https://marketplace.visualstudio.com/items?itemName=KrzysztofGabis.apelang).

## My other projects
* [parson](https://github.com/kgabis/parson) - JSON library
* [kgflags](https://github.com/kgabis/kgflags) - command-line flag parsing library   
* [agnes](https://github.com/kgabis/agnes) - header-only NES emulation library

## License
[The MIT License (MIT)](http://opensource.org/licenses/mit-license.php)
