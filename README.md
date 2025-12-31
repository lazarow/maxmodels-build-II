# maxmodels (build II)

This project is an extended version of the original [maxmodels](https://github.com/lazarow/maxmodels). While it builds upon the ideas and features from the previous version, **build II** includes major changes that set it apart from earlier releases.

## Installation and compiling

To build **maxmodels (build II)**, you'll need a working Linux, C++20 compiler, GNU make, and access to the CPLEX library (headers and static library). The build process also uses git submodules for dependencies.

### 1. Clone the repository

```sh
git clone <this-repo>
cd <this-repo>
```

### 2. Configure CPLEX environment

Set the following environment variables or edit the Makefile to specify the paths to your CPLEX installation:

-   `CPLEX_LIB_DIR`: Absolute or relative path to the CPLEX static libraries (should contain `libcplex.a`)
-   `CPLEX_INC_DIR`: Path to the CPLEX headers (should contain the subdirectory `ilcplex/`)

### 3. Prepare the build environment

Run the following to initialize git submodules and build dependencies:

```sh
make prepare
```

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
