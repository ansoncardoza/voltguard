
"""
VoltGuard - Physics-Aware Operating Rules

Validates simulated tank, pump, valve and flow conditions.

All thresholds are provisional project test values,
not certified industrial operating limits.
"""


def validate_operating_conditions(
    tank_level_percent: float,
    pump_on: bool,
    valve_opening_percent: float,
    flow_rate_lpm: float,
) -> dict:
    """Validate a simulated industrial operating condition."""

    violations = []
    warnings = []

    # 1. Tank level validation
    if not 0 <= tank_level_percent <= 100:
        violations.append("TANK LEVEL OUTSIDE PHYSICAL RANGE")
    elif tank_level_percent < 10:
        warnings.append("TANK LEVEL CRITICALLY LOW")
    elif tank_level_percent > 90:
        warnings.append("TANK LEVEL CRITICALLY HIGH")

    # 2. Valve position validation
    if not 0 <= valve_opening_percent <= 100:
        violations.append("INVALID VALVE OPENING PERCENTAGE")

    # 3. Flow rate validation
    if not 0 <= flow_rate_lpm <= 60:
        violations.append("FLOW RATE OUTSIDE CONFIGURED RANGE")

    # 4. Pump and flow consistency
    if pump_on and valve_opening_percent > 0:
        if not 20 <= flow_rate_lpm <= 60:
            violations.append(
                "PUMP ON WITH OPEN VALVE: FLOW OUTSIDE EXPECTED RANGE"
            )

    elif not pump_on:
        if not 0 <= flow_rate_lpm <= 5:
            violations.append(
                "PUMP OFF BUT FLOW EXCEEDS EXPECTED RANGE"
            )

    return {
        "valid": len(violations) == 0,
        "violations": violations,
        "warnings": warnings,
    }


if __name__ == "__main__":

    test_cases = [
        {
            "name": "Normal Operating Condition",
            "tank_level_percent": 70,
            "pump_on": True,
            "valve_opening_percent": 80,
            "flow_rate_lpm": 40,
        },
        {
            "name": "Pump OFF But Flow Detected",
            "tank_level_percent": 60,
            "pump_on": False,
            "valve_opening_percent": 80,
            "flow_rate_lpm": 40,
        },
        {
            "name": "Invalid Valve Opening",
            "tank_level_percent": 60,
            "pump_on": True,
            "valve_opening_percent": 120,
            "flow_rate_lpm": 40,
        },
        {
            "name": "Excessive Flow Rate",
            "tank_level_percent": 60,
            "pump_on": True,
            "valve_opening_percent": 80,
            "flow_rate_lpm": 75,
        },
        {
            "name": "Tank Level Above Capacity",
            "tank_level_percent": 110,
            "pump_on": True,
            "valve_opening_percent": 80,
            "flow_rate_lpm": 40,
        },
    ]

    print("=" * 50)
    print("   VOLTGUARD PHYSICS VALIDATION TEST LAB")
    print("=" * 50)

    for test in test_cases:
        name = test["name"]

        inputs = {
            key: value
            for key, value in test.items()
            if key != "name"
        }

        result = validate_operating_conditions(**inputs)

        print(f"\nTEST: {name}")
        print("Status:", "VALID" if result["valid"] else "ANOMALY DETECTED")

        for violation in result["violations"]:
            print("Violation:", violation)

        for warning in result["warnings"]:
            print("Warning:", warning)

