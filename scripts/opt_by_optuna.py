import os
import signal
import subprocess
import time
import optuna
import math

n = 13
allowed = [0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11]

instances_filepath=[
    "../datasets/data/longest-circuit/p01.lp",
    "../datasets/data/longest-circuit/p02.lp",
    "../datasets/data/longest-circuit/p03.lp",
    "../datasets/data/longest-circuit/p04.lp",
    "../datasets/data/longest-circuit/p10.lp",
    "../datasets/data/longest-circuit/p11.lp",
    "../datasets/data/longest-circuit/p12.lp",
    "../datasets/data/longest-circuit/p20.lp",
    "../datasets/data/longest-circuit/p21.lp",
    "../datasets/data/longest-circuit/p22.lp",
]

BASELINE_TIME = math.log(409.60)
INSTANCE_TIMEOUT = 600.0

def objective(trial):

    study = trial.study

    input_vector = [0] * n
    for i in allowed:
        input_vector[i] = trial.suggest_float(f"x{i}", -2, 2)

    if study.best_trials:
        incumbent = min(study.best_value, BASELINE_TIME)
    else:
        incumbent = BASELINE_TIME

    total_time = 0.0

    for instance_filepath in reversed(instances_filepath):

        start = time.perf_counter()

        try:
            proc = subprocess.Popen(
                ["bash", "maxmodels_tuning.sh", instance_filepath,
                 ",".join(str(x) for x in input_vector)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                start_new_session=True,  # Process group so we can kill entire pipeline on timeout
            )
            proc.communicate(timeout=INSTANCE_TIMEOUT)
            elapsed = time.perf_counter() - start

        except subprocess.TimeoutExpired:
            # Kill entire process group (bash + gringo + smodels + maxmodels + wmaxcdcl chain)
            # Prevents orphaned processes from accumulating on the server
            if hasattr(os, "killpg"):
                try:
                    os.killpg(os.getpgid(proc.pid), signal.SIGKILL)
                except (ProcessLookupError, OSError):
                    pass
            else:
                proc.kill()
            proc.wait()
            elapsed = INSTANCE_TIMEOUT

        total_time += math.log(max(elapsed, 1e-9))

        # Adaptive capping
        if total_time > incumbent:
            raise optuna.TrialPruned()

    return math.log(total_time)

sampler = optuna.samplers.CmaEsSampler(
    warn_independent_sampling=False
)
study = optuna.create_study(
    study_name="longest-circuit",
    storage="sqlite:///db.sqlite3",
    direction="minimize",
    sampler=sampler
)
study.optimize(objective, n_trials=2000)

print(study.best_trial.value, study.best_trial.params)