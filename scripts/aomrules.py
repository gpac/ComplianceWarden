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
import sys

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

print(f"Check srouce file: {SPECS[args.spec]['src_file']}\n")

with open(SPECS[args.spec]["src_file"], "r", encoding="utf-8") as src_f:
    src = src_f.read()

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
