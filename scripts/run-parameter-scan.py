import json
import subprocess
import os
import argparse
import itertools
import shutil
import csv

# Recursively search for a key in a nested dictionary
# Used for sanity check to ensure scan parameters exist in base config

def find_key_recursive(d, key):
  """Return True if key exists anywhere in nested dict d."""
  if isinstance(d, dict):
    if key in d:
      return True
    return any(find_key_recursive(v, key) for v in d.values())
  return False

# Recursively set a key to a value in a nested dictionary
# Used to update config for each scan run

def set_key_recursive(d, key, value):
  """Set key to value anywhere in nested dict d."""
  if isinstance(d, dict):
    if key in d:
      d[key] = value
      return True
    for v in d.values():
      if set_key_recursive(v, key, value):
        return True
  return False

def main():
  parser = argparse.ArgumentParser(
    description="""
    Automates parameter scans for O2Physics analysis by running a task multiple times with different config values.
    Reads a parameter JSON file specifying the base config, output directory, and scan parameters.
    For each combination of scan parameters, creates a config, runs the analysis, and organizes output files.
    Produces a mapping (JSON and CSV) between output files and parameter values for each run.
    Includes sanity checks: verifies base config file exists and all scanned parameters are present (recursively) in the base config.
    The parameter JSON must include:
      - base_config: path to the base config file
      - output_base: prefix for output files
      - output_dir: directory for scan results
      - scan_params: dictionary of parameters to scan
    """
  )
  parser.add_argument('--param-json', required=True, help='JSON file with base config and parameters to scan')
  parser.add_argument('--task-name', default=None, help='Name of the analysis task to run (passed to run-task.py --script). If not set, uses run-task.py default.')
  parser.add_argument('--data-type', default=None, help='Data type to pass to run-task.py (--data-type)')
  parser.add_argument('-n', '--dry-run', action='store_true', help='Print all commands that would be executed, but do not run them')
  args = parser.parse_args()

  run_task_script = os.path.expanduser("~/Desktop/run3-OO-jpsi/scripts/run-task.py")

  # Load scan parameters and config paths
  with open(args.param_json, "r") as f:
    param_dict = json.load(f)

  # Extract scan parameters group and config info
  scan_params = param_dict.get("scan_params", {})
  base_config_file = param_dict.get("base_config")
  base_output_name = param_dict.get("output_base", os.path.splitext(os.path.basename(args.param_json))[0])
  output_dir = param_dict.get("output_dir", "scan")

  # Check base config file exists
  if not base_config_file or not os.path.isfile(base_config_file):
    print(f"Error: base config file '{base_config_file}' not found.")
    exit(1)

  # Create output directory if needed
  if not os.path.exists(output_dir):
    os.makedirs(output_dir)
  with open(base_config_file, "r") as f:
    base_config = json.load(f)

  param_names = list(scan_params.keys())
  param_values = [scan_params[k] for k in param_names]

  # Prepare all combinations
  all_combinations = list(itertools.product(*param_values))
  print(f"\nScan summary:")
  print(f"  Number of runs: {len(all_combinations)}")
  print(f"  Parameter combinations:")
  for i, values in enumerate(all_combinations, 1):
    combo_str = ', '.join(f"{k}={v}" for k, v in zip(param_names, values))
    print(f"    Run {i}: {combo_str}")
  user_input = input("\nType 'exit' to abort, or press Enter to continue: ")
  if user_input.strip().lower() == 'exit':
    print('Aborted by user request.')
    return

  # Sanity check: all scanned parameters must be present in base config (recursively)
  # This prevents silent errors if a parameter is not used by the analysis
  missing_params = [p for p in param_names if not find_key_recursive(base_config, p)]
  if missing_params:
    print(f"Error: The following scanned parameters are missing in base config: {', '.join(missing_params)}")
    exit(1)

  # Map for output file names and parameter values
  file_map = {}
  file_counter = 1

  # Iterate over all combinations of scan parameters
  for values in all_combinations:
    # Deep copy base config for each run
    config = json.loads(json.dumps(base_config))
    # Set scan parameters in config (recursively)
    for k, v in zip(param_names, values):
      set_key_recursive(config, k, v)
    # Use dash for config file name
    config_name = f"{base_output_name}-{file_counter}.json"
    config_path = os.path.join(output_dir, config_name)
    with open(config_path, "w") as f:
      json.dump(config, f, indent=2)

    # Track .root files before running analysis
    before_files = set(f for f in os.listdir('.') if f.endswith('.root'))

    # Build command for analysis run
    cmd = [
      "python3", run_task_script,
      "-j", config_path
    ]
    if args.task_name:
      cmd += ["-s", args.task_name]
    if args.data_type:
      cmd += ["-t", args.data_type]
    print(f"\nRun {file_counter}:")
    print(f"  Parameters: {dict(zip(param_names, values))}")
    print(f"  Command: {' '.join(cmd)}")
    if args.dry_run:
      print("  [Dry-run] Command not executed.")
    else:
      subprocess.run(cmd, check=True)

    # Track new .root files produced by this run
    after_files = set(f for f in os.listdir('.') if f.endswith('.root'))
    new_files = after_files - before_files

    # Move and rename all new .root files for this run
    run_file_names = []
    for fname in new_files:
      ext = os.path.splitext(fname)[1]
      # Replace underscores in fname with dashes
      dash_fname = fname.replace('_', '-')
      new_name = f"{base_output_name}-{file_counter}-{dash_fname}"
      dest = os.path.join(output_dir, new_name)
      shutil.move(fname, dest)
      run_file_names.append(new_name)
    # Add the config json to the files list
    run_file_names.append(config_name)
    file_map[f"run-{file_counter}"] = {
      "params": {k: v for k, v in zip(param_names, values)},
      "files": run_file_names
    }
    file_counter += 1

  # Save the map to a json file in output_dir
  map_path = os.path.join(output_dir, f"{base_output_name}-file-map.json")
  with open(map_path, "w") as f:
    json.dump(file_map, f, indent=2)

  # Also produce a CSV for human readability
  csv_path = os.path.join(output_dir, f"{base_output_name}-file-map.csv")
  with open(csv_path, "w", newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["run", *param_names, "config-json", "root-files"])
    for run, info in file_map.items():
      params = [info["params"].get(k, "") for k in param_names]
      config_json = [f for f in info["files"] if f.endswith(".json")]
      root_files = [f for f in info["files"] if f.endswith(".root")]
      writer.writerow([
        run.replace("run-", ""),
        *params,
        ";".join(config_json),
        ";".join(root_files)
      ])

if __name__ == "__main__":
  main()