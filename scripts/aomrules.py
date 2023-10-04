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
python aomrules.py

DUMP:
python aomrules.py --dump
"""
import argparse
import urllib.request

from bs4 import BeautifulSoup

SPECS = {
    "av1hdr10plus": {
        "spec_url": "https://aomediacodec.github.io/av1-hdr10plus/",
        "src_file": "../src/specs/av1_hdr10plus/av1_hdr10plus.cpp"
    },
    "av1isobmff": {
        "spec_url": "https://aomediacodec.github.io/av1-isobmff/",
        "src_file": "../src/specs/av1_isobmff/av1_isobmff.cpp"
    }
}

parser = argparse.ArgumentParser(
    description="Check if all rules from the AOM specification are implemented\
        , or dump the C++ stub for the rules based on the specification URL.")

parser.add_argument("--spec",
                    help="Specifications: av1hdr10plus, av1isobmff",
                    required=True)
parser.add_argument("-i", "--input",
                    help="Spec HTML file if you don't want to use URL).")
parser.add_argument("--dump",
                    help="Dump code snippet based on asserts in HTML.",
                    action="store_true")
args = parser.parse_args()

print(f"AOM specification: '{args.spec}'\n")

with open(SPECS[args.spec]["src_file"], "r", encoding="utf-8") as src_f:
    src = src_f.read()

if args.input is not None:
    with open(args.input, "r", encoding="utf-8") as f:
        soup = BeautifulSoup(f, features="lxml")
else:
    html_string = urllib.request.urlopen(SPECS[args.spec]["spec_url"]).read()
    soup = BeautifulSoup(html_string, features="lxml")

assert_spans = soup.find_all("span",
                             {"id": lambda L: L and L.startswith("assert-")})

rules = []
for assert_span in assert_spans:
    rules.append({"id": assert_span.get("id"),
                  "description": assert_span.text.replace('"', ''),
                  "implemented": assert_span.get("id") in src})

duplicates = []
for rule in rules:
    dups = [r for r in rules if rule['id'] == r['id']]
    if len(dups) > 1:
        entry = dups[0]
        entry['dup_cnt'] = len(dups)
        if entry not in duplicates:
            duplicates.append(entry)

if duplicates:
    print(f"WARNING: Found {len(duplicates)} duplicate assert IDs:")
    for dup in duplicates:
        print(dup)
    print(10*'-----' + '\n')

if args.dump:
    for rule in rules:
        print(f'{{\n  "{rule["description"]}",\n  "{rule["id"]}",\n  \
              [](Box const & root, IReport * out)\n  {{\n  }}\n}},')
else:
    impl = [r for r in rules if r["implemented"]]
    nimpl = [r for r in rules if not r["implemented"]]

    print(f"{len(impl)} from {len(rules)} rules implemented.")
    print("List of rules to be implemented:")
    for rule in nimpl:
        print(f'\t"{rule["id"]}": "{rule["description"]}"')
