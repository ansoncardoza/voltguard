import argparse
import subprocess
import sys
from pathlib import Path

from src.python.integration.json_bridge import (
    save_combined_report,
    save_combined_decision_report,
)


PROJECT_ROOT = Path(__file__).resolve().parent

GENERATOR_EXE = PROJECT_ROOT / "build-release" / "voltguard_generator.exe"
PARSER_EXE = PROJECT_ROOT / "build-release" / "voltguard_parser.exe"

DATA_DIR = PROJECT_ROOT / "data" / "generated"
REPORT_DIR = PROJECT_ROOT / "reports"

PARSER_REPORT = DATA_DIR / "parser_results.json"

SCENARIOS = {
    "normal": {
        "snapshot": "process_snapshot.json",
        "combined_report": "normal_combined_analysis_report.json",
        "report": "normal_decision_report.json",
    },
    "abnormal": {
        "snapshot": "abnormal_process_snapshot.json",
        "combined_report": "abnormal_combined_analysis_report.json",
        "report": "abnormal_decision_report.json",
    },
}


def run_executable(executable: Path, description: str) -> None:
    """Execute a VoltGuard C++ component and check its exit status."""

    if not executable.is_file():
        raise FileNotFoundError(
            f"{description} executable not found: {executable}"
        )

    print(f"\n[RUNNING] {description}")

    result = subprocess.run(
        [str(executable)],
        cwd=PROJECT_ROOT,
        capture_output=True,
        text=True,
        check=False,
    )

    if result.stdout:
        print(result.stdout)

    if result.stderr:
        print(result.stderr, file=sys.stderr)

    if result.returncode != 0:
        raise RuntimeError(
            f"{description} failed with exit code {result.returncode}."
        )

    print(f"[SUCCESS] {description} completed.")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run the complete VoltGuard offline analysis pipeline."
    )

    parser.add_argument(
        "scenario",
        choices=["normal", "abnormal"],
        help="Select the process simulation scenario.",
    )

    args = parser.parse_args()
    selected = SCENARIOS[args.scenario]

    snapshot_file = DATA_DIR / selected["snapshot"]
    combined_report_file = REPORT_DIR / selected["combined_report"]
    latest_report_file = REPORT_DIR / "combined_analysis_report.json"
    output_file = REPORT_DIR / selected["report"]

    try:
        if not snapshot_file.is_file():
            raise FileNotFoundError(
                f"Process snapshot not found: {snapshot_file}"
            )

        REPORT_DIR.mkdir(parents=True, exist_ok=True)

        print("\n========================================")
        print("       VOLTGUARD ANALYSIS PIPELINE")
        print("========================================")

        # 1. Generate synthetic Modbus/TCP traffic.
        run_executable(
            GENERATOR_EXE,
            "Synthetic Modbus/TCP Traffic Generator",
        )

        # 2. Parse and validate generated frames.
        run_executable(
            PARSER_EXE,
            "C++ Modbus/TCP Parser",
        )

        if not PARSER_REPORT.is_file():
            raise FileNotFoundError(
                f"Parser did not generate its report: {PARSER_REPORT}"
            )

        # 3. Generate combined protocol and physics analysis reports.
        print("\n[RUNNING] Combined Protocol and Physics Analysis")

        save_combined_report(
            PARSER_REPORT,
            snapshot_file,
            combined_report_file,
        )

        save_combined_report(
            PARSER_REPORT,
            snapshot_file,
            latest_report_file,
        )

        print("[SUCCESS] Combined analysis reports generated.")

        # 4. Generate the decision report.
        print("\n[RUNNING] Offline Decision Engine")

        latest_decision_report_file = REPORT_DIR / "combined_decision_report.json"

        report = save_combined_decision_report(
            PARSER_REPORT,
            snapshot_file,
            output_file,
        )

        save_combined_decision_report(
            PARSER_REPORT,
            snapshot_file,
            latest_decision_report_file,
        )

        decision = report["decision_analysis"]["decision"]

        print("[SUCCESS] Combined analysis and decision completed.")

        # 4. Display final result.
        print("\n========================================")
        print("          VOLTGUARD RESULTS")
        print("========================================")
        print(f"Scenario       : {args.scenario.upper()}")
        print(f"Overall status : {report['overall_status']}")
        print(f"Decision       : {decision}")
        print(f"Report saved   : {output_file}")

        print("\nSimulation only.")
        print("Anomaly indicators require investigation.")
        print("They do not independently establish malicious intent.")
        print("No real industrial equipment or network traffic is controlled.")

        return 0

    except (FileNotFoundError, RuntimeError, ValueError, OSError) as error:
        print(f"\n[VOLТGUARD ERROR] {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
