import argparse

from bs4 import BeautifulSoup

src_file = "../src/specs/av1_hdr10plus/av1_hdr10plus.cpp"

parser = argparse.ArgumentParser(
    description="Check if all rules from the HDR10+ spec are implemented.")

parser.add_argument("-i", "--input", help="Spec HTML file.", required=True)
parser.add_argument(
    "--dump", help="Dump code snippet based on asserts in HTML.", action="store_true")
args = parser.parse_args()

with open(src_file, "r", encoding="utf-8") as src_f:
    src = src_f.read()

with open(args.input, "r", encoding="utf-8") as f:
    soup = BeautifulSoup(f, features='lxml')

assert_spans = soup.find_all(
    "span", {"id": lambda L: L and L.startswith("assert-")})

rules = []
for assert_span in assert_spans:
    rules.append({"id": assert_span.get("id"),
                 "description": assert_span.text.replace('"', ''), "implemented": assert_span.get("id") in src})

if args.dump:
    for rule in rules:
        print(
            f'{{\n  "{rule["description"]}",\n  "{rule["id"]}",\n  [](Box const & root, IReport * out)\n  {{\n  }}\n}},')
else:
    impl = [r for r in rules if r["implemented"]]
    nimpl = [r for r in rules if not r["implemented"]]

    print(f'{len(impl)} from {len(rules)} rules implemented.')
    print('List of rules to be implemented:')
    for rule in nimpl:
        print(f'\t"{rule["id"]}": "{rule["description"]}"')
