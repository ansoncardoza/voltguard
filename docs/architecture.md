# VoltGuard — System Architecture



### Introduction

VoltGuard uses a modular hybrid architecture combining C++ and Python to inspect simulated industrial network traffic and evaluate its impact on a simulated physical process.



The architecture separates protocol processing, physics modeling, anomaly detection, and desktop monitoring into independent components.



### Architectural Overview

The proposed system consists of the following components:

* Mock Modbus/TCP Traffic Generator — C++

The traffic generator creates synthetic Modbus/TCP hexadecimal messages for testing.



* Modbus/TCP Protocol Parser — C++

The protocol parser is implemented in C++.



* C++ and Python Integration Bridge — JSON

The bridge transfers decoded information from the C++ parser to the Python physics engine.



* Physics Simulation Engine — Python and SciPy

The physics engine will be implemented using Python and SciPy.



It represents a simplified water tank and fluid system containing:

* Tank liquid level
* Pump state
* Valve opening
* Fluid flow rate
* Pressure



* Physics-Aware Detection Engine — Python

The detection engine evaluates command values and simulated process behaviour against configured physical and operational rules.



* Event Logging and Alert Management

This component records relevant protocol and simulation events.



* Native Desktop Dashboard — Qt and C++

The dashboard will be developed using Qt 6 and C++.



### System Data Flow



The expected processing sequence is:



* The C++ traffic generator creates a mock Modbus/TCP message.
* The C++ parser receives the hexadecimal payload.
* The parser validates the message structure and decodes supported fields.
* The bridge converts or forwards the parsed information as structured JSON.
* The Python physics engine receives applicable command variables.
* The simulation engine calculates the resulting process state.
* The detection engine evaluates the command and process state.
* The event manager records the results.
* The Qt dashboard displays traffic, process information, and alerts.



### Proposed Data Exchange Format

{

&#x20; "timestamp": "2026-10-31T10:30:00Z",

&#x20; "transaction\_id": 1,

&#x20; "unit\_id": 1,

&#x20; "function\_code": 6,

&#x20; "register\_address": 100,

&#x20; "register\_value": 75,

&#x20; "command\_type": "valve\_opening",

&#x20; "source": "mock\_generator"

}

