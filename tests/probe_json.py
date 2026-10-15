"""Check the CLI report contract when ISOBMFF probing fails."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile


binary = Path(sys.argv[1]).resolve() / "cw.exe"
repo = Path(__file__).resolve().parent.parent


def check_failure(path, spec="iamf", dependencies=None):
    result = subprocess.run(
        [str(binary), "-s", spec, "-f", "json", str(path)],
        capture_output=True,
        text=True,
        check=False,
    )
    assert result.returncode == 1, result
    report = json.loads(result.stdout)
    assert report["cw_version"]
    assert report["input_file"] == path.name, report
    assert report["specification"] == spec, report
    assert report["spec_name"], report
    assert report["dependencies"] == (dependencies or []), report
    assert report["validation"] == [], report
    assert len(report["errors"]) == 1, report
    assert report["errors"][0]["stage"] == "probe", report
    assert report["errors"][0]["description"], report
    return report["errors"][0]["description"]


assert "first box size too big" in check_failure(repo / "LICENSE")
check_failure(repo / "LICENSE", "heif", ["isobmff"])

with tempfile.TemporaryDirectory(prefix="cw-probe-") as directory:
    cases = [
        (b"", "not enough bytes"),
        (b"short", "not enough bytes"),
        (b"\x00\x00\x00\x04ftyp", "first box size too small"),
        (b"\x00\x00\x00\x10ftyp", "first box size too big"),
        (b"\x00\x00\x00\x08\x00abc", "Box type is neither"),
    ]
    for data, expected in cases:
        path = Path(directory) / 'quoted"file\\name.mp4'
        path.write_bytes(data)
        assert expected in check_failure(path)

result = subprocess.run(
    [str(binary), "-s", "iamf", "-f", "text", str(repo / "LICENSE")],
    capture_output=True,
    text=True,
    check=False,
)
assert result.returncode == 1, result
assert not result.stdout, result
assert "Failed to probe ISOBMFF" in result.stderr, result

# A successful probe still reaches the normal compliance report.
result = subprocess.run(
    [str(binary), "-s", "heif", "-f", "json", str(repo / "tests" / "aac.mp4")],
    capture_output=True,
    text=True,
    check=False,
)
report = json.loads(result.stdout)
assert "errors" not in report, report
assert report["validation"], report
