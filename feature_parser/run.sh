WORKDIR=/n/eecs583a/home/luoxim/workspace/proj                   # TODO: Replace me with your work directory path
PATH2LIB=${WORKDIR}/feature_parser/build/parser/FEATURE.so       # Specify your build directory in the project
PASS=-feature-parser

BENCH=${1}

cleanup(){
rm ${BENCH}.bc
}

# Convert source code to bitcode (IR)
# This approach has an issue with -O2, so we are going to stick with default optimization level (-O0)
clang -emit-llvm -c ${BENCH}.c -o ${BENCH}.bc 

# Apply your pass to bitcode (IR)
opt -enable-new-pm=0 -load ${PATH2LIB} ${PASS} < ${BENCH}.bc > /dev/null

# clean up
cleanup
