
import pytest

from src.python.physics.operating_rules import (
    validate_operating_conditions,
)


def test_normal_operating_condition():
    result = validate_operating_conditions(
        tank_level_percent=70,
        pump_on=True,
        valve_opening_percent=80,
        flow_rate_lpm=40,
    )

    assert result["valid"] is True
    assert result["violations"] == []


@pytest.mark.parametrize(
    "inputs, expected_violation",
    [
        (
            {
                "tank_level_percent": 60,
                "pump_on": False,
                "valve_opening_percent": 80,
                "flow_rate_lpm": 40,
            },
            "PUMP OFF BUT FLOW EXCEEDS EXPECTED RANGE",
        ),
        (
            {
                "tank_level_percent": 60,
                "pump_on": True,
                "valve_opening_percent": 120,
                "flow_rate_lpm": 40,
            },
            "INVALID VALVE OPENING PERCENTAGE",
        ),
        (
            {
                "tank_level_percent": 60,
                "pump_on": True,
                "valve_opening_percent": 80,
                "flow_rate_lpm": 75,
            },
            "FLOW RATE OUTSIDE CONFIGURED RANGE",
        ),
        (
            {
                "tank_level_percent": 110,
                "pump_on": True,
                "valve_opening_percent": 80,
                "flow_rate_lpm": 40,
            },
            "TANK LEVEL OUTSIDE PHYSICAL RANGE",
        ),
    ],
)
def test_abnormal_operating_conditions(inputs, expected_violation):
    result = validate_operating_conditions(**inputs)

    assert result["valid"] is False
    assert expected_violation in result["violations"]
