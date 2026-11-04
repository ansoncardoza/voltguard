from src.python.integration.benchmark_engine import (
    measure_operation,
    run_benchmarks,
)


def test_measure_operation_counts_warmups_and_iterations():
    calls = []

    def operation():
        calls.append(1)

    result = measure_operation(
        operation,
        iterations=3,
        warmup_runs=2,
    )

    assert len(calls) == 5
    assert result["iterations"] == 3


def test_measure_operation_returns_timing_statistics():
    result = measure_operation(
        lambda: sum(range(100)),
        iterations=3,
        warmup_runs=1,
    )

    assert result["average_ms"] >= 0
    assert result["minimum_ms"] >= 0
    assert result["maximum_ms"] >= result["minimum_ms"]
    assert result["median_ms"] >= 0
    assert "throughput_operations_per_second" in result


def test_run_benchmarks_rejects_zero_iterations():
    try:
        run_benchmarks(iterations=0)
        assert False, "Expected ValueError for zero iterations"
    except ValueError:
        pass


def test_run_benchmarks_rejects_negative_warmups():
    try:
        run_benchmarks(iterations=1, warmup_runs=-1)
        assert False, "Expected ValueError for negative warm-up runs"
    except ValueError:
        pass
