# Compiler Project (Fall 2023) - The ARK Language
A simple compiler using LLVM-12.


**ARK** stands for **A**bedini-**R**afiee-**K**ebriti.

## Report Doc
For more information and examples read the report doc from [here](https://docs.google.com/document/d/1CVHNe-TRmAkv87q5B0dNQTUfN2ph-UzLq_N4HEYY1o0/).

## How to run?
Enter project folder(Preferably Phase 2), then:
```
mkdir build
cd build
cmake ..
make
cd src
```
Now here next to the ARK compiler in `build\src\`, you need to write your code in `main.ARK` file.
Then you can run it by:
```
./ARK
```
## How To See The Result?
### Step-by-Step Run:
```
./ARK > ark.ll
```
```
llc --filetype=obj -o=ark.o ark.ll
```
```
clang -o arkbin ark.o ../../rtARK.c
```
```
./arkbin
```
### One-Step Run:
```
./ARK > ark.ll && llc --filetype=obj -o=ark.o ark.ll && clang -o arkbin ark.o ../../rtARK.c && ./arkbin
```
