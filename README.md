# cliqpart
A Branch and Cut Algorithm for CLIQUE PARTITIONING

## Building
Make sure [Gurobi](https://www.gurobi.com) is installed properly
and the [environment variables](https://www.gurobi.com/documentation/10.0/quickstart_linux/software_installation_guid.html)
are set up so that your build environment (shell, IDE, etc.) can read them.

To create the output directory:
```
mkdir build/
cmake -S . -B build
```

And to build the project to `build/bin/cliqpart`:
```
cmake --build build
```
