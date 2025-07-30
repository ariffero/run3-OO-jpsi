import json
import subprocess
import os
import argparse
import itertools
import shutil
import csv
import sys

# Recursively search for a key in a nested dictionary
# Used for sanity check to ensure scan parameters exist in base config
def find_key_recursive(d, key):
  """Return True if key exists anywhere in nested dict d."""
  if isinstance(d, dict):
    if key in d:
      return True  # direct match found at this level
    return any(find_key_recursive(v, key) for v in d.values())  # recurse into values
  return False     # non-dict types cannot contain keys

# Recursively set a key to a value in a nested dictionary
# Used to update config for each scan run
def set_key_recursive(d, key, value):
  """Set key to value anywhere in nested dict d."""
  if isinstance(d, dict):
    if key in d:
      d[key] = value  # replace on first match
      return True     # stop further recursion for this key
    for v in d.values():
      if set_key_recursive(v, key, value):
        return True   # propagate success up the call stack
  return False        # key not found in this branch

# Entry point for parameter scan automation
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
  # Define CLI arguments
  parser.add_argument('--param-json', required=True, help='JSON file with base config and parameters to scan')
  parser.add_argument('--task-name', default=None, help='Name of the analysis task to run (passed to run-task.py --script). If not set, uses run-task.py default.')
  parser.add_argument('--data-type', default=None, help='Data type to pass to run-task.py (--data-type)')
  parser.add_argument('-n', '--dry-run', action='store_true', help='Print all commands that would be executed, but do not run them')
  parser.add_argument('--log', action='store_true', help='Enable logging of terminal output to a file named <output_base>.log Note that an action is required from the terminal and it will be written in the log')
  parser.add_argument('-u', '--use-sub-jobs', action='store_true', help='Enable processing in sub-jobs via run-task subjob support')
  parser.add_argument('--chunk-num', type=int, default=None, help='Number of items per chunk when using sub-jobs')
  parser.add_argument('--chunk-max-size', type=float, default=None, help='Maximum data size (GB) per chunk when using sub-jobs')
  args = parser.parse_args()        # parse and validate input flags

  # Path to the analysis script invoked for each parameter set
  run_task_script = os.path.expanduser("~/Desktop/run3-OO-jpsi/scripts/run-task.py")

  # Verify CLI JSON file exists
  if not os.path.isfile(args.param_json):
    print(f"Error: parameter JSON file '{args.param_json}' not found.")
    sys.exit(1)

  # Load and validate JSON
  try:
    with open(args.param_json, "r") as f:
      param_dict = json.load(f)
  except json.JSONDecodeError as e:
    print(f"Error: failed to parse JSON '{args.param_json}': {e}")
    sys.exit(1)

  # Check required keys
  for key in ("base_config", "scan_params"):
    if key not in param_dict:
      print(f"Error: '{key}' missing in parameter JSON.")
      sys.exit(1)

  # scan_params must be a non-empty dict of lists
  sp = param_dict["scan_params"]
  if (not isinstance(sp, dict) or not sp or
    any(not isinstance(v, list) or not v for v in sp.values())):
    print("Error: 'scan_params' must be a non-empty dict of non-empty lists.")
    sys.exit(1)

  # Verify base_config file exists
  base_cfg = param_dict["base_config"]
  if not os.path.isfile(base_cfg):
    print(f"Error: base config file '{base_cfg}' not found.")
    sys.exit(1)

  scan_params = param_dict.get("scan_params", {})   # mapping param_name -> [values]
  base_config_file = param_dict.get("base_config")  # template JSON config
  base_output_name = param_dict.get(
    "output_base", os.path.splitext(os.path.basename(args.param_json))[0])
  output_dir = param_dict.get("output_dir", "scan") # default output folder

  # Verify base config exists before proceeding
  if not base_config_file or not os.path.isfile(base_config_file):
    print(f"Error: base config file '{base_config_file}' not found.")
    exit(1)

  # Prepare output directory and load the base config into memory
  if not os.path.exists(output_dir):
    os.makedirs(output_dir)
  with open(base_config_file, "r") as f:
    base_config = json.load(f)

  # Setup logging to file if requested (captures all stdout/stderr including subprocesses)
  if args.log:
    log_filename = f"{base_output_name}.log"
    log_path = os.path.join(output_dir, log_filename)
    # Open log file in line-buffered mode to capture prints in real-time
    log_file = open(log_path, 'w', buffering=1)
    # Redirect OS-level stdout and stderr file descriptors (1 and 2)
    os.dup2(log_file.fileno(), 1)  # stdout -> log_file
    os.dup2(log_file.fileno(), 2)  # stderr -> log_file
    # Also redirect Python-level streams in case modules write directly
    sys.stdout = log_file
    sys.stderr = log_file
    print(f"Logging all output (including subprocesses) to {log_path}")

  # Extract parameter names and lists of values
  param_names = list(scan_params.keys())
  param_values = [scan_params[k] for k in param_names]

  # Compute every combination of parameters to test
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

  # Ensure that each scanned parameter key exists in the base config
  missing_params = [p for p in param_names if not find_key_recursive(base_config, p)]
  if missing_params:
    print(f"Error: The following scanned parameters are missing in base config: {', '.join(missing_params)}")
    exit(1)

  file_map = {}     # will hold mapping of runs to outputs
  file_counter = 1  # incremental run index

  # Loop through each parameter combination, run analysis, and collect outputs
  for values in all_combinations:
    config = json.loads(json.dumps(base_config))  # deep copy to isolate runs
    for k, v in zip(param_names, values):
      set_key_recursive(config, k, v)  # apply new values

    config_name = f"{base_output_name}-{file_counter}.json"
    config_path = os.path.join(output_dir, config_name)
    with open(config_path, "w") as f:
      json.dump(config, f, indent=2)  # save per-run config

    # Record existing ROOT files to detect new ones
    before_files = set(f for f in os.listdir('.') if f.endswith('.root'))

    # Construct and optionally execute the analysis command
    cmd = [
      "python3", run_task_script,
      "-j", config_path
    ]
    if args.task_name:
      cmd += ["-s", args.task_name]
    if args.data_type:
      cmd += ["-t", args.data_type]
    if args.use_sub_jobs:
      cmd += ["-u"]
      jobs_dest = os.path.join(output_dir, f"jobs-{file_counter}")
      print("jobs_dest", jobs_dest)
      cmd += ["--jobs-dir", jobs_dest]
      if args.chunk_num is not None:
        cmd += ["--chunk-num", str(args.chunk_num)]
      if args.chunk_max_size is not None:
        cmd += ["--chunk-max-size", str(args.chunk_max_size)]
  
    print(f"\nRun {file_counter}:")
    print(f"  Parameters: {dict(zip(param_names, values))}")
    print(f"  Command: {' '.join(cmd)}")
    if args.dry_run:
      print("  [Dry-run] Command not executed.")
    else:
      subprocess.run(cmd, check=True)  # halt on failure

    # Move any newly generated ROOT files into the output directory
    after_files = set(f for f in os.listdir('.') if f.endswith('.root'))
    new_files = after_files - before_files
    run_file_names = []
    for fname in new_files:
      dest = os.path.join(output_dir, fname)
      shutil.move(fname, dest)
      run_file_names.append(fname)
    run_file_names.append(config_name)  # include the config file in record
    file_map[f"run-{file_counter}"] = {
      "params": dict(zip(param_names, values)),
      "files": run_file_names
    }
    file_counter += 1

  # Save mapping of all runs to a JSON
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