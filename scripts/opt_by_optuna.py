import subprocess
import time
import optuna

n = 13
allowed = [0, 1, 4, 7]

instances_filepath=[
    "/mnt/d/Workspaces/research-workspace/2026-maxmodels-build-II/scripts/test5.lp",
]

def objective(trial) -> float:
    input_vector = [0] * n
    for i in allowed:
        input_vector[i] = trial.suggest_float(f"x{i}", -2, 2)
    start_time = time.perf_counter()
    for instance_filepath in instances_filepath:
        try:
            subprocess.run(
                ["bash", "maxmodels_tuning.sh", instance_filepath] + [",".join([str(x) for x in input_vector])],
                capture_output=True,
                timeout=600,
                text=True
            )
        except subprocess.TimeoutExpired:
            continue
    elapsed = time.perf_counter() - start_time
    return elapsed

study = optuna.create_study(
    storage="sqlite:///db.sqlite3",
    direction="minimize"
)
study.optimize(objective, n_trials=100)

print(study.best_trial.value, study.best_trial.params)