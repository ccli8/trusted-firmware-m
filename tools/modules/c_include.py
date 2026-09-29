#!/usr/bin/env python3
#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

import os
import json
import argparse
import sys

import logging
logger = logging.getLogger("TF-M.{}".format(__name__))

def get_compile_command(compile_commands_file, c_file):
    with open(compile_commands_file, "rt") as f:
        compile_commands = json.load(f)

    normalized_c_file = os.path.normcase(os.path.basename(c_file))
    for entry in compile_commands:
        entry_file = entry.get("file", "")
        entry_output = entry.get("output", "")
        if normalized_c_file and (
            normalized_c_file in os.path.normcase(os.path.basename(entry_file)) or
            normalized_c_file in os.path.normcase(os.path.basename(entry_output)) or
            c_file in entry_file or
            c_file in entry_output
        ):
            return entry["command"]

    raise IndexError(f"Could not find compile command for '{c_file}' in {compile_commands_file}")

def get_includes(compile_commands_file, c_file):
    compile_command = get_compile_command(compile_commands_file, c_file).split()
    ret = []
    ret.extend([x[2:].strip() for x in compile_command if x.rstrip()[:2] == "-I"])
    ret.extend([compile_command[i+1] for i, x in enumerate(compile_command[:-1]) if x == "-isystem"])
    return ret

def get_defines(compile_commands_file, c_file):
    compile_command = get_compile_command(compile_commands_file, c_file).split()
    return [x[2:].strip() for x in compile_command if x.rstrip()[:2] == "-D"]

def get_compiler(compile_commands_file, c_file):
    compile_command = get_compile_command(compile_commands_file, c_file).split()
    return compile_command[0].strip()

def main():
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument("--compile_commands_file", help="path to compile_command.json", required=True)
    parser.add_argument("--c_file", help="name of the c file to take", required=True)
    parser.add_argument("--log_level", help="log level", required=False, default="ERROR", choices=logging._levelToName.values())
    args = parser.parse_args()
    logging.getLogger("TF-M").setLevel(args.log_level)
    logger.addHandler(logging.StreamHandler())

    print(get_compile_command(args.compile_commands_file, args.c_file))
    print(get_includes(args.compile_commands_file, args.c_file))
    print(get_defines(args.compile_commands_file, args.c_file))


if __name__ == "__main__":
    sys.exit(main())
