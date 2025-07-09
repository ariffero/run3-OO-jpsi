#!/usr/bin/env python3

import os
import argparse

def main():
  parser = argparse.ArgumentParser(
    description="Download AnalysisResults.root files for given HY job URLs, merge them, and compute luminosity."
  )
  parser.add_argument(
    '-f', '--file',
    default='input.txt',
    help="Input file with the list of jobs from HY (default: %(default)s)"
  )
  parser.add_argument(
    '-d', '--data',
    default='',
    help="Name of the dataset (default: empty)"
  )

  args = parser.parse_args()

  # Read job URLs
  try:
    with open(args.file, 'r') as inputFile:
      lines = inputFile.readlines()
  except FileNotFoundError:
    print(f"Error: File '{args.file}' not found.")
    return

  # Create a temporary directory for download
  tmp_dir = 'this_merge'
  os.makedirs(tmp_dir, exist_ok=True)

  # Download each AnalysisResults.root
  for line in lines:
    line = line.rstrip()
    try:
      hy_id = line.split('/')[8]
    except IndexError:
      print(f"Warning: URL '{line}' is not in expected format, skipping.")
      continue

    print(f"Processing HY ID: {hy_id}")
    dwn_comm = (
      f"alien_cp {line}/AOD/*/AnalysisResults.root "
      f"file://{tmp_dir}/{hy_id}/"
    )
    os.system(dwn_comm)

  # Merge all downloaded ROOT files
  lumi_file = 'AnalysedLumi.root'
  merge_comm = f"hadd -f {lumi_file} {tmp_dir}/*/*/AnalysisResults.root"
  os.system(merge_comm)

  # Rename output based on dataset or input filename
  if args.data:
    out_name = f"AnalysedLumi_{args.data}.root"
  else:
    base = os.path.splitext(os.path.basename(args.file))[0]
    out_name = f"AnalysedLumi_{base}.root"
    print(f"No --data given; using base '{base}'")

  os.rename(lumi_file, out_name)
  print(f"Renamed merged file to: {out_name}")

  # Run the ROOT macro to get luminosity
  lumi_comm = f"root -l -b -q 'getLumi.cpp(\"{out_name}\")'"
  os.system(lumi_comm)

  # Clean up
  os.system(f"rm -rf {tmp_dir}/")

if __name__ == "__main__":
  main()
