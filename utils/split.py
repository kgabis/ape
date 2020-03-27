#!/usr/bin/python

import argparse
import os

def process_input(input, output_path):
    if os.path.exists(output_path) and os.path.isfile(output_path):
        raise Exception(output_path + " is a file")
    if not os.path.exists(output_path):
        os.makedirs(output_path)

    file_path = ""
    file_contents = ""
    for line in input:
        if line.startswith("//FILE_START:"):
            file_name = line.split(":")[1].strip()
            file_path = os.path.join(output_path, file_name)
            file_contents = ""
        elif line == "//FILE_END\n":
            print("Writing: " + file_path)
            with open(file_path, "w") as output:
                output.write(file_contents)
        else:
            file_contents += line

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output-path", required=True)

    args = parser.parse_args()

    with open(args.input, "r") as f:
        process_input(f.readlines(), args.output_path)
        
if __name__ == "__main__":
    main()