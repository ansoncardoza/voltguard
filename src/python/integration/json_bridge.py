
import json
from pathlib import Path

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


if __name__ == "__main__":
    project_root = Path(__file__).resolve().parents[3]
    snapshot_path = (
        project_root / "data" / "generated" / "process_snapshot.json"
    )

    evaluation = evaluate_snapshot(snapshot_path)

    print("\n========== VOLTGUARD JSON BRIDGE ==========")
    print(json.dumps(evaluation, indent=4))
