# VoltGuard — Operating Rules and Physics Constraints



### Purpose



This document defines the provisional operating rules and physical constraints for the simulated industrial water tank system used by VoltGuard.



These rules establish baseline conditions against which simulated commands and process readings will be evaluated.



The initial model is intended for educational testing and does not represent a certified industrial process.



### Simulated Process Description

The initial VoltGuard simulation represents a water tank system containing:

* Water storage tank
* Water pump
* Control valve
* Flow sensor
* Tank level sensor
* Pressure measurement



The pump moves water through a pipe and control valve into or out of the simulated tank, depending on the selected model configuration.



The simulation will use consistent units and a documented flow direction.



### Process Variables

#### Tank Level

Tank level represents the percentage of the tank's usable capacity occupied by water.



Provisional limits:



**Condition**                **Tank Level**

Minimum physical value       0%

Maximum physical value      100%

Normal operating range     20–80%

Low-level warning         Below 10%

High-level warning        Above 90%



Values below 0% or above 100% are invalid for the configured percentage representation.



#### Flow Rate



Flow rate represents the volume of fluid moving through the simulated pipe per minute.



Provisional limits:



**Condition                     Flow Rate**

Minimum                         0 L/min

Maximum                        60 L/min

Expected flow with pump 

ON and open path             20–60 L/min

Expected flow with pump OFF    0–5 L/min



These values are initial simulation assumptions. Actual expected flow depends on the pump model, valve opening, pipe resistance and other parameters.



3.3 Pump State



The pump has two operating states:



ON — pump is active.



OFF — pump is inactive.



The pump state influences expected fluid flow.



A pump that is ON does not automatically guarantee flow. A closed valve, blocked pipe, pump fault or sensor issue could also result in low flow.



3.4 Valve Opening



Valve opening represents the percentage of the valve's available opening.



Value



Interpretation



0%



Fully closed



1–99%



Partially open



100%



Fully open



Below 0%



Invalid



Above 100%



Invalid



The initial model will use valve opening as an input to the flow calculation.



3.5 Pressure



Pressure represents the simulated fluid pressure at a defined measurement point.



The model must specify whether the pressure variable represents absolute pressure or gauge pressure.



For an initial absolute-pressure model:



Pressure must be greater than zero.



Negative absolute pressure is physically invalid.



Pressure must remain within the configured model limits.



If gauge pressure is used instead, negative values may be physically possible under certain conditions. Therefore, a negative gauge-pressure reading must not automatically be classified as impossible.



The selected pressure definition, units and limits must be documented before implementing pressure validation.



4\. Provisional Operating Rules



Rule OR-01: Tank Level Validation



The tank level must remain between 0% and 100%.



Values outside this range shall be classified as invalid simulation inputs or process readings.



Rule OR-02: Low Tank Level



A tank level below 10% shall generate a low-level warning.



This condition is an operational warning and does not independently indicate an intrusion.



Rule OR-03: High Tank Level



A tank level above 90% shall generate a high-level warning.



A tank level above 100% shall be classified as invalid.



Rule OR-04: Flow Rate Validation



Flow rate must remain within the configured range of 0–60 L/min.



Negative flow values are outside the initial model's supported range. Reverse flow can be modeled later if the system explicitly supports it.



Rule OR-05: Pump ON and Flow Path Open



When the pump is ON and the valve is sufficiently open, the model shall calculate an expected flow rate.



For initial testing, expected flow is provisionally 20–60 L/min.



A flow reading below this range may generate an operational anomaly requiring investigation.



Rule OR-06: Pump OFF and Unexpected Flow



When the pump is OFF, the initial model expects flow between 0 and 5 L/min.



Flow significantly above this range shall generate an unexpected-flow alert.



The model must account for any explicitly configured residual flow or other physical effects.



Rule OR-07: Valve Opening Validation



Valve opening must remain between 0% and 100%.



Values outside this range shall be rejected as invalid command values before being applied to the simulation.



Rule OR-08: Pressure Validation



Pressure values shall be validated according to the documented pressure definition and configured model limits.



For absolute pressure, zero or negative values shall be rejected.



For gauge pressure, the model shall use an appropriate documented range.



Rule OR-09: Tank Level Change



Tank level changes must be consistent with the modeled fluid balance.



A sudden change that cannot be explained by the configured inlet flow, outlet flow, tank capacity and simulation time step shall generate a physics inconsistency warning.



The initial change threshold will be calibrated during simulation testing rather than treated as a universal industrial limit.



5\. Physics Model Considerations



The simulation should use a consistent relationship between flow and tank level.



A simplified mass-balance relationship is:



Change in tank volume =

&#x20;   Inlet volume - Outlet volume



For a tank with constant cross-sectional area:



Change in liquid height =

&#x20;   Net flow rate × Time interval / Tank area



The model must convert units consistently.



The pump, valve and pressure relationships should be simplified but internally consistent. The initial simulation should not treat all sensor values as independent if they are physically related.



6\. Detection Classification



VoltGuard should distinguish the following conditions:



Classification



Meaning



Normal



Values are consistent with configured rules



Protocol Error



Message structure or protocol field is invalid



Invalid Command



Command value is outside the supported range



Physics Anomaly



Process behaviour violates the configured physical model



Operational Warning



A potentially abnormal operating condition requires attention



Unclassified



Insufficient information to determine the condition



A single event may have more than one classification.



7\. Threshold Calibration



All initial thresholds are provisional.



During development, they will be reviewed using:



Normal simulation runs.



Pump ON and OFF scenarios.



Different valve openings.



Different flow rates.



Tank filling and draining scenarios.



Invalid input tests.



Repeated test runs.



Threshold changes must be recorded in the project progress log or test documentation.



8\. Important Limitation



A violation of an operating rule does not prove malicious activity.



Possible causes include:



Sensor malfunction.



Mechanical failure.



Incorrect configuration.



Communication errors.



Simulation limitations.



Unexpected but legitimate process conditions.



Malicious command activity.



VoltGuard will report observed evidence and the violated rule without claiming certainty about the cause.

