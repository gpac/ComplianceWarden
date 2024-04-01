#!/usr/bin/python3
 
"""
Python script to check if all spec rules (assert-ids) are implemented,
or to dump assert-ids when a new specification checker is created.

INSTALL:
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

USAGE:
make sure to run the script from the scripts directory

CHECK:
python aomrules.py --spec [specname]

DUMP:
python aomrules.py --spec [specname] --dump
"""

import argparse
import urllib.request
import sys
import re
import json

from bs4 import BeautifulSoup


def is_url(input_string):
    return input_string.startswith('http://') or input_string.startswith('https://')


def read_input(input_source):
    try:
        if is_url(input_source):
            with urllib.request.urlopen(input_source) as response:
                return response.read()
        else:
            with open(input_source, "r", encoding="utf-8") as file:
                return file.read()
    except Exception as e:
        print(f"Error reading input: {e}")
        return None


def extract_assert_ids(src):
    # 'assert-' followed by exactly 8 alphanumeric characters
    pattern = r'assert-[a-zA-Z0-9]{8}'
    matches = re.findall(pattern, src)
    return matches


SPECS = {
    "av1hdr10plus": {
        "spec_url": "https://aomediacodec.github.io/av1-hdr10plus/",
        "src_file": "../src/specs/av1_hdr10plus/av1_hdr10plus.cpp"
    },
    "av1isobmff": {
        "spec_url": "https://aomediacodec.github.io/av1-isobmff/",
        "src_file": "../src/specs/av1_isobmff/av1_isobmff.cpp"
    },
    "avif": {
        "spec_url": "https://aomediacodec.github.io/av1-avif/",
        "src_file": "../src/specs/avif/avif.cpp"
    }
}

parser = argparse.ArgumentParser(
    description="Check if all rules from the AOM specification are implemented\
        , or dump the C++ stub for the rules based on the specification URL.")

parser.add_argument("--spec",
                    help="Specifications: av1hdr10plus, av1isobmff, avif",
                    required=True)
parser.add_argument("-i", "--input",
                    help="Path to the HTML file (can be a URL or a file path).")
parser.add_argument("--dump",
                    help="Dump code snippet based on asserts in HTML.",
                    action="store_true")
args = parser.parse_args()

if args.input is not None and args.spec is not None:
    print(f"NOTE: -i option overrides the defualt URL from the --spec option. Run analysis on {args.input}\n")

input_source = args.input if args.input is not None else SPECS[args.spec]["spec_url"]
content = read_input(input_source)
if content is not None:
    soup = BeautifulSoup(content, features="lxml")
else:
    print("Failed to read input content.")
    sys.exit(1)

# Find the <h2> element with the specified id and get text if found
subtitle_element = soup.find('h2', {'id': 'subtitle'})
if subtitle_element:
    subtitle_text = subtitle_element.get_text(strip=True)
    print(f"AOM specification: {input_source} ({subtitle_text})")
else:
    print(f"AOM specification: {input_source} (no version found)")

print(f"Check source file: {SPECS[args.spec]['src_file']}\n")

with open(SPECS[args.spec]["src_file"], "r", encoding="utf-8") as src_f:
    src = src_f.read()

assert_spans = soup.find_all("span",
                             {"id": lambda L: L and L.startswith("assert-")})

implemented_assert_ids = extract_assert_ids(src)

spec_rules = []
for assert_span in assert_spans:
    spec_rules.append({"id": assert_span.get("id"),
                  "description": assert_span.text.replace('"', ''),
                  "implemented": assert_span.get("id") in src})

# Get duplicate rules from the spec so we can report them.
# This should be a part of AOM spec publication automation but it does not harm to verify just in case.
duplicates = []
for rule in spec_rules:
    dups = [r for r in spec_rules if rule['id'] == r['id']]
    if len(dups) > 1:
        entry = dups[0]
        entry['dup_cnt'] = len(dups)
        if entry not in duplicates:
            duplicates.append(entry)

# Get rules that are in the source file but not in the spec
implemented_but_not_in_spec = []
for rule_src in implemented_assert_ids:
    found_rule = next((rule for rule in spec_rules if rule["id"] == rule_src), None)
    if found_rule is None:
        implemented_but_not_in_spec.append(rule_src)

if args.dump:
    for rule in spec_rules:
        print(f'{{\n  "{rule["description"]}",\n  "{rule["id"]}",\n  \
              [](Box const & root, IReport * out)\n  {{\n  }}\n}},')
else:
    impl = [r for r in spec_rules if r["implemented"]]
    nimpl = [r for r in spec_rules if not r["implemented"]]

    # provide an overvew at the end
    print(10*'-----')
    print(" SUMMARY")
    print(10*'-----')
    print(f"{len(impl)} from {len(spec_rules)} rules implemented.")
    if len(nimpl) > 0:
        todo_json_file = args.spec + '_todos.json'

        out_file = open(todo_json_file, "w")
        json.dump(nimpl, out_file, indent=4)
        print(f"{len(nimpl)} rules are not implemented yet. Take a look at {todo_json_file}")
    if duplicates:
        print(f"WARNING: Found {len(duplicates)} duplicate assert IDs:")
        for rule in duplicates:
            print(f' - {rule}')
        print(10*'-----' + '\n')
    if implemented_but_not_in_spec:
        print(f"WARNING: Found {len(implemented_but_not_in_spec)} assert IDs in the source file that could not be mapped to the spec.:")
        for rule in implemented_but_not_in_spec:
            print(f' - {rule}')
        print(10*'-----' + '\n')
