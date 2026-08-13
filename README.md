# maxmodels (build II)

This project is an extended version of the original [maxmodels](https://github.com/lazarow/maxmodels). While it builds upon the ideas and features from the previous version, **build II** includes major changes that set it apart from earlier releases.

## Installation and compiling

To build **maxmodels (build II)**, you'll need:

- a working Linux,
- C++20 compiler,
- GNU make,
- Python 3.x,
- `patch` command.

### 1. Clone the repository

```sh
git clone <this-repo>
cd <this-repo>
```

### 2. Install dependencies

To install the dependencies (a default MaxSAT solver), run:

```sh
make install_dependencies
```

### 3. Build the executable file

To build the project, run:

```sh
make clean build
```

The solver will be placed at `bin/maxmodels`.

## Running

You can strictly simplify or encode & solve or both:

- simplification via `bin/maxmodels --simplify` (returns Smodels Internal Format),
- encoding and solving via `bin/maxmodels --solve`,
- both: `bin/maxmodels --simplify --solve`.

The solving requires an external MaxSAT solver compatible with the standard WCNF format (after 2022). There two ways of providing a path to the external solver:

1. In `Makefile`, there is the variable `EXTERNAL_SOLVER_PATH`, which put the default path during compilation. By default, the script looks after `wmaxcdcl` in `PATH`.
2. You can provided the path via `--external-solver=...`.

The solver expects a logic program in Smodels Internal Format. Hence, the example usage of `gringo` and `idlv` is:

- `gringo --output=smodels program.lp | bin/maxmodels --simplify --solve`,
- `dlv --mode=idlv program.lp | bin/maxmodels --simplify --solve`.

The default encoding is Clark Completion and [ASSAT](https://cse.hkust.edu.hk/assat/)-like loop formulas.

There is also [lp2sat](http://www.tcs.hut.fi/Software/lp2sat/)-like compact completion for a program extended with level rankings by means of [lp2lp2](http://www.tcs.hut.fi/Software/lp2sat/), call it like below:  
`gringo --output=smodels program.lp | bin/maxmodels --simplify | lp2lp2 | bin/maxmodels --solve`

## Datasets

To generate datasets (logic programs of 10 well-known problems, 269 instances in total) run the below commands.

```sh
cd extras/datasets/generator
python generate_from_jsonl.py
```

The datasets' configuration are encoded as \*.jsonl files. The generated logic programs are placed in the `extras/datasets/data` directory.

### Exploration experiments

To conduct limited exploration experiments, the randomly selected set of problems can be created from all problems. Run the below commands.

```sh
cd extras/datasets/generator
python create_exploration_experiment_dataset.py
```

Note that, all problems must be ex ante generated. The selected renamed problems will be placed in the `extras/datasets/data/exploration-experiment` directory.
