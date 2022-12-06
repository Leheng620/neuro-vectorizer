PATH2LIB=~/EECS583/neuro-vectorizer/feature_parser/build/parser/FEATURE.so       # Specify your build directory in the project
PASS=-feature-parser

# Convert source code to bitcode (IR)
clang -emit-llvm -c ${1}.c -o ${1}.bc
# Canonicalize natural loops
opt -enable-new-pm=0 -loop-simplify ${1}.bc -o ${1}.ls.bc
# Instrument profiler
opt -enable-new-pm=0 -pgo-instr-gen -instrprof ${1}.ls.bc -o ${1}.ls.prof.bc
# Generate binary executable with profiler embedded
clang -fprofile-instr-generate ${1}.ls.prof.bc -o ${1}_prof

opt -enable-new-pm=0 -pgo-instr-use -pgo-test-profile-file=pgo.profdata -load ${PATH2LIB} ${PASS} < ${1}.bc > /dev/null