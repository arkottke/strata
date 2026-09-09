#!/usr/bin/env python3
"""Exercise batch-mode validation with an invalid persisted model."""

import argparse
import json
import shutil
import subprocess
import tempfile
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("strata_binary")
    parser.add_argument("example_file")
    args = parser.parse_args()

    strata_binary = Path(args.strata_binary).resolve()
    example_file = Path(args.example_file).resolve()

    with tempfile.TemporaryDirectory() as directory:
        invalid_file = Path(directory) / example_file.name
        shutil.copy2(example_file, invalid_file)

        with invalid_file.open() as stream:
            model = json.load(stream)
        model["siteProfile"]["soilLayers"][0]["thickness"] = 0
        with invalid_file.open("w") as stream:
            json.dump(model, stream)

        result = subprocess.run(
            [strata_binary, "-b", str(invalid_file)],
            capture_output=True,
            text=True,
            timeout=60,
        )

    if result.returncode == 0:
        raise SystemExit("Invalid model unexpectedly completed successfully.")
    if "Soil layer 1 thickness must be a finite positive value." not in result.stdout:
        raise SystemExit(
            "Batch output did not report the invalid thickness:\n"
            f"{result.stdout}\n{result.stderr}"
        )


if __name__ == "__main__":
    main()
