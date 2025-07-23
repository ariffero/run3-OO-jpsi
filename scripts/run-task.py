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

    args = parser.parse_args()

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
