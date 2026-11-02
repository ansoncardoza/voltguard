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
