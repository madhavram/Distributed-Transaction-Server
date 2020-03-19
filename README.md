# Distributed-Transaction-Server
Berkeley Algorithm, steps for implementation is below:
g++ -o berkeley Berkeley.cpp -pthread
./berkeley process 1
./berkeley process 2 
./berkeley process 3
./berkeley master

Steps for compiling two versions of causal ordering is given below
Causally ordering multicast
g++ -std=c++11 -o causalordering causalordering.cpp -pthread
./causalordering 0 <message>
./causalordering 1 <message>
./causalordering 2 <message>

Non-Causal ordering multicast
g++ -std=c++11 -o noncausalordering noncausalordering.cpp -pthread
./causalordering 0 <message>
./causalordering 1 <message>
./causalordering 2 <message>
