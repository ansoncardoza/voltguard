# VoltGuard — Project Requirements



**01: Mock Industrial Traffic Generation**



The system shall generate synthetic Modbus/TCP messages for testing.



**02: Modbus/TCP Protocol Parsing**



The C++ parser shall read and decode mock Modbus/TCP hexadecimal payloads.



The parser shall validate message structure and handle malformed input safely.



**03: Protocol-Level Validation**



The parser shall check:



Whether the payload has sufficient bytes.



Whether the protocol identifier is valid.



Whether the declared message length is consistent with the available data.



Whether the function code is supported by the prototype.



Whether the message contains the expected fields for that function code.



**04: Physical Process Simulation**



The Python physics engine shall simulate a basic water tank and fluid system.



The physics engine shall calculate or update process conditions based on configured inputs and simulation assumptions.



**05: Baseline Physics Validation**



The system shall define normal operating conditions for the simulated process.



It shall evaluate whether incoming command values and resulting process conditions are consistent with the configured model.



**06: Physics-Aware Anomaly Detection**



The system shall identify conditions that violate configured physical or operational rules.



Examples include:



Negative valve opening percentage.



Flow rate outside the configured model range.



Invalid pressure values according to the selected pressure definition.



Unexpected flow while the pump is OFF.



Insufficient flow while the pump is ON and the flow path is open.



Sudden or implausible changes in tank level.



A detected anomaly shall be recorded as an observation requiring investigation, not automatic proof of malicious activity.



**07: C++ and Python Bridge Integration**



The system shall transfer parsed command information from the C++ parser to the Python physics engine.



The initial integration shall use structured JSON data exchanged through a local process interface, such as standard input/output.



**08: Native Desktop Dashboard**



The project shall provide the foundation of a native desktop application using Qt and C++.



**09: Event Logging**



The system shall record relevant events, including:



Timestamp.



Original payload.



Parsed protocol fields.



Command variables.



Simulation results.



Validation status.



Detection reason.



Severity or classification where configured.



**10: Testing and Evaluation**



The project shall provide repeatable tests for:



Valid Modbus/TCP messages.



Invalid and malformed payloads.



Normal industrial commands.



Suspicious command values.



Normal simulated process conditions.



Physically invalid process conditions.



C++ and Python bridge communication.



Dashboard event display.





