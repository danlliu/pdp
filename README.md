# pdp

An assembler and simulator for the PDP-1 computer, written in C++17.

## How to Use

To compile:

```
make
```

creates `./assembler` and `./simulator` executables.

```
make debug
```

creates `./assembler_debug` and `./simulator_debug` executables, which provide additional logging.

```
make clean
```

removes all object files and executables.

Check out [assembler.cpp](./assembler_src/assembler.cpp) and [simulator.cpp](./simulator_src/simulator.cpp) for usage of the assembler and simulator.

