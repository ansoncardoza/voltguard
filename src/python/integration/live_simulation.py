
"""
VoltGuard - Live Process Simulation Engine

Provides a continuous JSON Lines stream for the Qt dashboard.

SIMULATION ONLY:
No physical industrial equipment is controlled.
All operating values are project simulation assumptions.
"""

import argparse
import json
import sys
import time

from datetime import datetime, timezone

from src.python.physics.tank_model import (
    TankConfig,
    tank_dynamics,
)

from src.python.physics.operating_rules import (
    validate_operating_conditions,
)


SCENARIOS = {
    "normal_filling": {
        "description": "Normal Tank Filling",
        "pump_on": True,
        "inflow_lpm": 40.0,
        "outflow_lpm": 5.0,
        "valve_opening_percent": 80.0,
    },
    "pump_off_consumption": {
        "description": "Pump OFF - Normal Water Consumption",
        "pump_on": False,
        "inflow_lpm": 0.0,
        "outflow_lpm": 2.0,
        "valve_opening_percent": 80.0,
    },
}


def create_sample(
    sequence,
    simulation_time,
    tank_volume,
    capacity,
    scenario_name,
):
    """Build one timestamped live simulation sample."""

    scenario = SCENARIOS[scenario_name]

    tank_level = (tank_volume / capacity) * 100.0

    # The existing operating rule interprets flow_rate_lpm
    # as pump/inlet flow, not normal outlet consumption.
    validation = validate_operating_conditions(
        tank_level_percent=tank_level,
        pump_on=scenario["pump_on"],
        valve_opening_percent=scenario["valve_opening_percent"],
        flow_rate_lpm=scenario["inflow_lpm"],
    )

    return {
        "schema_version": "1.0",
        "source": "VoltGuard Live Simulation Engine",
        "data_source": "SIMULATION",
        "data_status": "LIVE",
        "timestamp": datetime.now(timezone.utc).isoformat(
            timespec="milliseconds"
        ).replace("+00:00", "Z"),
        "sequence": sequence,
        "unit_id": 1,
        "scenario": scenario_name,
        "scenario_description": scenario["description"],
        "simulation_time_seconds": round(simulation_time, 2),
        "process_state": {
            "tank_volume_liters": round(tank_volume, 2),
            "tank_capacity_liters": capacity,
            "tank_level_percent": round(tank_level, 2),
            "pump_on": scenario["pump_on"],
            "valve_opening_percent": scenario[
                "valve_opening_percent"
            ],
            "inflow_lpm": scenario["inflow_lpm"],
            "outflow_lpm": scenario["outflow_lpm"],
            "flow_rate_lpm": scenario["inflow_lpm"],
        },
        "validation": validation,
    }


def run_simulation(args):
    """Run the continuous simulation and emit JSON Lines."""

    tank = TankConfig(
        capacity_liters=1000.0,
        initial_volume_liters=args.initial_level
        * 10.0,
        simulation_duration_minutes=1.0,
    )

    tank_volume = tank.initial_volume_liters
    sequence = 0
    simulation_time = 0.0

    previous_tick = time.monotonic()
    start_time = previous_tick

    while True:
        current_tick = time.monotonic()
        elapsed = current_tick - previous_tick
        previous_tick = current_tick

        if sequence > 0:
            scenario = SCENARIOS[args.scenario]

            rates = tank_dynamics(
                simulation_time,
                [tank_volume],
                scenario["inflow_lpm"],
                scenario["outflow_lpm"],
            )

            # Convert litres/minute into litres/second.
            tank_volume += rates[0] * elapsed / 60.0

            # Keep the simulated tank within its physical capacity.
            tank_volume = max(
                0.0,
                min(tank.capacity_liters, tank_volume),
            )

            simulation_time = current_tick - start_time

        sample = create_sample(
            sequence=sequence,
            simulation_time=simulation_time,
            tank_volume=tank_volume,
            capacity=tank.capacity_liters,
            scenario_name=args.scenario,
        )

        print(json.dumps(sample), flush=True)

        sequence += 1

        if (
            args.duration_seconds > 0
            and simulation_time >= args.duration_seconds
        ):
            break

        time.sleep(args.interval)


def main():
    parser = argparse.ArgumentParser(
        description="VoltGuard continuous live simulation"
    )

    parser.add_argument(
        "--scenario",
        choices=list(SCENARIOS.keys()),
        default="normal_filling",
        help="Select the simulation scenario",
    )

    parser.add_argument(
        "--interval",
        type=float,
        default=1.0,
        help="Seconds between live samples",
    )

    parser.add_argument(
        "--duration-seconds",
        type=float,
        default=0.0,
        help="Run duration; 0 means continuous until stopped",
    )

    parser.add_argument(
        "--initial-level",
        type=float,
        default=70.0,
        help="Initial tank level percentage",
    )

    args = parser.parse_args()

    if args.interval <= 0:
        parser.error("--interval must be greater than zero")

    if args.duration_seconds < 0:
        parser.error("--duration-seconds cannot be negative")

    if not 0 <= args.initial_level <= 100:
        parser.error("--initial-level must be between 0 and 100")

    try:
        run_simulation(args)
    except KeyboardInterrupt:
        print(
            "VoltGuard live simulation stopped.",
            file=sys.stderr,
        )


if __name__ == "__main__":
    main()