
import json
import argparse
from datetime import datetime
from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.units import mm
from reportlab.platypus import (
    SimpleDocTemplate,
    Paragraph,
    Spacer,
    Table,
    TableStyle,
    PageBreak,
)

def _number(value, default=0):
    try:
        return float(value)
    except (TypeError, ValueError):
        return default

def _timestamp(value):
    if not value:
        return "Unavailable"
    return str(value)

def build_live_session_report(payload, output_directory):
    """
    Generate JSON and PDF evidence from one captured VoltGuard live session.

    payload keys:
      session_start, session_end, scenario, samples, decisions
    """

    if not isinstance(payload, dict):
        raise ValueError("Session payload must be a JSON object.")

    samples = payload.get("samples", [])
    decisions = payload.get("decisions", [])

    if not isinstance(samples, list):
        raise ValueError("Session samples must be a list.")

    if not isinstance(decisions, list):
        raise ValueError("Session decisions must be a list.")

    if not samples:
        raise ValueError("Cannot generate a report without live samples.")

    start = payload.get("session_start")
    end = payload.get("session_end")
    scenario = payload.get("scenario", "unknown")

    start_dt = datetime.fromisoformat(start) if start else None
    end_dt = datetime.fromisoformat(end) if end else None

    duration_seconds = (
        max(0, (end_dt - start_dt).total_seconds())
        if start_dt and end_dt
        else None
    )

    levels = []
    volumes = []
    inflows = []
    outflows = []
    pump_states = []

    valid_frames = 0
    validation_failures = 0
    invalid_frames = 0
    violations = 0
    warnings = 0

    for sample in samples:
        process = sample.get("process_state", {})
        validation = sample.get("validation", {})
        modbus = sample.get("modbus", {})

        levels.append(_number(process.get("tank_level_percent")))
        volumes.append(_number(process.get("tank_volume_liters")))
        inflows.append(_number(process.get("inflow_lpm")))
        outflows.append(_number(process.get("outflow_lpm")))
        pump_states.append(bool(process.get("pump_on", False)))

        status = str(modbus.get("status", "")).upper()

        if status == "VALID":
            valid_frames += 1
        elif status == "VALIDATION_FAILED":
            validation_failures += 1
        elif status == "INVALID_FRAME":
            invalid_frames += 1

        violations += len(validation.get("violations", []))
        warnings += len(validation.get("warnings", []))

    decision_counts = {"ALLOW": 0, "HOLD": 0, "DROP": 0}
    severity_counts = {"INFO": 0, "MEDIUM": 0, "HIGH": 0}

    for event in decisions:
        decision = event.get("decision_analysis", {})
        decision_value = str(
            decision.get("decision", event.get("decision", "UNKNOWN"))
        ).upper()

        severity = str(event.get("severity", "")).upper()

        if decision_value in decision_counts:
            decision_counts[decision_value] += 1

        if severity in severity_counts:
            severity_counts[severity] += 1

    report = {
        "report_type": "VoltGuard Live Session Report",
        "generated_at": datetime.now().astimezone().isoformat(),
        "session": {
            "scenario": scenario,
            "start": start,
            "end": end,
            "duration_seconds": duration_seconds,
            "sample_count": len(samples),
        },
        "process_summary": {
            "tank_level_percent": {
                "minimum": min(levels),
                "maximum": max(levels),
                "first": levels[0],
                "last": levels[-1],
            },
            "tank_volume_liters": {
                "minimum": min(volumes),
                "maximum": max(volumes),
                "first": volumes[0],
                "last": volumes[-1],
            },
            "inflow_lpm": {
                "minimum": min(inflows),
                "maximum": max(inflows),
            },
            "outflow_lpm": {
                "minimum": min(outflows),
                "maximum": max(outflows),
            },
            "pump_on_samples": sum(pump_states),
            "pump_off_samples": len(pump_states) - sum(pump_states),
        },
        "protocol_summary": {
            "valid_frames": valid_frames,
            "validation_failures": validation_failures,
            "invalid_frames": invalid_frames,
        },
        "physics_summary": {
            "violations": violations,
            "warnings": warnings,
        },
        "security_summary": {
            "decision_counts": decision_counts,
            "severity_counts": severity_counts,
            "decision_event_count": len(decisions),
        },
        "samples": samples,
        "decisions": decisions,
    }

    output_path = Path(output_directory)
    output_path.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

    scenario_names = {
    "normal_filling": "Normal_Tank_Filling",
    "pump_off_consumption": "Pump_Off",
    "invalid_traffic": "Invalid_Traffic_Test",
    }

    scenario_filename = scenario_names.get(
    scenario,
    "Other_Scenario"
    )

    base_name = f"VoltGuard_{scenario_filename}_{timestamp}"

    json_path = output_path / f"{base_name}.json"
    pdf_path = output_path / f"{base_name}.pdf"

    with json_path.open("w", encoding="utf-8") as file:
        json.dump(report, file, indent=4, ensure_ascii=False)

    styles = getSampleStyleSheet()
    styles.add(ParagraphStyle(
        name="ReportTitle",
        parent=styles["Title"],
        alignment=TA_CENTER,
        textColor=colors.HexColor("#17365D"),
        spaceAfter=12,
    ))

    story = [
        Paragraph("VoltGuard Live Session Report", styles["ReportTitle"]),
        Paragraph(
            "Simulation evidence and security monitoring summary",
            styles["Normal"],
        ),
        Spacer(1, 8 * mm),
    ]

    def add_section(title, rows):
        story.append(Paragraph(title, styles["Heading2"]))
        table = Table(rows, colWidths=[65 * mm, 105 * mm])
        table.setStyle(TableStyle([
            ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#D9EAF7")),
            ("TEXTCOLOR", (0, 0), (-1, 0), colors.HexColor("#17365D")),
            ("GRID", (0, 0), (-1, -1), 0.4, colors.grey),
            ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold"),
            ("FONTNAME", (0, 1), (-1, -1), "Helvetica"),
            ("FONTSIZE", (0, 0), (-1, -1), 8),
            ("PADDING", (0, 0), (-1, -1), 6),
            ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ]))
        story.extend([table, Spacer(1, 5 * mm)])

    add_section("Session Information", [
        ["Field", "Value"],
        ["Scenario", str(scenario)],
        ["Session Start", _timestamp(start)],
        ["Session End", _timestamp(end)],
        [
            "Duration",
            f"{duration_seconds:.1f} seconds"
            if duration_seconds is not None else "Unavailable",
        ],
        ["Captured Samples", str(len(samples))],
    ])

    add_section("Process Monitoring", [
        ["Measurement", "Summary"],
        ["Tank Level", f"{min(levels):.2f}% to {max(levels):.2f}%"],
        ["Tank Volume", f"{min(volumes):.2f} to {max(volumes):.2f} L"],
        ["Inflow", f"{min(inflows):.2f} to {max(inflows):.2f} L/min"],
        ["Outflow", f"{min(outflows):.2f} to {max(outflows):.2f} L/min"],
        ["Pump ON samples", str(sum(pump_states))],
        ["Pump OFF samples", str(len(pump_states) - sum(pump_states))],
    ])

    add_section("Protocol Monitoring", [
        ["Metric", "Count"],
        ["Valid frames", str(valid_frames)],
        ["Validation failures", str(validation_failures)],
        ["Invalid frames", str(invalid_frames)],
    ])

    add_section("Physics Validation", [
        ["Metric", "Count"],
        ["Violations", str(violations)],
        ["Warnings", str(warnings)],
    ])

    add_section("Security Decisions", [
        ["Decision / Severity", "Count"],
        *[
            [name, str(count)]
            for name, count in decision_counts.items()
        ],
        *[
            [name, str(count)]
            for name, count in severity_counts.items()
        ],
        ["Total decision events", str(len(decisions))],
    ])

    story.append(PageBreak())
    story.append(Paragraph("Captured Live Sample History", styles["Heading2"]))

    sample_rows = [[
        "Timestamp", "Sequence", "Tank %", "Volume (L)",
        "Pump", "Inflow", "Outflow",
    ]]

    for sample in samples:
        process = sample.get("process_state", {})
        sample_rows.append([
            str(sample.get("timestamp", "")),
            str(sample.get("sequence", "")),
            f"{_number(process.get('tank_level_percent')):.2f}",
            f"{_number(process.get('tank_volume_liters')):.2f}",
            "ON" if process.get("pump_on") else "OFF",
            f"{_number(process.get('inflow_lpm')):.2f}",
            f"{_number(process.get('outflow_lpm')):.2f}",
        ])

    sample_table = Table(
        sample_rows,
        repeatRows=1,
        colWidths=[37 * mm, 15 * mm, 18 * mm, 22 * mm,
                   15 * mm, 22 * mm, 22 * mm],
    )
    sample_table.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#D9EAF7")),
        ("GRID", (0, 0), (-1, -1), 0.3, colors.grey),
        ("FONTSIZE", (0, 0), (-1, -1), 6.5),
        ("PADDING", (0, 0), (-1, -1), 4),
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
    ]))
    story.append(sample_table)

    doc = SimpleDocTemplate(
        str(pdf_path),
        pagesize=A4,
        rightMargin=15 * mm,
        leftMargin=15 * mm,
        topMargin=15 * mm,
        bottomMargin=15 * mm,
        title="VoltGuard Live Session Report",
        author="VoltGuard",
    )
    doc.build(story)

    return {
        "json_path": str(json_path.resolve()),
        "pdf_path": str(pdf_path.resolve()),
        "sample_count": len(samples),
        "decision_count": len(decisions),
    }

def main():
    parser = argparse.ArgumentParser(
        description="Generate VoltGuard live session reports."
    )

    parser.add_argument(
        "--input",
        required=True,
        help="Path to the captured live session JSON payload.",
    )

    parser.add_argument(
        "--output-dir",
        required=True,
        help="Directory where PDF and JSON reports will be saved.",
    )

    args = parser.parse_args()

    input_path = Path(args.input)

    if not input_path.is_file():
        raise FileNotFoundError(
            f"Session payload not found: {input_path}"
        )

    with input_path.open("r", encoding="utf-8") as file:
        payload = json.load(file)

    result = build_live_session_report(
        payload,
        args.output_dir,
    )

    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()