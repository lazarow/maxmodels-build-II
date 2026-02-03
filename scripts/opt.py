import subprocess
import time

k = 3
n = 13
allowed = [0, 1, 4, 7]

instances_filepath=[
    "/mnt/d/Workspaces/research-workspace/2026-maxmodels-build-II/scripts/test4.lp",
]

def test_vector(input_vector, timeout=600) -> float:
    start_time = time.perf_counter()
    for instance_filepath in instances_filepath:
        try:
            subprocess.run(
                ["bash", "maxmodels_tuning.sh", instance_filepath] + [",".join([str(x) for x in input_vector])],
                capture_output=True,
                timeout=timeout,
                text=True
            )
        except subprocess.TimeoutExpired:
            continue
    elapsed = time.perf_counter() - start_time
    return elapsed

base_vector = [0] * n
best_vector = base_vector
best_time = float('inf')
best_index = -1
best_pairs = {}
timeout = test_vector(base_vector, 600)
print(f"Base vector: " + ",".join([str(x) for x in base_vector]) + f" - Measured time: {timeout}")
timeout = timeout + 1
for j in range(k):
    print("=" * 40)
    print(f"Round {j+1} of {k} Base vector: " + ",".join([str(x) for x in base_vector]))
    for i in range(n):
        if i not in allowed:
            print(f"Skipping index {i} because it is not in allowed")
            continue
        if i in best_pairs:
            print(f"Skipping index {i} because it is in best pairs")
            continue
        for sign in [1, -1]:
            base_vector[i] = sign
            measured_time = test_vector(base_vector, timeout)
            print(f"Vector: " + ",".join([str(x) for x in base_vector]) + f" - Timeout: {timeout} - Measured time: {measured_time}")
            if measured_time < best_time:
                best_time = measured_time
                best_vector = base_vector.copy()
                best_index = i
                timeout = measured_time + 1
        base_vector[i] = 0
    best_pairs[best_index] = best_vector[best_index]
    base_vector[best_index] = best_vector[best_index]
    print("Best vector: " + ",".join([str(x) for x in best_vector]) + f" - Best time: {best_time} - Best index: {best_index} - Best sign: {best_vector[best_index]}")
print("=" * 40)
print("Final vector: " + ",".join([str(x) for x in best_vector]) + f" - Best time: {best_time}")