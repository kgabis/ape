# Ape Documentation

<p><b>APE IS UNDER ACTIVE DEVELOPMENT SO EVERYTHING HERE CAN BE WRONG OR OUT OF DATE.<br/>
IF IN DOUBT, CHECK THE SOURCE CODE.</b></p>

### [Table of Contents](#)

[1. Error handling](#error-handling)<br/>
[2. Builtins](#builtins)<br/>

<a id="error-handling"></a>
### 1. Error handling

During program execution runtime errors can occur (for instance if wrong number of arguments is passed to a function). They can be handled with ```recover``` statement that works similar to ```catch``` statement in other languages with few important differences:
* It needs to be declard <b>before</b> a runtime error it handles might occur
* It cannot be nested
* It's only valid within functions and <b>must</b> end with a return statement

For instance, let's say there's a function ```foo()``` that calls ```bar()``` and then ```baz()```.

```javascript
fn foo() {
    recover (e) { // e is a runtime error wrapped in error
        println(e)
        return 0
    }
    bar() // if it fails foo() will return 0

    recover (e) {
        println(e)
        return 1
    }
    baz() // if it fails foo() will return 1

    return 2
}
```

Runtime crashes can be caused by calling ```crash("msg")``` function, however, if it's something expected that can be handled by caller, it's better to return an ```error``` value.

```javascript
const err = error("something bad happened")
if (is_error(err)) {
    println(err)
}
```

<a id="builtins"></a>
### 2. Builtins

`len(string | array | map)` -> `number`
```javascript
  var aStr = "a string"
  var aArr = [1, 2, 3]
  var aMap = { "1": 1, "2": 2 }

  len(aStr) // 8
  len(aArr) // 3
  len(aMap) // 2
```
<br/>

`first(array)` -> `object`
```javascript
  var aArr = [1, 2, 3]

  first(aArr) // 1
```
<br/>

`last(array)` -> `object`
```javascript
  var aArr = [1, 2, 3]

  last(aArr) // 3
```
<br/>

`rest(array)` -> `array`
```javascript
  var aArr = [1, 2, 3, 4, 5, 6, 7]
  var bArr = []

  rest(aArr) // [2, 3, 4, 5, 6, 7]
  rest(bArr) // null
```
<br/>

`reverse(array | string)` -> `array | string`
```javascript
  var aArr = [1, 2, 3]
  var aStr = "abc"

  reverse(aArr) // [3, 2, 1]
  reverse(aStr) // "cba"
```
<br/>

`array(number)` -> `array`<br>
`array(number, object)` -> `array`
```javascript
  var aArr = array(3) // [null, null, null]
  var bArr = array(3, "a") // ["a", "a", "a"]
  var cArr = array(3, {}) // [{}, {}, {}]
```
<br/>

`append(array, object)` -> `number`
```javascript
  var aArr = [1]

  append(aArr, 1) // 2
  append(aArr, "a") // 3
```
<br/>

`println(object, ...)` -> `null`
```javascript
  var aMap = { "a": 1, "b": 2 }

  // appends newline
  println("a") // "a"
  println(aMap) // { "a": 1, "b": 2 }
```
<br/>

`print(object, ...)` -> `null`
```javascript
  var aMap = { "a": 1, "b": 2 }

  println("a") // "a"
  println(aMap) // { "a": 1, "b": 2 }
```
<br/>

`write_file(string, string)` -> `number`
```javascript
  var path = "./ex.txt"
  var data = "A string in a text file"

  write_file(path, data) // 22
```
<br/>

`read_file(string)` -> `string`
```javascript
  var path = "./ex.txt"

  read_file(path) // "A string in a text file"
```
<br/>

`to_str(string | number | bool | null | map | array)` -> `string`
```javascript
  var aVal = true

  to_str(aVal) // "true"
```
<br/>

`char_to_str(number)` -> `string`
```javascript
  char_to_str('a') // "a"
```
<br/>

`range(number)` -> `array`<br/>
`range(number, number)` -> `array`<br/>
`range(number, number, number)` -> `array`<br/>
```javascript
  var aStart = 2
  var aEnd = 10
  var aStep = 2

  range(aEnd) // [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
  range(aStart, aEnd) // [2, 3, 4, 5, 6, 7, 8, 9]
  range(aStart, aEnd, aStep) // [2, 4, 6, 8]
```
<br/>

`keys(map)` -> `array`
```javascript
  var aMap = { "a": 1, "b": 2 }

  keys(aMap) // ["a", "b"]
```
<br/>

`values(map)` -> `array`
```javascript
  var aMap = { "a": 1, "b": 2 }

  values(aMap) // [1, 2]
```
<br/>

`copy(object)` -> `object`
```javascript
  var aMap = { "a": 1, "b": 2 }
  var bMap = null

  bMap = copy(aMap) // { "a": 1, "b": 2 }
```
<br/>

`deep_copy(object)` -> `object`
```javascript
  var aMap = { "a": 1, "b": 2, "c": { "a": 1, "b": 2, "c": { "a": 1 } } }
  var bMap = null

  bMap = deep_copy(aMap) // { "a": 1, "b": 2, "c": { "a": 1, "b": 2, "c": { "a": 1 } } }
```
<br/>

`concat(array | string, object)` -> `number | string`
```javascript
  var aArr = [1, 2]
  var aStr = "ab"
  var bStr = ""

  concat(aArr, [3]) // aArr [1, 2, [3]] -> 3
  concat(aArr, "c") // type error!

  bStr = concat(aStr, "c") // "abc"
  // aStr == "ab"
```
<br/>

`assert(bool)` -> `bool`
```javascript
  assert((1 == 1)) // true
  assert(1 == 2)) // false

  assert((1+2)) // error!
```
<br/>

`remove(array, object)` -> `bool`
```javascript
  var aArr = [1, 2, 3, true]

  remove(aArr, 3) // true
  remove(aArr, 3) // false
```
<br/>

`remove_at(array, number)` -> `bool`
```javascript
  var aArr = [1, 2, 3, true]

  remove_at(aArr, 2) // true
  remove_at(aArr, 2) // true
```
<br/>

`error(string | null)` -> `error`
```javascript
  error("an error")
  error()
```
<br/>

`random(number, number)` -> `number`
```javascript
  random(1, 5)
  random(10, 2) // error!
```
<br/>


#### Type Checks
---

`is_string(object)` -> `bool`
<br/>

`is_array(object)` -> `bool`
<br/>

`is_map(object)` -> `bool`
<br/>

`is_number(obejct)` -> `bool`
<br/>

`is_bool(object)` -> `bool`
<br/>

`is_null(object)` -> `bool`
<br/>

`is_function(object)` -> `bool`
<br/>

`is_external(object)` -> `bool`
<br/>

`is_error(object)` -> `bool`
<br/>


#### Math
---

`sqrt(number)` -> `number`
<br/>

`pow(number, number)` -> `number`
```javascript
  pow(2, 2) // 4
```

`sin(number)` -> `number`
<br/>

`cos(number)` -> `number`
<br/>

`tan(number)` -> `number`
<br/>

`log(number)` -> `number`
<br/>

`ceil(number)` -> `number`
<br/>

`floor(number)` -> `number`
<br/>

`abs(number)` -> `number`
<br/>

