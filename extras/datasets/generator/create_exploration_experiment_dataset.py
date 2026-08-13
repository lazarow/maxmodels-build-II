import os
import random
import shutil

def pick_random_instances_per_problem(
    source_dir="../data",
    dest_dir="../data/exploration-experiment",
    n_instances=3,
    file_extension=".lp",
    random_seed=8913606
):
    """
    For each problem (i.e., each immediate subdirectory under source_dir),
    pick n_instances random files with the given extension and copy them
    into a folder named exploration-experiment, preserving the problem structure.

    Renames selected files to avoid filename conflicts, using canonical names.
    """
    random.seed(random_seed)
    os.makedirs(dest_dir, exist_ok=True)

    i = 1
    for problem_name in os.listdir(source_dir):
        problem_path = os.path.join(source_dir, problem_name)
        if not os.path.isdir(problem_path):
            continue
        if problem_name == "exploration-experiment":
            continue

        # Gather all instance files for this problem
        files = [
            f
            for f in os.listdir(problem_path)
            if f.endswith(file_extension) and os.path.isfile(os.path.join(problem_path, f))
        ]
        if not files:
            continue

        # Pick n_instances at random
        selected = random.sample(files, min(n_instances, len(files)))
        new_fnames = []
        for fname in selected:
            src = os.path.join(problem_path, fname)
            # New filename to avoid conflicts: use problem name and index
            new_fname = f"p{i:02d}{file_extension}"
            new_fnames.append(new_fname)
            i += 1
            dst = os.path.join(dest_dir, new_fname)
            shutil.copy2(src, dst)
        print(f"Selected from {problem_name}: {selected} (written as {new_fnames})")

if __name__ == "__main__":
    pick_random_instances_per_problem()