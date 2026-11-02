# \# VoltGuard — Safety and Ethical Boundaries



### Purpose



This document defines the safety, ethical and operational boundaries for developing and testing VoltGuard.



VoltGuard is an educational cybersecurity prototype designed to analyze simulated industrial protocol traffic and simulated physical process behaviour.



The project will operate in a controlled local development environment.



### Simulation-Only Environment



All initial development and testing shall use synthetic data and simulated industrial processes.



The project may include:

\- Mock Modbus/TCP hexadecimal messages.

\- Simulated industrial register values.

\- Synthetic normal and suspicious commands.

\- A simulated water tank.

\- Simulated pump and valve states.

\- Simulated flow and pressure measurements.

\- Locally generated test logs and reports.



No real industrial process is required for the prototype.



### Prohibited Activities



VoltGuard development and testing shall not:



1\. Connect to production PLCs.

2\. Send commands to real pumps, valves or industrial actuators.

3\. Modify real industrial control settings.

4\. Perform unauthorized scanning of industrial networks.

5\. Conduct unauthorized penetration testing against ICS/SCADA infrastructure.

6\. Interfere with production HMI or SCADA applications.

7\. Introduce test traffic into operational industrial networks.

8\. Use real industrial credentials or sensitive operational information without explicit authorization.



All testing must remain within the local simulated environment or another explicitly authorized isolated lab.



\## 4. Network and Protocol Safety



The initial Modbus/TCP implementation will parse and generate mock messages locally.



The protocol parser will be tested using:



\- Valid synthetic payloads.

\- Invalid hexadecimal strings.

\- Incomplete messages.

\- Unsupported function codes.

\- Unexpected register values.

\- Incorrect length fields.

\- Boundary-value test cases.



The initial traffic generator will not transmit commands to a real Modbus/TCP device.



If network-based simulation is introduced later, it must use an isolated local test environment and must not be connected to a production ICS/SCADA network.



\## 5. Physics Simulation Safety



The physics engine will represent a simplified water tank and fluid system.



All process values are simulated.



The model will not:



\- Control real pumps.

\- Open or close real valves.

\- Adjust industrial pressure systems.

\- Send physical control signals.

\- Replace industrial engineering calculations or safety controls.



Configured thresholds are educational assumptions and must not be interpreted as certified industrial limits.



\## 6. Input Validation



The application should validate external and generated inputs before processing them.



Examples include:



\- Hexadecimal payload format.

\- Modbus/TCP message length.

\- Supported function codes.

\- Register addresses.

\- Numeric data types.

\- Valve opening limits.

\- Flow rate limits.

\- Pressure definition and limits.

\- JSON structure.



Malformed inputs should be rejected or recorded safely rather than causing uncontrolled application behaviour.



\## 7. Data Handling and Evidence Integrity



VoltGuard should preserve relevant test evidence for review.



Evidence may include:



\- Original mock hexadecimal payload.

\- Parsed protocol fields.

\- Input command values.

\- Simulation output.

\- Detection results.

\- Timestamp.

\- Test scenario identifier.

\- Explanation of the violated rule.



Test records should not be silently altered during evaluation.



If test data is changed or regenerated, the relevant test run should be identified separately.



\## 8. Ethical Use



VoltGuard is intended for:



\- Cybersecurity education.

\- Industrial protocol learning.

\- Defensive security research.

\- Physics-aware anomaly detection experimentation.

\- Controlled software testing.

\- Project demonstrations.



The prototype must not be used to disrupt, manipulate or gain unauthorized access to industrial systems.



\## 9. Interpretation of Detection Results



A protocol anomaly or physics inconsistency is not sufficient evidence to conclude that a cyberattack has occurred.



Possible explanations include:



\- Sensor failure.

\- Equipment malfunction.

\- Incorrect configuration.

\- Communication issues.

\- Model inaccuracies.

\- Legitimate process changes.

\- Suspicious or malicious activity.



VoltGuard shall report the observed condition and supporting evidence while avoiding unsupported claims about intent.



\## 10. Project Limitations



The initial prototype has the following limitations:



\- It uses synthetic industrial traffic.

\- It supports only the Modbus/TCP message types implemented for the prototype.

\- It uses a simplified physical model.

\- Its operating thresholds are provisional.

\- It does not provide certified industrial safety monitoring.

\- It does not replace industrial intrusion detection products.

\- It does not establish malicious intent from anomalies alone.



\## 11. Safety Review



Before introducing a new testing capability, the development team should verify:



1\. Whether the activity remains within the authorized environment.

2\. Whether real industrial equipment could be affected.

3\. Whether generated messages are confined to the intended test setup.

4\. Whether test inputs are synthetic or explicitly authorized.

5\. Whether results can be safely recorded and reviewed.



Any proposed change that could interact with a real industrial system requires a separate authorization and safety assessment.



\---



\*\*Document Status:\*\* Draft — Phase 1  

\*\*Project:\*\* VoltGuard  

\*\*Last Updated:\*\* 2 November 2026

