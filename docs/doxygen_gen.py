#!/usr/bin/env python3

import os, sys
import argparse
from os.path import isdir, join, dirname, abspath

def main(source_dir, build_dir, output_dir):
    source_dirs = [join(source_dir, p) for p in os.listdir(source_dir) if isdir(join(source_dir, p))]
    build_dirs = [join(build_dir, p) for p in os.listdir(build_dir) if isdir(join(build_dir, p)) and p!='CMakeFiles']
    dirs = source_dirs + build_dirs

    script_dir = dirname(abspath(__file__))
    doxyfile_path = join(script_dir, "Doxyfile.in")

    with open(doxyfile_path, 'r') as f:
        doxyfile = f.read()

    input_list = ['']
    for d in dirs:
        input_list += [
            join(d, s) for s in ['include', 'src', 'pybindsrc', 'apps', 'docs', 'python', 'scripts', 'codegen']
        ]

    doxyfile = doxyfile.replace('@INPUT_LIST', ' \\\n'.join(input_list))

    output_doxyfile_path = output_dir+"/Doxyfile"
    with open(output_doxyfile_path, 'w') as f:
        f.write(doxyfile)

    print(f'Doxyfile written to {abspath(output_doxyfile_path)}')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="""
        Script to write input directories to the INPUT field of Doxyfile.in,
        then save as Doxyfile. This allows the command `doxygen` to be run.
        """)
    dbt_area_root = os.getenv('DBT_AREA_ROOT')
    if dbt_area_root is None:
        print("Error: The environment variable DBT_AREA_ROOT is not set. Exiting...")
        sys.exit(1)
    parser.add_argument('--source_dir', type=str, default=join(dbt_area_root, 'sourcecode'), 
                        help='Path to sourcecode directory in a dbt environment')
    parser.add_argument('--build_dir', type=str, default=join(dbt_area_root, 'build'), 
                        help='Path to build directory in a dbt environment')
    parser.add_argument('--output_dir', type=str, default='.', 
                        help='Path where output Doxyfile will be written')
    args = parser.parse_args()

    main(args.source_dir, args.build_dir, args.output_dir)
