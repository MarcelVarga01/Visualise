# Visualise

Imperative Programming Coursework wrote in C. Visualise can take as an argument a basic data type (or a list of basic data types) and decimal/binary numbers, converting decimal to binary and vice-versa.

## Compilation

On UNIX systems, `cd` to the repository's directory and use `make`.


## Usage
```bash
./visualise [DataType] [Value]
./visualise {[DataType1]\;...\;[DatatypeN]} [Value1]...[ValueN]
```
Using `./visualise` without any attributes tests the program
## Examples

```bash
./visualise char 7
0000 0111

./visualise char 255
Input error.

./visualise char 1000 0000
-128

./visualise {char\;int\;unsigned char} 7 10000000 255
0000 0111 0000 0000 1001 1000 1001 0110 1000 0000 1111 1111

./visualise {char\;int\;unsigned char} 0000 0111 0000 0000 1001 1000 1001 0110 1000 0000 1111 1111
7 10000000 255
```

## Notes
The program correctly checks for any input errors: 
* Provided datatype cannot hold the Value
* Too many/few values
