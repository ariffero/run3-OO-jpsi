## Scripts
Scripts used to automate operations. All the scripts comes with instructions, just run them with the option `--help`.

### Content
1. **write-content.py** writes in a txt file all the path of the files contained in a chosen directory.
2. **run-task** is used to run a task in O2Physics, usually the analysis task. It writes output table into trees and it merges the file such that it contains only one DF. To perform the saving of the trees and the merging it needs some input files, that can be found under `~/Desktop/run3-OO-jpsi/utilities/`.
3. **run-parameter-scan.py** automates parameter scans for O2Physics analysis by running a task multiple times with different config values, organizing outputs and mapping results to parameter sets. It needs a config file that tells the starting config file, and the parameters to scan (+ some other info). An example of this file can be found under `~/Desktop/run3-OO-jpsi/utilities/scan_example.json`.