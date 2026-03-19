#!/usr/bin/env python3
"""Compare Strata example output catalogs against reference results.

Usage:
    compare_examples.py <strata_binary> <example_dir> [--rtol=2e-2] [--atol=1e-6]
    compare_examples.py <strata_binary> <example_dir> --update

Workflow for each example-NN.json with existing results:
  1. Copy the file to a temporary directory
  2. Run strata in batch mode on the copy
  3. Compare the outputCatalog from the result against the original

Use --update to regenerate reference results in-place.
"""

import argparse
import glob
import json
import os
import shutil
import subprocess
import sys
import tempfile


def compare_values(ref, actual, path, rtol, atol, errors):
    """Recursively compare two JSON values, collecting mismatches."""
    if isinstance(ref, dict) and isinstance(actual, dict):
        for key in ref:
            if key == "log":
                # Skip log text — messages vary between runs
                continue
            if key not in actual:
                errors.append(f"{path}.{key}: missing in actual output")
                continue
            compare_values(ref[key], actual[key], f"{path}.{key}", rtol, atol, errors)
        return

    if isinstance(ref, list) and isinstance(actual, list):
        if len(ref) != len(actual):
            errors.append(
                f"{path}: list length mismatch: expected {len(ref)}, got {len(actual)}"
            )
            return
        for i, (r, a) in enumerate(zip(ref, actual)):
            compare_values(r, a, f"{path}[{i}]", rtol, atol, errors)
        return

    if isinstance(ref, (int, float)) and isinstance(actual, (int, float)):
        if ref == actual:
            return
        abs_err = abs(ref - actual)
        if abs_err <= atol:
            return
        denom = max(abs(ref), abs(actual), 1e-15)
        rel_err = abs_err / denom
        if rel_err > rtol:
            errors.append(
                f"{path}: value mismatch: expected {ref}, got {actual} "
                f"(rel_err={rel_err:.2e}, abs_err={abs_err:.2e})"
            )
        return

    if isinstance(ref, str) and isinstance(actual, str):
        if ref != actual:
            errors.append(f"{path}: string mismatch: expected {ref!r}, got {actual!r}")
        return

    if isinstance(ref, bool) and isinstance(actual, bool):
        if ref != actual:
            errors.append(f"{path}: bool mismatch: expected {ref}, got {actual}")
        return

    if ref is None and actual is None:
        return

    if type(ref) != type(actual):
        errors.append(
            f"{path}: type mismatch: expected {type(ref).__name__}, "
            f"got {type(actual).__name__}"
        )


def extract_output_data(output_catalog):
    """Extract only the result data fields from the outputCatalog for comparison."""
    data = {}
    for catalog_key in [
        "profilesOutputCatalog",
        "ratiosOutputCatalog",
        "soilTypesOutputCatalog",
        "spectraOutputCatalog",
        "timeSeriesOutputCatalog",
    ]:
        entries = output_catalog.get(catalog_key, [])
        catalog_data = []
        for entry in entries:
            if entry.get("enabled") and entry.get("data"):
                catalog_data.append(
                    {
                        "className": entry.get("className"),
                        "data": entry["data"],
                    }
                )
        if catalog_data:
            data[catalog_key] = catalog_data
    return data


def run_example(strata_bin, example_path, rtol, atol, update=False):
    """Run strata on an example file and compare results. Returns (name, errors, skipped)."""
    name = os.path.basename(example_path)

    # Load reference
    with open(example_path) as f:
        ref_doc = json.load(f)

    if not update and not ref_doc.get("hasResults", False):
        return name, [], True  # skip

    ref_output = extract_output_data(ref_doc.get("outputCatalog", {}))
    if not update and not ref_output:
        return name, ["No enabled output data in reference file"], False

    # Copy to temp directory and run
    with tempfile.TemporaryDirectory() as tmpdir:
        tmp_file = os.path.join(tmpdir, name)
        shutil.copy2(example_path, tmp_file)

        # Also copy any motion files referenced (same directory)
        example_dir = os.path.dirname(example_path)
        motions_dir = os.path.join(example_dir, "motions")
        if os.path.isdir(motions_dir):
            shutil.copytree(motions_dir, os.path.join(tmpdir, "motions"))

        # Run strata in batch mode
        try:
            result = subprocess.run(
                [strata_bin, "-b", tmp_file],
                capture_output=True,
                text=True,
                timeout=300,
            )
        except subprocess.TimeoutExpired:
            return name, ["Strata timed out after 300 seconds"], False

        if result.returncode != 0:
            return (
                name,
                [f"Strata exited with code {result.returncode}: {result.stderr}"],
                False,
            )

        # Load results
        with open(tmp_file) as f:
            actual_doc = json.load(f)

        if not actual_doc.get("hasResults", False):
            return name, ["Strata did not produce results"], False

        if update:
            # Overwrite the original file with new results
            shutil.copy2(tmp_file, example_path)
            return name, [], False

        actual_output = extract_output_data(actual_doc.get("outputCatalog", {}))

        # Compare
        errors = []
        compare_values(ref_output, actual_output, "outputCatalog", rtol, atol, errors)
        return name, errors, False


def main():
    parser = argparse.ArgumentParser(
        description="Run Strata examples and compare output catalogs"
    )
    parser.add_argument("strata_binary", help="Path to the strata executable")
    parser.add_argument("example_dir", help="Path to the example directory")
    parser.add_argument(
        "--rtol",
        type=float,
        default=2e-2,
        help="Relative tolerance for numerical comparison (default: 2e-2)",
    )
    parser.add_argument(
        "--atol",
        type=float,
        default=1e-6,
        help="Absolute tolerance for numerical comparison (default: 1e-6)",
    )
    parser.add_argument(
        "--update",
        action="store_true",
        help="Regenerate reference results by running strata and saving output",
    )
    args = parser.parse_args()

    strata_bin = os.path.abspath(args.strata_binary)
    example_dir = os.path.abspath(args.example_dir)

    if not os.path.isfile(strata_bin):
        print(f"ERROR: Strata binary not found: {strata_bin}", file=sys.stderr)
        sys.exit(1)

    # Find example files (example-NN.json pattern, excluding -test variants)
    pattern = os.path.join(example_dir, "example-[0-9][0-9].json")
    example_files = sorted(glob.glob(pattern))

    if not example_files:
        print(f"ERROR: No example files found matching {pattern}", file=sys.stderr)
        sys.exit(1)

    print(f"Found {len(example_files)} example file(s)")
    if args.update:
        print("Mode: updating reference results")
    else:
        print(f"Tolerances: rtol={args.rtol}, atol={args.atol}")
    print()

    failed = []
    passed = []
    skipped = []

    for example_path in example_files:
        name, errors, was_skipped = run_example(
            strata_bin, example_path, args.rtol, args.atol, update=args.update
        )
        if was_skipped:
            skipped.append(name)
            print(f"  SKIP: {name} (no reference results)")
        elif args.update:
            if errors:
                failed.append((name, errors))
                print(f"  FAIL: {name}")
                for err in errors:
                    print(f"        {err}")
            else:
                passed.append(name)
                print(f"  UPDATED: {name}")
        elif errors:
            failed.append((name, errors))
            print(f"  FAIL: {name}")
            for err in errors[:10]:
                print(f"        {err}")
            if len(errors) > 10:
                print(f"        ... and {len(errors) - 10} more errors")
        else:
            passed.append(name)
            print(f"  PASS: {name}")

    print()
    print(f"Results: {len(passed)} passed, {len(failed)} failed, {len(skipped)} skipped")

    if failed:
        sys.exit(1)


if __name__ == "__main__":
    main()
