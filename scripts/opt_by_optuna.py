import subprocess
import time
import optuna

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

BASELINE_TIME = 1234.56
INSTANCE_TIMEOUT = 600.0

def objective(trial):

    study = trial.study

    input_vector = [0] * n
    for i in allowed:
        input_vector[i] = trial.suggest_float(f"x{i}", -2, 2)

    if study.best_trial is not None:
        incumbent = min(study.best_value, BASELINE_TIME)
    else:
        incumbent = BASELINE_TIME

    total_time = 0.0

    for instance_filepath in reversed(instances_filepath):

        start = time.perf_counter()

        try:
            subprocess.run(
                ["bash", "maxmodels_tuning.sh", instance_filepath,
                 ",".join(str(x) for x in input_vector)],
                capture_output=True,
                timeout=INSTANCE_TIMEOUT,
                text=True
            )
            elapsed = time.perf_counter() - start

        except subprocess.TimeoutExpired:
            elapsed = INSTANCE_TIMEOUT

        total_time += elapsed

        # Adaptive capping
        if total_time > incumbent:
            raise optuna.TrialPruned()

    return total_time

sampler = optuna.samplers.CmaEsSampler()
study = optuna.create_study(
    study_name="opt_by_optuna",
    storage="sqlite:///db.sqlite3",
    direction="minimize",
    sampler=sampler
)
study.optimize(objective, n_trials=1200)

print(study.best_trial.value, study.best_trial.params)