import json

from src.python.integration.json_bridge import (
    load_parser_report,
    evaluate_combined_snapshot,
)

import pytest

from src.python.integration.json_bridge import (
    load_process_snapshot,
    evaluate_snapshot,
)


def test_load_normal_snapshot():
    snapshot = load_process_snapshot(
        "data/generated/process_snapshot.json"
    )

    assert snapshot["transaction_id"] == 1001
    assert snapshot["environment"] == "LOCAL_TEST"


def test_normal_snapshot_is_valid():
    result = evaluate_snapshot(
        "data/generated/process_snapshot.json"
    )

    assert result["validation_result"]["valid"] is True
    assert result["validation_result"]["violations"] == []


def test_abnormal_snapshot_is_detected():
    result = evaluate_snapshot(
        "data/generated/abnormal_process_snapshot.json"
    )

    assert result["validation_result"]["valid"] is False

    assert (
        "PUMP OFF BUT FLOW EXCEEDS EXPECTED RANGE"
        in result["validation_result"]["violations"]
    )


def test_missing_snapshot_raises_error():
    with pytest.raises(FileNotFoundError):
        load_process_snapshot("data/generated/missing.json")


def test_load_cpp_parser_report():
    report = load_parser_report(
        "data/generated/parser_results.json"
    )

    assert report["total_frames"] == 9
    assert len(report["events"]) == 9


def test_combined_analysis_with_normal_physics():
    result = evaluate_combined_snapshot(
        "data/generated/parser_results.json",
        "data/generated/process_snapshot.json",
    )

    assert result["parser_analysis"]["summary"]["total_frames"] == 9
    assert result["parser_analysis"]["summary"]["valid_requests"] == 2
    assert result["parser_analysis"]["summary"]["validation_failures"] == 6
    assert result["parser_analysis"]["summary"]["invalid_frames"] == 1

    assert result["physics_analysis"]["validation_result"]["valid"] is True

    assert result["overall_status"] == "ANOMALY_INDICATORS_FOUND"


def test_combined_analysis_with_abnormal_physics():
    result = evaluate_combined_snapshot(
        "data/generated/parser_results.json",
        "data/generated/abnormal_process_snapshot.json",
    )

    assert result["physics_analysis"]["validation_result"]["valid"] is False

    assert result["overall_status"] == "ANOMALY_INDICATORS_FOUND"


def test_parser_report_rejects_incorrect_frame_count(tmp_path):
    invalid_report = {
        "schema_version": "1.0",
        "events": [],
        "total_frames": 5
    }

    report_path = tmp_path / "invalid_report.json"
    report_path.write_text(
        json.dumps(invalid_report),
        encoding="utf-8"
    )

    with pytest.raises(ValueError, match="Frame count"):
        load_parser_report(report_path)

