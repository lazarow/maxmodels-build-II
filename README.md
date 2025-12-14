# maxmodels (build II)

## Installation and compiling

To build **maxmodels 2**, you'll need a working C++20 compiler, GNU make, and access to the CPLEX library (headers and static library). The build process also uses two git submodules for dependencies.

### 1. Clone the repository

```sh
git clone <this-repo>
cd <this-repo>
```

### 2. Prepare the build environment

Run the following to initialize git submodules and build dependencies:

```sh
make prepare
```

### 3. Configure CPLEX environment

Set the following environment variables or edit the Makefile to specify the paths to your CPLEX installation:

-   `CPLEX_LIB_DIR`: Absolute or relative path to the CPLEX static libraries (should contain `libcplex.a`)
-   `CPLEX_INC_DIR`: Path to the CPLEX headers (should contain the subdirectory `ilcplex/`)

### 4. Check prerequisites

Before building, you can verify everything is set up via:

```sh
make prereqs
```

### 5. Build the main executable

To build the project, run:

```sh
make build
```

The main executable will be placed at `bin/maxmodels`.
