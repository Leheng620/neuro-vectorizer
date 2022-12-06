# Install Instructions for Neurovectorizer on Ubuntu 18.04
----
### Table of Contents:
1. [Installing Anaconda, CMAKE, and LLVM](#anaconda-cmake-llvm)
2. [Install LLVM](#install-llvm)
3. [Install Neurovectorizer Dependencies](#install-neurovectorizer-dependencies)  
4. [Edit configure&#46;sh](#edit-configure)
5. [RISELab Students Directory Setup on Clusters](#directory-setup)

### Anaconda CMAKE LLVM: 
----
Skip any dependency you already have.
```
wget https://repo.anaconda.com/archive/Anaconda3-2020.02-Linux-x86_64.sh
bash Anaconda3-2020.02-Linux-x86_64.sh  # RISELab Students make sure to install in /data/[username]/ when asked rather than HOME
wget https://github.com/Kitware/CMake/releases/download/v3.17.1/cmake-3.17.1-Linux-x86_64.tar.gz
tar xzf cmake-3.17.1-Linux-x86_64.tar.gz
cd cmake-3.17.1-Linux-x86_64
make
make install
```


### Install LLVM
- Use the official release of source codes(Recommended)

1.  download the release llvm-project 14.0.6 source code from https://github.com/llvm/llvm-project/releases/download/llvmorg-14.0.6/llvm-project-14.0.6.src.tar.xz ; or git clone https://github.com/llvm/llvm-project.git and git checkout llvmorg-14.0.6

2.  cd llvm-project
    
3.  mkdir build && cd build
    
4.  cmake -G \[options\] ${LLVM_HOME}
    
    - Example using ninja: cmake -DLLVM\_ENABLE\_PROJECTS="clang;compiler-rt" -G "Ninja" ../llvm
        
    - **Example using make:  cmake -DLLVM\_ENABLE\_PROJECTS="clang;compiler-rt" -G "Unix Makefiles" ../llvm**
    - If you do not have enough space under ‘/’ (Use \`df -h\` to check that), you can specify the option -DCMAKE\_INSTALL\_PREFIC to another directory that have enough space
        
    - set -DLLVM\_USE\_LINKER=gold will speedup the linker time on x86 / x86_64 machines.
    - `-DCMAKE_INSTALL_PREFIX=directory` — Specify for *directory* the full pathname of where you want the LLVM tools and libraries to be installed (default `/usr/local`).

5.  The default target (i.e. ninja or make) install will build all of LLVM.
    
    -  Example using ninja:  ninja install -j N (N: N is the number of cores you want to use)
        
    -  Example using make: make install -j N
        
    -  Running a serial build will be slow. To improve speed, try running a parallel build. But if you set ‘N’ too high, the build process may freeze due to insufficient memory. (Usually during linking process around 97% of completion based on my experience)
        
    -  The build will take a long time, usually more than 2 hours.
        
6.  Try run ‘clang --version’ and ‘opt --version’ to see if you can get the version info (v14.0.6) without any error. In case of "Command 'opt' not found", opt is located in `./llvm-project/build/bin/opt`.


After LLVM is installed, verify that by creating a test pass. Create a project directory looks like this.
```
e.g, myproject/                       # Your own project directory
         |-> build/                   # Build directory for your pass
         |-> CMakeLists.txt           # label as (a)
         |-> mypass/ 
               |-> CMakeLists.txt     # label as (b)
               |-> mypass.cpp         # Your LLVM pass


```
Copy the following into (a).

```
cmake_minimum_required(VERSION 3.4.3)
find_package(LLVM REQUIRED CONFIG)                        # This will find the shared LLVM build.
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")        # You don't need to change ${LLVM_CMAKE_DIR} since it is already defined.
include(AddLLVM)
add_definitions(${LLVM_DEFINITIONS})                      # You don't need to change ${LLVM_DEFINITIONS} since it is already defined.
include_directories(${LLVM_INCLUDE_DIRS})                 # You don't need to change ${LLVM_INCLUDE_DIRS} since it is already defined.
add_subdirectory(mypass)                                  # Add the directory which your pass lives.
```

Copy the following into (b).

```
add_llvm_library(LLVMMypass MODULE            # Name of the shared library that you will generate
       mypass.cpp                          # Your pass
       PLUGIN_TOOL
       opt
)
```

Copy the code to mypass.cpp

```cpp
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct Hello : public FunctionPass {
  static char ID;
  Hello() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    errs() << "Hello: ";
    errs().write_escaped(F.getName()) << '\n';
    return false;
  }
}; // end of struct Hello
}  // end of anonymous namespace

char Hello::ID = 0;
static RegisterPass<Hello> X("hello", "Hello World Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
```

Go to the build directory of your project and run the following.

```bash
# export LLVM_DIR which is the directory that contains LLVMConfig.cmake
export LLVM_DIR=<path>/llvm-project/build/lib/cmake/llvm/
# <path> is the absolute path to llvm-project directory
cd myproject/build
cmake ..       
```

Run make. (Main LLVM build is configured with Makefile system)

```
make -j2
```

You can find the generated shared library under your build directory. (e.g., build/mypass/LLVMMypass.so)

The way you apply your pass to LLVM bitcode will be the same.

Then you need another .c file, I will use the example from [Getting Started with the LLVM System](https://llvm.org/docs/GettingStarted.html#an-example-using-the-llvm-tool-chain).

First, create a simple C file under the project root directory, name it ‘hello.c’

```
#include <stdio.h>

int main() {
  printf("hello world\n");
  return 0;
}
```

Next, compile the C file into a native executable:

```
$ clang -emit-llvm hello.c -c -o hello.bc
```

The -emit-llvm option can be used with the -S or -c options to emit an LLVM `.ll` or `.bc` file (respectively) for the code. This allows you to use the [standard LLVM tools](https://llvm.org/docs/CommandGuide/index.html) on the bitcode file.

After you have generated *.bc file, you are able to use the HelloWorld pass to deal with it.

```
$ opt -enable-new-pm=0 -load [path-to-llvm-library] -hello < hello.bc > /dev/null
```

In this example, `path-to-llvm-library` is `~/myproject/build/mypass/LLVMMypass.so`. LLVMMypass is set in`mypass/CMakeLists.txt`

The `-hello` is the name of the registered pass in Hello.cpp.

**Do not forget** `-enable-new-pm=0` since we use the legacy pass manager https://llvm.org/docs/WritingAnLLVMPass.html#introduction-what-is-a-pass 

hello.bc is the llvm bitcode file generated from hello.c

**In case of command "opt" not found**

```bash
# <path> is the absolute path to llvm-project
export opt=<path>/llvm-project/build/bin/opt
$opt -enable-new-pm=0 -load [path-to-llvm-library] -hello < hello.bc > /dev/null
```

You will see the output as

```
Hello: main
```

That’s the function name of hello.c

You can replace any c or c++ files with hello.c, and the HelloWorld pass will print the function names of them.

For more details, please refer to [Writing an LLVM Pass](https://llvm.org/docs/WritingAnLLVMPass.html)

### Install Neurovectorizer Dependencies: 
----
First make sure you're in your anaconda environment.
```
conda install -c conda-forge clang
conda install -c conda-forge tensorflow
pip install tensorflow
pip install ray
pip install ray[rllib]
pip install clang
```
**Important**: Uninstall and reinstall protobuf because the default version is not compatible.
```
pip uninstall protobuf
pip install protobuf==3.20.1
```

### Edit preprocess/configure.sh:
----
```
CLANG_PATH=<path>/llvm-project/build/lib/libclang.so
CLANG_BIN_PATH=<path>/llvm-project/build/bin
```

####  Directory Setup: 
----
```
mkdir ./code2vec/data
```
