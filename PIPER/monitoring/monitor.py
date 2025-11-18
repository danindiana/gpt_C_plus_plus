#!/usr/bin/env python3
"""
Real-time Greyskull Monitoring Tool

Monitors Tenstorrent Greyskull device metrics in real-time:
- Temperature
- Power consumption
- Memory usage
- Core utilization

Usage:
    python monitor.py [--interval SECONDS] [--duration SECONDS] [--output FILE]
"""

import argparse
import time
import subprocess
import json
from datetime import datetime
from pathlib import Path
from typing import Dict, List


class GreyskullMonitor:
    """Monitor Greyskull device metrics"""

    def __init__(self, interval: float = 1.0):
        self.interval = interval
        self.metrics_history: List[Dict] = []

    def get_tt_smi_metrics(self) -> Dict:
        """Get metrics from tt-smi"""
        try:
            # Run tt-smi (adapt based on actual tt-smi output format)
            result = subprocess.run(
                ["tt-smi", "-d", "0", "--json"],
                capture_output=True,
                text=True,
                timeout=5
            )

            if result.returncode == 0:
                return json.loads(result.stdout)
            else:
                return {}

        except (subprocess.TimeoutExpired, FileNotFoundError, json.JSONDecodeError):
            return {}

    def get_system_metrics(self) -> Dict:
        """Get system-level metrics"""
        metrics = {
            "timestamp": datetime.now().isoformat(),
        }

        # Read temperature (example - adjust path based on actual sysfs location)
        try:
            with open("/sys/class/hwmon/hwmon0/temp1_input", "r") as f:
                temp_raw = int(f.read().strip())
                metrics["temperature_c"] = temp_raw / 1000.0
        except FileNotFoundError:
            metrics["temperature_c"] = None

        # Get metrics from tt-smi
        tt_metrics = self.get_tt_smi_metrics()
        metrics.update(tt_metrics)

        return metrics

    def collect_metrics(self):
        """Collect current metrics"""
        metrics = self.get_system_metrics()
        self.metrics_history.append(metrics)
        return metrics

    def print_metrics(self, metrics: Dict):
        """Print metrics to console"""
        print(f"\r[{metrics['timestamp']}] ", end="")

        if metrics.get("temperature_c"):
            print(f"Temp: {metrics['temperature_c']:.1f}°C | ", end="")

        if metrics.get("power_w"):
            print(f"Power: {metrics['power_w']:.1f}W | ", end="")

        print("", end="", flush=True)

    def monitor(self, duration: float = None):
        """Monitor for specified duration"""
        start_time = time.time()

        try:
            while True:
                metrics = self.collect_metrics()
                self.print_metrics(metrics)

                if duration and (time.time() - start_time) >= duration:
                    break

                time.sleep(self.interval)

        except KeyboardInterrupt:
            print("\nMonitoring stopped")

    def save_history(self, output_path: Path):
        """Save metrics history to JSON"""
        with open(output_path, 'w') as f:
            json.dump(self.metrics_history, f, indent=2)
        print(f"\nMetrics saved to {output_path}")


def main():
    parser = argparse.ArgumentParser(
        description="Greyskull Real-time Monitor"
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=1.0,
        help="Sampling interval in seconds"
    )
    parser.add_argument(
        "--duration",
        type=float,
        default=None,
        help="Monitoring duration in seconds (default: infinite)"
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="Output file for metrics history"
    )

    args = parser.parse_args()

    monitor = GreyskullMonitor(interval=args.interval)
    monitor.monitor(duration=args.duration)

    if args.output:
        monitor.save_history(args.output)


if __name__ == "__main__":
    main()
