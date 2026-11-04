
import pytest

from src.python.integration.decision_engine import evaluate_decision


def make_report(
    validation_failures=0,
    invalid_frames=0,
    malformed_inputs=0,
    violations=None,
    warnings=None,
):
    """Create a synthetic combined report for decision testing."""

    violations = violations or []
    warnings = warnings or []

    total = 2 + validation_failures + invalid_frames + malformed_inputs

    return {
        "parser_analysis": {
            "summary": {
                "total_frames": total,
                "valid_requests": 2,
                "validation_failures": validation_failures,
                "invalid_frames": invalid_frames,
                "malformed_inputs": malformed_inputs,
            }
        },
        "physics_analysis": {
            "validation_result": {
                "valid": len(violations) == 0,
                "violations": violations,
                "warnings": warnings,
            }
        },
    }


def test_allow_when_all_checks_pass():
    report = make_report()

    result = evaluate_decision(report)

    assert result["decision"] == "ALLOW"
    assert result["evidence"]["physics_valid"] is True


def test_hold_when_protocol_validation_fails():
    report = make_report(validation_failures=1)

    result = evaluate_decision(report)

    assert result["decision"] == "HOLD"


def test_hold_when_physics_warning_exists():
    report = make_report(
        warnings=["TANK LEVEL CRITICALLY HIGH"]
    )

    result = evaluate_decision(report)

    assert result["decision"] == "HOLD"


def test_drop_when_invalid_frame_exists():
    report = make_report(invalid_frames=1)

    result = evaluate_decision(report)

    assert result["decision"] == "DROP"


def test_drop_when_malformed_input_exists():
    report = make_report(malformed_inputs=1)

    result = evaluate_decision(report)

    assert result["decision"] == "DROP"


def test_drop_when_physics_violation_exists():
    report = make_report(
        violations=["PUMP OFF BUT FLOW EXCEEDS EXPECTED RANGE"]
    )

    result = evaluate_decision(report)

    assert result["decision"] == "DROP"


def test_drop_has_priority_over_hold():
    report = make_report(
        validation_failures=1,
        invalid_frames=1,
    )

    result = evaluate_decision(report)

    assert result["decision"] == "DROP"


def test_invalid_report_type_is_rejected():
    with pytest.raises(ValueError):
        evaluate_decision([])


def test_missing_parser_analysis_is_rejected():
    with pytest.raises(ValueError):
        evaluate_decision({})


def test_inconsistent_frame_counts_are_rejected():
    report = make_report()
    report["parser_analysis"]["summary"]["total_frames"] = 10

    with pytest.raises(ValueError):
        evaluate_decision(report)
