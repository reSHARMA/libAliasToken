<h1 align="center">
  <br>
  <a href="#"><img src="https://i.ibb.co/mCjDmJW/d9a96151-4b9f-4c4b-bc66-bdbf64d46b2e-200x200.png" alt="libAliasToken" width="200"></a>
  <br>
  libAliasToken
  <br>
</h1>

<h4 align="center">A minimalistic abstraction over LLVM IR for cleaner alias analysis implementation in LLVM</h4>


## Table of Contents

- [Getting Started](#getting-started)
  - [Building from source](#build-from-source)
  - [Using with opt](#using-with-opt)
- [Usage](#usage)
  - [Creating a new alias token](#create-a-new-alias-token)
  - [Abstracting information from LLVM IR instructions](#abstracting-information-from-llvm-ir-instructions)
  - [Creating a dummy alias token](#creating-a-dummy-alias-token)
- [Supported Instructions](#supported-instructions)
  - [LoadInst](#loadinst)
  - [StoreInst](#storeinst)
  - [BitCastInst](#bitcastinst)
  - [AllocaInst](#allocainst)
  - [ReturnInst](#returninst)

## Getting Started

Any entity from LLVM IR can be easily converted into an alias token that acts like a unified abstraction.

For example the operands of an instruction, ```x = load y``` can be abstracted into two alias tokens ```x``` and ```y```.

### Building from source
```sh
$ git clone this_repository.git
$ cd this_repository
$ mkdir build; cd build
$ cmake .. && make
$ make install
```
### Using with opt
* Path to shared libary and headers should be searchable by the compiler
* In your LLVM IR pass:
  * Include the header file, ```AliasToken/AliasToken.h```
  * Use namespace ```AliasUtil```
* Load libAliasToken.so before your pass's shared library
  * ``` opt -load /usr/local/lib/libAliasToken.so -load yourPass.so ... ```

## Usage
Alias Tokens can be generated for any LLVM IR's entity, below are some common use cases.

### Creating a new alias token
Directly creating new alias tokens from the entities is supported but not recommended until required as it requires to the alias token bank to avoid duplication of tokens.
There are cases where we need to explicitly create a alias token example GEP instructions.
```cpp
...
#include "AliasToken/AliasToken.h"
...
using namespace AliasUtil
...
for(llvm::StoreInst* SI = llvm::dyn_cast<llvm::StoreInst>(Inst)){
  Alias * X = new Alias(SI -> getPointerOperand());
  X = AT.getAliasToken(X);
}
```

* ```AT``` is an object of ```AliasTokens``` class and should be unique to a module. It store all the tokens for a single module
* ```getAliasToken``` returns alias token from ```AliasTokens``` either by creating a new one or using the already existing one.

### Abstracting information from LLVM IR instructions
LibAliasToken provides abstraction for common LLVM IR instructions that can be used to generate alias tokens with out explicitly handling each operand.
```cpp
...
#include "AliasToken/AliasToken.h"
...
using namespace AliasUtil
...
for(llvm::StoreInst* SI = llvm::dyn_cast<llvm::StoreInst>(Inst)){
  auto StoreAliasVec = AT.extractAliasToken(SI);
  // StoreAliasVec[0] is the alias token for SI -> getValueOperand()
  // StoreAliasVec[1] is the alias token for SI -> getPointerOperand()
}
```
### Creating a dummy alias token
Creating of dummy alias token can be useful in few use cases. LibAliasToken supports generation of dummy alias token.
```cpp
...
#include "AliasToken/AliasToken.h"
...
using namespace AliasUtil
...
AT.getAliasToken("?", nullptr) // Creates a dummy alias token with name ? and with global scope
```
## Supported Instructions
Some commonly used instructions are supported directly and can be used as follows:
```cpp
...
#include "AliasToken/AliasToken.h"
...
using namespace AliasUtil
...
auto AliasVec = AT.extractAliasToken(Inst);
// AT is the object of AliasTokens
// Inst is a pointer to llvm::Instruction
```
### LoadInst
```LoadInst``` of syntax ```x = load y``` can be extracted into ```{X, Y}```
### StoreInst
```StoreInst``` of syntax ```store x y``` can be extracted into ```{X, Y}```
### BitCastInst
```BitCastInst``` of syntax ```x = bitcast y``` can be extracted into ```{X, Y}```
### AllocaInst
```AllocaInst``` of syntax ```x = alloca``` can be extracted into ```{X, Y}``` where Y is a dummy node
### ReturnInst
```ReturnInst``` of syntax ```return X``` can be extracted into ```{X}```
