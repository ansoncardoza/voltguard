"""
VoltGuard - Performance Benchmark Engine

Measures the execution time of the synthetic parser,
physics engine, combined analysis, and decision engine.

All benchmarks operate on local synthetic test data.
"""

import argparse
import json
import platform
import statistics
import subprocess
import sys
import time
from pathlib import Path

from src.python.integration.decision_engine import evaluate_decision
from src.python.integration.json_bridge import (
    evaluate_combined_decision,
    evaluate_combined_snapshot,
)
from src.python.physics.operating_rules import (
    validate_operating_conditions,
)
from src.python.physics.tank_model import TankConfig, simulate_tank


def measure_operation(operation, iterations, warmup_runs):
    """Measure an operation using a high-resolution timer."""

    for _ in range(warmup_runs):
        operation()

    timings = []

    for _ in range(iterations):
        start = time.perf_counter()
        operation()
        elapsed = time.perf_counter() - start
        timings.append(elapsed * 1000)

    average_ms = statistics.mean(timings)

    return {
        "iterations": iterations,
        "average_ms": round(average_ms, 4),
        "minimum_ms": round(min(timings), 4),
        "maximum_ms": round(max(timings), 4),
        "median_ms": round(statistics.median(timings), 4),
        "throughput_operations_per_second": round(
            1000 / average_ms, 2
        ) if average_ms > 0 else None,
    }


def run_parser(parser_executable, project_root):
    """Execute the compiled C++ parser as a separate process."""

    result = subprocess.run(
        [str(parser_executable)],
        cwd=project_root,
        capture_output=True,
        text=True,
        check=False,
    )

    if result.returncode != 0:
        raise RuntimeError(
            "C++ parser execution failed: " + result.stderr
        )


def run_benchmarks(
    project_root=None,
    iterations=10,
    warmup_runs=1,
    output_path=None,
):
    """Run all VoltGuard performance benchmarks."""

    if iterations < 1:
        raise ValueError("Iterations must be at least 1.")

    if warmup_runs < 0:
        raise ValueError("Warm-up runs cannot be negative.")

    if project_root is None:
        project_root = Path(__file__).resolve().parents[3]
    else:
        project_root = Path(project_root).resolve()

    parser_executable = (
        project_root / "build-release" / "voltguard_parser.exe"
    )

    parser_input = (
        project_root / "data" / "generated" / "modbus_traffic.hex"
    )

    parser_report = (
        project_root / "data" / "generated" / "parser_results.json"
    )

    snapshot_path = (
        project_root / "data" / "generated" / "process_snapshot.json"
    )

    required_files = [
        parser_executable,
        parser_input,
        snapshot_path,
    ]

    for file_path in required_files:
        if not file_path.is_file():
            raise FileNotFoundError(
                f"Required benchmark file not found: {file_path}"
            )

    results = {}

    print("VoltGuard Performance Benchmark")
    print("=" * 45)
    print(f"Iterations: {iterations}")
    print(f"Warm-up runs: {warmup_runs}")

    # 1. C++ Parser
    print("\n[1/6] Benchmarking C++ parser...")

    results["cpp_parser"] = measure_operation(
        lambda: run_parser(parser_executable, project_root),
        iterations,
        warmup_runs,
    )

    with parser_report.open("r", encoding="utf-8") as file:
        parser_data = json.load(file)

    total_frames = parser_data.get("total_frames", 0)

    if total_frames < 1:
        raise ValueError("Parser report contains no processed frames.")

    average_seconds = (
        results["cpp_parser"]["average_ms"] / 1000
    )

    results["cpp_parser"]["frames_processed_per_run"] = total_frames
    results["cpp_parser"]["frames_per_second"] = round(
        total_frames / average_seconds, 2
    ) if average_seconds > 0 else None

    # 2. Tank Physics
    print("[2/6] Benchmarking tank physics...")

    tank_config = TankConfig()

    results["tank_physics"] = measure_operation(
        lambda: simulate_tank(
            tank_config,
            inflow_lpm=40,
            outflow_lpm=20,
        ),
        iterations,
        warmup_runs,
    )

    # 3. Operating Rules
    print("[3/6] Benchmarking operating rules...")

    results["operating_rules"] = measure_operation(
        lambda: validate_operating_conditions(
            tank_level_percent=70,
            pump_on=True,
            valve_opening_percent=80,
            flow_rate_lpm=40,
        ),
        iterations,
        warmup_runs,
    )

    # 4. Combined Analysis
    print("[4/6] Benchmarking combined analysis...")

    combined_report = evaluate_combined_snapshot(
        parser_report,
        snapshot_path,
    )

    results["combined_analysis"] = measure_operation(
        lambda: evaluate_combined_snapshot(
            parser_report,
            snapshot_path,
        ),
        iterations,
        warmup_runs,
    )

    # 5. Standalone Decision Engine
    print("[5/6] Benchmarking decision engine...")

    results["decision_engine"] = measure_operation(
        lambda: evaluate_decision(combined_report),
        iterations,
        warmup_runs,
    )

    # 6. Complete Combined Decision Workflow
    print("[6/6] Benchmarking complete decision workflow...")

    results["combined_decision_workflow"] = measure_operation(
        lambda: evaluate_combined_decision(
            parser_report,
            snapshot_path,
        ),
        iterations,
        warmup_runs,
    )

    report = {
        "project": "VoltGuard",
        "benchmark_type": "LOCAL_SYNTHETIC_PERFORMANCE_TEST",
        "environment": {
            "operating_system": platform.platform(),
            "python_version": sys.version.split()[0],
            "processor": platform.processor(),
            "parser_executable": str(parser_executable),
        },
        "configuration": {
            "iterations": iterations,
            "warmup_runs": warmup_runs,
            "timer": "time.perf_counter",
            "parser_measurement": (
                "End-to-end process execution including file I/O"
            ),
        },
        "input_data": {
            "parser_input": str(parser_input),
            "parser_frames": total_frames,
            "process_snapshot": str(snapshot_path),
        },
        "benchmark_results": results,
        "interpretation": (
            "Measurements describe local synthetic test execution. "
            "They are not certified industrial performance guarantees."
        ),
    }

    if output_path is None:
        output_path = (
            project_root
            / "reports"
            / "performance_benchmark_report.json"
        )
    else:
        output_path = Path(output_path)

        if not output_path.is_absolute():
            output_path = project_root / output_path

    output_path.parent.mkdir(parents=True, exist_ok=True)

    with output_path.open("w", encoding="utf-8") as file:
        json.dump(report, file, indent=4)

    print("\n" + "=" * 45)
    print("BENCHMARK RESULTS")

    for name, metrics in results.items():
        print(
            f"{name}: "
            f"Average {metrics['average_ms']} ms"
        )

    print(f"\nReport saved to: {output_path}")

    return report


def main():
    parser = argparse.ArgumentParser(
        description="VoltGuard Performance Benchmark"
    )

    parser.add_argument(
        "--iterations",
        type=int,
        default=10,
        help="Number of measurement iterations.",
    )

    parser.add_argument(
        "--warmup",
        type=int,
        default=1,
        help="Number of warm-up runs.",
    )

    parser.add_argument(
        "--output",
        default=None,
        help="Optional JSON report output path.",
    )

    args = parser.parse_args()

    run_benchmarks(
        iterations=args.iterations,
        warmup_runs=args.warmup,
        output_path=args.output,
    )


if __name__ == "__main__":
    main()