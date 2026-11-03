\# VoltGuard – Windows Installation \& User Guide



\*\*Version:\*\* 1.0.0

\*\*Platform:\*\* Windows 10 / Windows 11 (64-bit)

\*\*Project:\*\* Physics-Aware ICS/SCADA Intrusion Detection System



\## 1. Overview



VoltGuard is a locally executed, physics-aware ICS/SCADA intrusion detection prototype. It analyses synthetic Modbus/TCP traffic and simulated industrial process conditions to identify protocol validation failures and physics-based anomalies.



The package includes:



\- Qt6 desktop dashboard

\- C++ Modbus/TCP traffic generator

\- C++ protocol parser

\- Python physics analysis engine

\- Combined JSON analysis reports

\- Synthetic normal and abnormal process snapshots



\## 2. System Requirements



\- Windows 10 or Windows 11, 64-bit

\- Python 3.12

\- Internet connection for initial Python dependency installation

\- Approximately 500 MB of free disk space recommended

\- No physical PLC, pump, valve, or industrial network required



\## 3. Extract the Package



1\. Download the VoltGuard Windows ZIP package.

2\. Extract the ZIP file to a suitable folder.

3\. Open PowerShell inside the extracted `VoltGuard-v1.0.0-Windows-x64` folder.



The package structure should include:



\- `app/` – Windows executable files and Qt runtime libraries

\- `data/generated/` – synthetic traffic and process snapshots

\- `docs/` – project documentation

\- `reports/` – generated analysis reports

\- `src/python/` – Python analysis modules

\- `run\_analysis.py` – analysis launcher

\- `requirements-runtime.txt` – Python runtime dependencies



\## 4. Install Python Dependencies



Install Python 3.12 if it is not already available.



From the extracted package directory, create a virtual environment:



```powershell

py -3.12 -m venv .venv

```



Install the required dependencies:



```powershell

.\\.venv\\Scripts\\python.exe -m pip install -r requirements-runtime.txt

```



Verify the Python installation:



```powershell

.\\.venv\\Scripts\\python.exe --version

```



\## 5. Launch the VoltGuard Desktop Dashboard



From the extracted package directory, run:



```powershell

Start-Process -FilePath ".\\app\\voltguard\_dashboard.exe" -WorkingDirectory (Get-Location).Path

```



The dashboard provides these sections:



1\. Overview

2\. Network Traffic

3\. Protocol Analysis

4\. Physics Monitoring

5\. Security Alerts

6\. Reports



Use the scenario selector to view the available normal and abnormal analysis reports.



\## 6. Run Normal Scenario Analysis



Execute:



```powershell

.\\.venv\\Scripts\\python.exe .\\run\_analysis.py normal

```



This analyses:



\- `data/generated/parser\_results.json`

\- `data/generated/process\_snapshot.json`



The generated report is saved to:



`reports/normal\_combined\_analysis\_report.json`



\## 7. Run Abnormal Scenario Analysis



Execute:



```powershell

.\\.venv\\Scripts\\python.exe .\\run\_analysis.py abnormal

```



This analyses:



\- `data/generated/parser\_results.json`

\- `data/generated/abnormal\_process\_snapshot.json`



The abnormal process snapshot includes a simulated condition in which the pump is switched off while flow continues above the expected range.



The generated report is saved to:



`reports/abnormal\_combined\_analysis\_report.json`



\## 8. Run the C++ Generator and Parser (Optional)



From the extracted package directory:



Generate synthetic Modbus/TCP traffic:



```powershell

.\\app\\voltguard\_generator.exe

```



Parse the generated traffic:



```powershell

.\\app\\voltguard\_parser.exe

```



These executables operate on local synthetic files. Running them may overwrite existing generated data and parser results.



After running the generator and parser, you can run either Python analysis scenario again.



\## 9. Understanding Analysis Results



VoltGuard combines protocol validation and physics-based process monitoring.



The reports include:



\- Parser analysis summary

\- Protocol validation results

\- Physics analysis results

\- Overall status

\- Interpretation of detected indicators



An `ANOMALY\_INDICATORS\_FOUND` status means that one or more checks identified conditions requiring investigation.



It does not independently establish malicious activity or malicious intent. Review the individual parser and physics findings to understand the reason for the status.



\## 10. Safety and Operational Limitations



\- VoltGuard is a prototype intended for educational, research, and demonstration purposes.

\- All included traffic and process data are synthetic.

\- The application does not connect to or control real industrial equipment.

\- The physics thresholds are provisional simulation parameters, not certified industrial safety limits.

\- An anomaly score or validation failure is an indicator for further investigation, not proof of an attack.

\- Do not use this prototype as the sole security or safety mechanism in a production ICS/SCADA environment.



\## 11. Project Documentation



Additional documentation is available in the `docs/` directory:



\- `architecture.md`

\- `operating-rules.md`

\- `progress-log.md`

\- `requirements.md`

\- `safety-boundaries.md`



\*\*VoltGuard v1.0.0 – Windows x64\*\*

Physics-Aware ICS/SCADA Intrusion Detection System

Intern at Infotact Solutions
