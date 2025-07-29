#!/usr/bin/env python3

import os
import sys
import shutil
import argparse


def main():
  parser = argparse.ArgumentParser(
    description="Run O2Physics analysis with configurable inputs from repository utilities"
  )

  # Mutually exclusive: JSON config vs. data file
  group = parser.add_mutually_exclusive_group(required=True)
  group.add_argument(
    '-j', '--json',
    dest='json_file',
    help='JSON configuration filename in current directory (e.g. data.json)'
  )
  group.add_argument(
    '-d', '--data',
    dest='data_file',
    help='Path to input data file, e.g. dd1/AO2D.root'
  )

  parser.add_argument(
    '-s', '--script',
    dest='script',
    default='o2-analysis-ud-fwd-muons-upc',
    help='Analysis executable (default: o2-analysis-ud-fwd-muons-upc)'
  )
  parser.add_argument(
    '-t', '--data-type',
    dest='data_type',
    choices=['reco', 'gen', 'data'],
    default='reco',
    help='Data type: reco, gen, or data (default: reco)'
  )
  parser.add_argument(
    '-n', '--dry-run',
    dest='dry_run',
    action='store_true',
    help='Print commands and check inputs without executing them'
  )
  parser.add_argument('-u', '--use-sub-jobs', action='store_true', help='Enable chunked processing for txt input files')
  group_chunk = parser.add_mutually_exclusive_group()
  group_chunk.add_argument('--chunk-num', type=int, help='Number of files per chunk for txt input files (requires --use-sub-jobs)')
  group_chunk.add_argument('--chunk-max-size', type=float, help='Maximum total data size (in GB) per chunk for txt input files (requires --use-sub-jobs)')
  parser.add_argument('--jobs-dir', type=str, default='jobs', help='Directory for sub-job outputs (default: jobs, requires --use-sub-jobs)')
  args = parser.parse_args()

  # Enforce that chunking and jobs-dir options are only used if --use-sub-jobs is set
  if (args.chunk_num is not None or args.chunk_max_size is not None or args.jobs_dir != 'jobs') and not args.use_sub_jobs:
    print('Error: --chunk-num, --chunk-max-size, and --jobs-dir can only be used if --use-sub-jobs is set.')
    sys.exit(1)
  jobs_dir = args.jobs_dir if args.jobs_dir is not None else 'jobs'

  # Directories
  cwd = os.getcwd()
  script_dir = os.path.dirname(os.path.realpath(__file__))
  util_dir = os.path.abspath(os.path.join(script_dir, os.pardir, 'utilities'))

  json_file  = args.json_file
  data_file  = args.data_file
  use_config = bool(json_file)
  script     = args.script
  data_type  = args.data_type
  dry_run    = args.dry_run

  # Required utility files
  in_list   = os.path.join(util_dir, 'in.txt')
  tree_json = os.path.join(util_dir, f'tree-{data_type}.json')
  required  = [in_list, tree_json]

  # Add JSON or data paths
  if use_config:
    json_path = os.path.join(cwd, json_file)
    required.append(json_path)
    # Check for 'aod-file-private' in the correct nested location
    with open(json_path, 'r') as jf:
      try:
        import json as _json
        config = _json.load(jf)
      except Exception as e:
        print(f'Error: Could not parse config JSON: {e}')
        sys.exit(1)
      # Check for 'aod-file-private' under 'internal-dpl-aod-reader'
      reader = config.get('internal-dpl-aod-reader', {})
      aod_file_private = reader.get('aod-file-private', None)
      if not aod_file_private or str(aod_file_private).strip() == "":
        print(f"Error: 'aod-file-private' is missing or empty {json_path} in config JSON.")
        sys.exit(1)
      # Remove leading '@' if present
      file_ref = str(aod_file_private).lstrip('@')
      abs_input_path = os.path.join(cwd, file_ref) if not os.path.isabs(file_ref) else file_ref
      if not os.path.isfile(abs_input_path):
        print(f"Error: Input file specified in config ('aod-file-private'): {abs_input_path} not found.")
        sys.exit(1)
      required.append(abs_input_path)
  else:
    data_path = os.path.join(cwd, data_file)
    required.append(data_path)

  # Check for missing inputs
  missing = [f for f in required if not os.path.isfile(f)]
  if missing:
    print('Error: Missing required file(s):', ', '.join(missing))
    sys.exit(1)
  print('Input files verified:')
  for f in required:
    print(f'  - {f}')

  # Copy writer JSON config from utilities to cwd (always needed for saving trees)
  writer_json_src = tree_json
  writer_json_dst = os.path.join(cwd, f'tree-{data_type}.json')
  shutil.copy(writer_json_src, writer_json_dst)

  # Build analysis command
  writer_json_src = tree_json
  writer_json_dst = os.path.join(cwd, f'tree-{data_type}.json')
  if use_config:
    shutil.copy(writer_json_src, writer_json_dst)
  # else: assume writer_json already exists or generated

  # Build analysis command
  if use_config:
    cmd_analysis = (
      f"{script} --configuration json://{json_path} "
      f"--aod-writer-json {writer_json_dst} -b"
    )
    base = os.path.splitext(os.path.basename(json_file))[0]
    output_root = os.path.join(cwd, f"{base}-AnalysisResults.root")
    merge_output = f"{base}-tree.root"
  else:
    writer_json = writer_json_dst
    cmd_analysis = (
      f"{script} --aod-file {data_path} "
      f"--aod-writer-json {writer_json} -b"
    )
    output_root = os.path.join(cwd, 'AnalysisResults.root')
    merge_output = 'data-tree.root' if data_type=='data' else f"{data_type}-tree.root"

  # Merge command
  cmd_merge = (
    f"o2-aod-merger --input {in_list} --output {merge_output} --max-size 1000000000"
  )

  # Report
  print('\nCommands to be executed:')
  print(f'  Analysis: {cmd_analysis}')
  print(f'  Merge:    {cmd_merge}\n')
  if dry_run:
    print('Dry-run mode: no commands will be executed.')
    sys.exit(0)

  # Sub-job logic: if enabled and input is .txt, run only sub-jobs and skip main analysis/merge
  if args.use_sub_jobs and use_config:
    reader = None
    aod_file_private = None
    with open(json_path, 'r') as jf:
      import json as _json
      config_base = _json.load(jf)
      reader = config_base.get('internal-dpl-aod-reader', {})
      aod_file_private = reader.get('aod-file-private', None)
    file_ref = str(aod_file_private).lstrip('@')
    if file_ref.endswith('.txt'):
      abs_input_path = os.path.join(cwd, file_ref) if not os.path.isabs(file_ref) else file_ref
      jobs_dir = args.jobs_dir
      if not os.path.exists(jobs_dir):
        os.makedirs(jobs_dir)
      with open(abs_input_path, 'r') as fin:
        lines = [line for line in fin if line.strip()]
      # Determine chunking method
      if args.chunk_max_size:
        # Chunk by total data size (in GB)
        chunk_max_bytes = args.chunk_max_size * 1024**3
        chunks = []
        current_chunk = []
        current_size = 0
        for line in lines:
          file_path = os.path.expanduser(line.strip())
          if not os.path.isabs(file_path):
            file_path = os.path.join(os.path.dirname(abs_input_path), file_path)
          try:
            fsize = os.path.getsize(file_path)
          except Exception as e:
            print(f'Warning: Could not get size for {file_path}: {e}. Skipping.')
            continue
          if current_size + fsize > chunk_max_bytes and current_chunk:
            chunks.append(current_chunk)
            current_chunk = []
            current_size = 0
          current_chunk.append(line)
          current_size += fsize
        if current_chunk:
          chunks.append(current_chunk)
        print(f'Chunking by max size: {args.chunk_max_size} GB per chunk, total {len(chunks)} chunks.')
      else:
        # Default: chunk by number of lines (files)
        chunk_num = args.chunk_num if args.chunk_num else 2
        chunks = [lines[i:i+chunk_num] for i in range(0, len(lines), chunk_num)]
        print(f'Chunking by number of files: {chunk_num} per chunk, total {len(chunks)} chunks.')
      job_outputs = []
      for idx, chunk in enumerate(chunks, 1):
        chunk_file = os.path.join(jobs_dir, f'chunk-{idx}.txt')
        with open(chunk_file, 'w') as fout:
          fout.writelines([l if l.endswith('\n') else l+'\n' for l in chunk])
        # Deep copy config for each job
        import copy
        config = copy.deepcopy(config_base)
        config['internal-dpl-aod-reader']['aod-file-private'] = '@' + chunk_file
        job_json = os.path.join(jobs_dir, f'{os.path.splitext(os.path.basename(json_file))[0]}-job-{idx}.json')
        with open(job_json, 'w') as jf:
          _json.dump(config, jf, indent=2)
        writer_json_dst = os.path.join(cwd, f'tree-{data_type}.json')
        cmd_analysis = (
          f"{script} --configuration json://{job_json} "
          f"--aod-writer-json {writer_json_dst} -b"
        )
        base = os.path.splitext(os.path.basename(json_file))[0]
        output_root = os.path.join(jobs_dir, f"{base}-AnalysisResults-job-{idx}.root")
        print(f'\nSub-job {idx}:')
        print(f'  Analysis: {cmd_analysis}')
        print(f'  Output:   {output_root}')
        if dry_run:
          print('  [Dry-run] Command not executed.')
        else:
          ret = os.system(cmd_analysis)
          if ret != 0:
            print(f"Error: Analysis command failed with exit code {ret}")
            sys.exit(ret)
          default_out = os.path.join(cwd, 'AnalysisResults.root')
          if not os.path.isfile(default_out):
            print(f"Error: Expected output '{default_out}' not found. Analysis may have failed.")
            sys.exit(1)
          shutil.move(default_out, output_root)
        # Move dimu.root if present
        dimu_src = os.path.join(cwd, 'dimu.root')
        dimu_dst = os.path.join(jobs_dir, f'dimu-job-{idx}.root')
        if os.path.isfile(dimu_src):
          shutil.move(dimu_src, dimu_dst)
        job_outputs.append(output_root)
      print('All sub-jobs completed.')
      # Prepare merge lists
      analysis_results = [os.path.join(jobs_dir, f) for f in os.listdir(jobs_dir) if f.startswith(base + '-AnalysisResults-job-') and f.endswith('.root')]
      analysis_list_file = os.path.join(jobs_dir, 'analysis_merge_list.txt')
      with open(analysis_list_file, 'w') as f:
        for fname in sorted(analysis_results):
          f.write(fname + '\n')
      final_analysis = os.path.join(cwd, f'{base}-AnalysisResults.root')
      merge_cmd_analysis = f"hadd -f {final_analysis} {' '.join(sorted(analysis_results))}"
      print(f'  Merging AnalysisResults (histograms) with hadd: {merge_cmd_analysis}')
      if not dry_run:
        ret_merge = os.system(merge_cmd_analysis)
        if ret_merge != 0:
          print(f"Error: Merging AnalysisResults with hadd failed with exit code {ret_merge}")
          sys.exit(ret_merge)
      # Merge dimu.root files with o2-aod-merger
      dimu_files = [os.path.join(jobs_dir, f) for f in os.listdir(jobs_dir) if f.startswith('dimu-job-') and f.endswith('.root')]
      dimu_list_file = os.path.join(jobs_dir, 'dimu_merge_list.txt')
      with open(dimu_list_file, 'w') as f:
        for fname in sorted(dimu_files):
          f.write(fname + '\n')
      final_dimu = os.path.join(cwd, f'{base}-tree.root')
      merge_cmd_dimu = f"o2-aod-merger --input {dimu_list_file} --output {final_dimu} --max-size 1000000000"
      print(f'  Merging dimu.root files: {merge_cmd_dimu}')
      if not dry_run:
        ret_merge_dimu = os.system(merge_cmd_dimu)
        if ret_merge_dimu != 0:
          print(f"Error: Merging dimu.root files failed with exit code {ret_merge_dimu}")
          sys.exit(ret_merge_dimu)
      print('All sub-jobs merged. Final outputs:')
      print(f'  {final_analysis}')
      print(f'  {final_dimu}')
      # Remove temporary writer JSON copy
      try:
        os.remove(writer_json_dst)
      except FileNotFoundError:
        pass
      return
  # Main single-job analysis/merge (only if not sub-job chunked mode)
  # Execute analysis
  ret = os.system(cmd_analysis)
  if ret != 0:
    print(f"Error: Analysis command failed with exit code {ret}")
    sys.exit(ret)

  # Verify and rename output
  default_out = os.path.join(cwd, 'AnalysisResults.root')
  if not os.path.isfile(default_out):
    print(f"Error: Expected output '{default_out}' not found. Analysis may have failed.")
    sys.exit(1)
  os.rename(default_out, output_root)

  # Merge trees
  print("\nData processed: merging the trees\n")
  ret_merge = os.system(cmd_merge)
  if ret_merge != 0:
    print(f"Error: Merging trees failed with exit code {ret_merge}")
    sys.exit(ret_merge)

  # Cleanup
  try:
    os.remove(os.path.join(cwd, 'dimu.root'))
  except FileNotFoundError:
    pass

  # Remove temporary writer JSON copy
  writer_json_dst = os.path.join(cwd, f'tree-{data_type}.json')
  try:
    os.remove(writer_json_dst)
  except FileNotFoundError:
    pass

if __name__ == '__main__':
  main()
