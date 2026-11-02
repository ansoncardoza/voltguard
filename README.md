# **VoltGuard:** Physics-Aware ICS/SCADA Intrusion Detection System

#### **Project Domain:** Operational Technology (OT) / Industrial Control Systems (ICS)

#### **Project Type:** Security Monitoring and Intrusion Detection

#### **Organization:** Infotact Solutions

#### **Role:** Security Admin Associate L1 Intern

## 

## Project Overview: VoltGuard

VoltGuard is a Physics-Aware Industrial Control System (ICS) and Supervisory Control and Data Acquisition (SCADA) Intrusion Detection System designed to identify suspicious activities by analysing both cybersecurity events and industrial process behaviour.



Traditional network-based intrusion detection focuses primarily on network traffic and known attack patterns. However, in industrial environments, suspicious activity may also become visible through unexpected physical process behaviour, abnormal sensor readings, and inconsistencies between equipment states.



VoltGuard proposes combining two independent detection approaches:



***Cybersecurity Evidence Detection:*** Examines simulated cybersecurity events and communication-related indicators for suspicious activity.



***Physical Process Evidence Detection:*** Analyses sensor readings and operating conditions to identify unexpected or physically inconsistent behaviour.



By correlating these two evidence sources, VoltGuard aims to provide more informative and explainable security alerts.



The initial implementation uses a simulated industrial water-tank process rather than connecting to real industrial equipment.



## Why the project is being developed?

The initial prototype will simulate a water-tank control system containing:

* Water storage tank
* Water pump
* Control valve
* Tank-level sensor
* Flow-rate sensor



The simulator will generate process readings and equipment states for the detection engine.



## Problem States:

Standard IT firewalls analyze packets, but they don't understand physics. If state-sponsored malware intercepts a water treatment plant's network and sends a perfectly formatted Modbus packet telling a pump to spin at 50,000 RPM, the IT firewall allows it because the syntax is correct—resulting in a catastrophic physical pipe burst.



## How the proposed system works?

The proposed processing workflow consists of:

* Industrial Process Simulation
* Sensor and Event Data Collection
* Data Validation
* Physics-Aware Anomaly Detection
* Cybersecurity Event Detection
* Evidence Correlation and Alert Generation
* Reporting and Evaluation



## Which technologies are being used?

**Python** – Application development

**C++ Programming**

**NumPy** – Numerical operations

**pandas** – Data handling and analysis

**pytest** – Automated testing

**CSV / JSON** – Data storage and reporting

**Git** – Version control

**GitHub** – Source code management and collaboration



**Additional libraries may be introduced as development requirements evolve.**



## What features are planned?

1. **Industrial Process Simulation:** Generate synthetic readings for the water tank, pump, valve, and sensors.
2. **Physics-Aware Detection:** Identify abnormal values and inconsistencies between sensor readings and equipment operating states.
3. **Cybersecurity Evidence Analysis:** Examine selected simulated control and communication-related events.
4. **Evidence Correlation:** Combine relevant indicators and provide explanations for generated alerts.
5. **Alert Management:** Record event timestamps, observations, violated rules, and severity classifications.
6. **Reporting:** Generate structured findings and evaluation results in CSV or JSON format.



## Repository Structure

VoltGuard/

│

├── cpp/

│   ├── parser/

│   │   ├── main.cpp

│   │   ├── modbus\_parser.cpp

│   │   └── modbus\_parser.h

│   │

│   ├── traffic\_generator/

│   │   └── traffic\_generator.cpp

│   │

│   └── dashboard/

│       ├── main.cpp

│       ├── mainwindow.cpp

│       ├── mainwindow.h

│       └── CMakeLists.txt

│

├── python/

│   ├── physics\_engine.py

│   ├── bridge.py

│   └── detection.py

│

├── data/

│   ├── mock\_traffic/

│   └── generated/

│

├── tests/

│   ├── cpp/

│   └── python/

│

├── docs/

│   ├── requirements.md

│   ├── architecture.md

│   ├── operating-rules.md

│   ├── safety-boundaries.md

│   └── progress-log.md

│

├── reports/

├── README.md

├── requirements.txt

├── CMakeLists.txt

└── .gitignore



## Project Disclaimer

VoltGuard is a prototype developed for educational and defensive cybersecurity research. It is not a certified industrial intrusion detection product and must not be deployed to protect operational industrial infrastructure without appropriate engineering, validation, and safety assessment.

