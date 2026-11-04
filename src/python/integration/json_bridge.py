import json
from pathlib import Path

from src.python.integration.decision_engine import evaluate_decision

from src.python.physics.operating_rules import (
    validate_operating_conditions,
)


def load_process_snapshot(file_path):
    """Load and validate a VoltGuard JSON process snapshot."""

    path = Path(file_path)

    if not path.is_file():
        raise FileNotFoundError(
            f"Process snapshot not found: {path}"
        )

    with path.open("r", encoding="utf-8") as file:
        snapshot = json.load(file)

    if not isinstance(snapshot, dict):
        raise ValueError("Snapshot must be a JSON object.")

    required_fields = {
        "schema_version",
        "source",
        "environment",
        "transaction_id",
        "unit_id",
        "process_state",
    }

    missing_fields = required_fields - snapshot.keys()

    if missing_fields:
        raise ValueError(
            f"Missing snapshot fields: {sorted(missing_fields)}"
        )

    if not isinstance(snapshot["process_state"], dict):
        raise ValueError("process_state must be a JSON object.")

    process = snapshot["process_state"]

    required_process_fields = {
        "tank_level_percent",
        "pump_on",
        "valve_opening_percent",
        "flow_rate_lpm",
    }

    missing_process_fields = required_process_fields - process.keys()

    if missing_process_fields:
        raise ValueError(
            f"Missing process fields: {sorted(missing_process_fields)}"
        )

    if type(process["pump_on"]) is not bool:
        raise ValueError("pump_on must be true or false.")

    numeric_fields = (
        "tank_level_percent",
        "valve_opening_percent",
        "flow_rate_lpm",
    )

    for field in numeric_fields:
        value = process[field]

        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise ValueError(f"{field} must be numeric.")

    return snapshot


def evaluate_snapshot(file_path):
    """Evaluate process values using VoltGuard operating rules."""

    snapshot = load_process_snapshot(file_path)
    process = snapshot["process_state"]

    result = validate_operating_conditions(
        tank_level_percent=process["tank_level_percent"],
        pump_on=process["pump_on"],
        valve_opening_percent=process["valve_opening_percent"],
        flow_rate_lpm=process["flow_rate_lpm"],
    )

    return {
        "transaction_id": snapshot["transaction_id"],
        "source": snapshot["source"],
        "environment": snapshot["environment"],
        "validation_result": result,
    }



def load_parser_report(file_path):
    """Load and validate the C++ parser JSON report."""

    path = Path(file_path)

    if not path.is_file():
        raise FileNotFoundError(
            f"Parser report not found: {path}"
        )

    with path.open("r", encoding="utf-8") as file:
        report = json.load(file)

    if not isinstance(report, dict):
        raise ValueError("Parser report must be a JSON object.")

    if report.get("schema_version") != "1.0":
        raise ValueError("Unsupported parser report schema version.")

    events = report.get("events")

    if not isinstance(events, list):
        raise ValueError("Parser report events must be a list.")

    if report.get("total_frames") != len(events):
        raise ValueError("Frame count does not match event count.")

    allowed_statuses = {
        "VALID",
        "VALIDATION_FAILED",
        "INVALID_FRAME",
        "MALFORMED_HEX_INPUT",
    }

    for event in events:
        if not isinstance(event, dict):
            raise ValueError("Each parser event must be an object.")

        if event.get("status") not in allowed_statuses:
            raise ValueError("Unknown parser event status.")

        if not isinstance(event.get("parsed"), bool):
            raise ValueError("Parser event parsed field must be boolean.")

        if not isinstance(event.get("function_valid"), bool):
            raise ValueError(
                "Parser event function_valid field must be boolean."
            )

    return report

def evaluate_combined_snapshot(parser_path, snapshot_path):
    """Combine protocol validation and physics validation evidence."""

    parser_report = load_parser_report(parser_path)
    physics_result = evaluate_snapshot(snapshot_path)

    events = parser_report["events"]

    parser_summary = {
        "total_frames": len(events),
        "valid_requests": sum(
            event["status"] == "VALID" for event in events
        ),
        "validation_failures": sum(
            event["status"] == "VALIDATION_FAILED" for event in events
        ),
        "invalid_frames": sum(
            event["status"] == "INVALID_FRAME" for event in events
        ),
        "malformed_inputs": sum(
            event["status"] == "MALFORMED_HEX_INPUT" for event in events
        ),
    }

    parser_clean = all(
        event["status"] == "VALID" for event in events
    )

    physics_valid = (
        physics_result["validation_result"]["valid"]
    )

    overall_status = (
        "NO_CONFIGURED_ANOMALIES"
        if parser_clean and physics_valid
        else "ANOMALY_INDICATORS_FOUND"
    )

    return {
        "analysis_source": "VoltGuard Combined Analysis",
        "environment": "LOCAL_TEST",
        "parser_analysis": {
            "summary": parser_summary,
            "all_requests_valid": parser_clean,
        },
        "physics_analysis": physics_result,
        "overall_status": overall_status,
        "interpretation": (
            "An anomaly indicator is evidence requiring investigation; "
            "it does not establish malicious intent."
        ),
    }


if __name__ == "__main__":
    project_root = Path(__file__).resolve().parents[3]
    snapshot_path = (
        project_root / "data" / "generated" / "process_snapshot.json"
    )

    evaluation = evaluate_snapshot(snapshot_path)

    print("\n========== VOLTGUARD JSON BRIDGE ==========")
    print(json.dumps(evaluation, indent=4))


def save_combined_report(parser_path, snapshot_path, output_path):
    """Generate and save a consolidated VoltGuard JSON report."""

    report = evaluate_combined_snapshot(
        parser_path,
        snapshot_path,
    )

    report["project"] = {
        "name": "VoltGuard",
        "description": (
            "Physics-Aware ICS/SCADA Intrusion Detection System"
        ),
        "report_version": "1.0",
    }

    output = Path(output_path)
    output.parent.mkdir(parents=True, exist_ok=True)

    with output.open("w", encoding="utf-8") as file:
        json.dump(report, file, indent=4)

    return report


def evaluate_combined_decision(parser_path, snapshot_path):
    """Generate combined analysis and an offline simulated decision."""

    report = evaluate_combined_snapshot(
        parser_path,
        snapshot_path,
    )

    report["decision_analysis"] = evaluate_decision(report)

    return report


def save_combined_decision_report(
    parser_path,
    snapshot_path,
    output_path,
):
    """Generate and save a combined report with decision analysis."""

    report = evaluate_combined_decision(
        parser_path,
        snapshot_path,
    )

    report["project"] = {
        "name": "VoltGuard",
        "description": (
            "Physics-Aware ICS/SCADA Intrusion Detection System"
        ),
        "report_version": "1.0",
    }

    output = Path(output_path)
    output.parent.mkdir(parents=True, exist_ok=True)

    with output.open("w", encoding="utf-8") as file:
        json.dump(report, file, indent=4)

    return report
