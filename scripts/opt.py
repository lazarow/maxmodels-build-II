import optuna
import subprocess
import time

instances_filepath=[
    "/mnt/d/Workspaces/research-workspace/2025-aspek-benchmarks/longest-circuit/p5.lp",
    "/mnt/d/Workspaces/research-workspace/2025-aspek-benchmarks/longest-circuit/p7.lp",
    "/mnt/d/Workspaces/research-workspace/2025-aspek-benchmarks/longest-circuit/p9.lp"
]
encoding_filepath="/mnt/d/Workspaces/research-workspace/2025-aspek-benchmarks/longest-circuit/encoding.lp"

def objective(trial) -> float:
    input_vector = [trial.suggest_float(f'v{i}', -2, 2) for i in range(11)]
    max_metrics_weight = trial.suggest_int("max_metrics_weight", 1, 9)
    start_time = time.perf_counter()
    for instance_filepath in instances_filepath:
        result = subprocess.run(
            ["bash", "maxmodels_tuning.sh", instance_filepath, encoding_filepath] + [",".join([str(x) for x in input_vector])] + [str(max_metrics_weight)],
            capture_output=True,
            text=True
        )
    elapsed = time.perf_counter() - start_time
    return elapsed

study = optuna.create_study(
    storage="sqlite:///db.sqlite3",
    direction="minimize"
)
study.optimize(objective, n_trials=100)

print(study.best_trial.value, study.best_trial.params)