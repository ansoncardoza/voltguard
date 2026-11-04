
"""
VoltGuard - Offline Simulated Decision Engine

Evaluates combined protocol and physics analysis evidence
and produces an explainable ALLOW, HOLD, or DROP decision.

This engine is for local synthetic simulation only.
It does not control real ICS/SCADA equipment.
"""


VALID_DECISIONS = {"ALLOW", "HOLD", "DROP"}


def evaluate_decision(combined_report: dict) -> dict:
    """
    Evaluate a combined VoltGuard analysis report.

    Decision priority:
        1. DROP - invalid protocol frames or physics violations.
        2. HOLD - protocol validation failures or physics warnings.
        3. ALLOW - all configured checks pass.
    """

    if not isinstance(combined_report, dict):
        raise ValueError("Combined report must be a dictionary.")

    parser_analysis = combined_report.get("parser_analysis")
    physics_analysis = combined_report.get("physics_analysis")

    if not isinstance(parser_analysis, dict):
        raise ValueError("Missing or invalid parser_analysis.")

    if not isinstance(physics_analysis, dict):
        raise ValueError("Missing or invalid physics_analysis.")

    parser_summary = parser_analysis.get("summary")
    physics_result = physics_analysis.get("validation_result")

    if not isinstance(parser_summary, dict):
        raise ValueError("Missing or invalid parser summary.")

    if not isinstance(physics_result, dict):
        raise ValueError("Missing or invalid physics validation result.")

    required_counts = (
        "total_frames",
        "valid_requests",
        "validation_failures",
        "invalid_frames",
        "malformed_inputs",
    )

    for field in required_counts:
        value = parser_summary.get(field)

        if type(value) is not int or value < 0:
            raise ValueError(
                f"Parser summary field '{field}' must be a "
                "non-negative integer."
            )

    total = parser_summary["total_frames"]

    classified_total = sum(
        parser_summary[field]
        for field in required_counts
        if field != "total_frames"
    )

    if total != classified_total:
        raise ValueError(
            "Parser event counts do not match total_frames."
        )

    if type(physics_result.get("valid")) is not bool:
        raise ValueError("Physics validation valid field must be boolean.")

    violations = physics_result.get("violations")
    warnings = physics_result.get("warnings")

    if not isinstance(violations, list):
        raise ValueError("Physics violations must be a list.")

    if not isinstance(warnings, list):
        raise ValueError("Physics warnings must be a list.")

    if not all(isinstance(item, str) for item in violations + warnings):
        raise ValueError("Physics findings must contain strings.")

    reasons = []

    invalid_frames = parser_summary["invalid_frames"]
    malformed_inputs = parser_summary["malformed_inputs"]
    validation_failures = parser_summary["validation_failures"]

    # Highest-priority rejection conditions.
    if invalid_frames > 0:
        reasons.append(
            f"{invalid_frames} structurally invalid protocol frame(s) detected."
        )

    if malformed_inputs > 0:
        reasons.append(
            f"{malformed_inputs} malformed hexadecimal input(s) detected."
        )

    if violations:
        reasons.extend(
            f"Physics violation: {item}"
            for item in violations
        )

    if invalid_frames > 0 or malformed_inputs > 0 or violations:
        decision = "DROP"

    # Review-required conditions.
    elif validation_failures > 0 or warnings:
        decision = "HOLD"

        if validation_failures > 0:
            reasons.append(
                f"{validation_failures} protocol validation failure(s) "
                "require review."
            )

        reasons.extend(
            f"Physics warning: {item}"
            for item in warnings
        )

    else:
        decision = "ALLOW"
        reasons.append(
            "All configured protocol and physics checks passed."
        )

    return {
        "engine": "VoltGuard Offline Decision Engine",
        "decision": decision,
        "decision_valid": decision in VALID_DECISIONS,
        "evidence": {
            "total_frames": total,
            "valid_requests": parser_summary["valid_requests"],
            "validation_failures": validation_failures,
            "invalid_frames": invalid_frames,
            "malformed_inputs": malformed_inputs,
            "physics_violations": len(violations),
            "physics_warnings": len(warnings),
            "physics_valid": physics_result["valid"],
        },
        "reasons": reasons,
        "interpretation": (
            "This is a simulated decision based on configured test rules. "
            "A DROP or HOLD outcome is not proof of malicious activity. "
            "No real industrial equipment or network traffic is controlled."
        ),
    }
