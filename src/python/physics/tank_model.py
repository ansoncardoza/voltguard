
"""
VoltGuard - Water Tank Physics Model

Purpose:
Simulate water-level changes using inflow and outflow
and basic physical operating constraints.

Simulation only. No real industrial equipment is controlled.
"""

from dataclasses import dataclass

from scipy.integrate import solve_ivp


@dataclass
class TankConfig:
    """Configuration of the simulated water tank."""

    capacity_liters: float = 1000.0
    initial_volume_liters: float = 500.0
    simulation_duration_minutes: float = 10.0


def tank_dynamics(time: float, state: list[float],
                  inflow: float, outflow: float) -> list[float]:
    """
    Calculate the rate of change of water volume.

    dV/dt = Qin - Qout

    Flow units: litres/minute
    Volume units: litres
    """

    volume = state[0]

    # Flow balance
    volume_change = inflow - outflow

    return [volume_change]


def simulate_tank(
    config: TankConfig,
    inflow_lpm: float,
    outflow_lpm: float,
) -> dict:
    """Run the tank simulation."""

    if config.capacity_liters <= 0:
        raise ValueError("Tank capacity must be positive.")

    if not 0 <= config.initial_volume_liters <= config.capacity_liters:
        raise ValueError("Initial volume is outside tank capacity.")

    if config.simulation_duration_minutes <= 0:
        raise ValueError("Simulation duration must be positive.")

    if inflow_lpm < 0 or outflow_lpm < 0:
        raise ValueError("Flow rates cannot be negative.")

    result = solve_ivp(
        fun=lambda time, state: tank_dynamics(
            time, state, inflow_lpm, outflow_lpm
        ),
        t_span=(0, config.simulation_duration_minutes),
        y0=[config.initial_volume_liters],
        t_eval=[0, config.simulation_duration_minutes],
    )

    if not result.success:
        raise RuntimeError("Tank simulation failed.")

    final_volume = float(result.y[0][-1])

    final_level = (
        final_volume / config.capacity_liters
    ) * 100

    return {
        "initial_volume_liters": config.initial_volume_liters,
        "final_volume_liters": final_volume,
        "final_level_percent": final_level,
        "inflow_lpm": inflow_lpm,
        "outflow_lpm": outflow_lpm,
        "duration_minutes": config.simulation_duration_minutes,
        "simulation_success": result.success,
        "within_tank_capacity": (
            0 <= final_volume <= config.capacity_liters
        ),
    }


if __name__ == "__main__":
    tank = TankConfig()

    simulation = simulate_tank(
        config=tank,
        inflow_lpm=40.0,
        outflow_lpm=20.0,
    )

    print("=" * 45)
    print("       VOLTGUARD PHYSICS ENGINE")
    print("          WATER TANK MODEL")
    print("=" * 45)

    for key, value in simulation.items():
        print(f"{key}: {value}")
