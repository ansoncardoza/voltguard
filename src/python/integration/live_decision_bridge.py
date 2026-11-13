
"""VoltGuard - Live Offline Decision Bridge.

Combines live synthetic Modbus events and physics validation,
then reuses the existing offline Decision Engine.

SIMULATION ONLY:
No physical ICS/SCADA equipment is controlled.
"""

import json
import sys

from src.python.integration.decision_engine import evaluate_decision


class LiveDecisionBridge:
    """Aggregate live evidence and evaluate the current session."""

    def __init__(self):
        self.reset()

    def reset(self):
        """Reset cumulative evidence for a new simulation session."""

        self.total_frames = 0
        self.valid_requests = 0
        self.validation_failures = 0
        self.invalid_frames = 0
        self.malformed_inputs = 0

    def process_sample(self, sample):
        """Evaluate one live simulation sample."""

        if not isinstance(sample, dict):
            raise ValueError("Live sample must be a JSON object.")

        event = sample.get("modbus")
        validation = sample.get("validation")

        if not isinstance(event, dict):
            raise ValueError("Missing live Modbus event.")

        if not isinstance(validation, dict):
            raise ValueError("Missing physics validation result.")

        if event.get("data_source") != "SIMULATION":
            raise ValueError("Modbus event is not simulated.")

        if event.get("data_status") != "LIVE":
            raise ValueError("Modbus event is not marked LIVE.")

        status = event.get("status")

        if status not in {
            "VALID",
            "VALIDATION_FAILED",
            "INVALID_FRAME",
            "MALFORMED_HEX_INPUT",
        }:
            raise ValueError("Unknown Modbus event status.")

        if type(validation.get("valid")) is not bool:
            raise ValueError("Physics valid field must be boolean.")

        violations = validation.get("violations")
        warnings = validation.get("warnings")

        if not isinstance(violations, list):
            raise ValueError("Physics violations must be a list.")

        if not isinstance(warnings, list):
            raise ValueError("Physics warnings must be a list.")

        if not all(
            isinstance(item, str)
            for item in violations + warnings
        ):
            raise ValueError("Physics findings must be strings.")

        # Accumulate protocol evidence.
        self.total_frames += 1

        if status == "VALID":
            self.valid_requests += 1
        elif status == "VALIDATION_FAILED":
            self.validation_failures += 1
        elif status == "INVALID_FRAME":
            self.invalid_frames += 1
        elif status == "MALFORMED_HEX_INPUT":
            self.malformed_inputs += 1

        combined_report = {
            "parser_analysis": {
                "summary": {
                    "total_frames": self.total_frames,
                    "valid_requests": self.valid_requests,
                    "validation_failures": self.validation_failures,
                    "invalid_frames": self.invalid_frames,
                    "malformed_inputs": self.malformed_inputs,
                }
            },
            "physics_analysis": {
                "validation_result": validation
            },
        }

        decision = evaluate_decision(combined_report)

        return {
            "source": "VoltGuard Live Decision Bridge",
            "data_source": "SIMULATION",
            "data_status": "LIVE",
            "sequence": sample.get("sequence"),
            "scenario": sample.get("scenario"),
            "frame_number": event.get("frame_number"),
            "transaction_id": event.get("transaction_id"),
            "frame_status": status,
            "frame_reason": event.get("reason", ""),
            "decision_analysis": decision,
        }


def main():
    """Read JSON Lines from stdin and emit decision JSON Lines."""

    bridge = LiveDecisionBridge()

    for line in sys.stdin:
        try:
            sample = json.loads(line)

            if sample.get("data_status") == "SESSION_RESET":
                bridge.reset()
                continue

            result = bridge.process_sample(sample)

            print(json.dumps(result), flush=True)

        except (ValueError, TypeError, json.JSONDecodeError) as error:
            print(
                json.dumps({
                    "source": "VoltGuard Live Decision Bridge",
                    "data_source": "SIMULATION",
                    "data_status": "ERROR",
                    "error": str(error),
                }),
                flush=True,
            )


if __name__ == "__main__":
    main()
