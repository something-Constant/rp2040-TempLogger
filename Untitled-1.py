"""
TCP Scanner GUI with PyQt6 - Scan IP ranges for open TCP ports,
measure latency, and show fastest SOCKS proxies in real-time.
"""

import sys
import asyncio
import time
import ipaddress
from base64 import b64encode
from typing import List, Tuple, Optional
from threading import Thread
import subprocess
import platform


from PyQt6.QtWidgets import (
    QApplication,
    QMainWindow,
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QSpinBox,
    QDoubleSpinBox,
    QPushButton,
    QTableWidget,
    QTableWidgetItem,
    QProgressBar,
    QTextEdit,
    QGroupBox,
    QFormLayout,
    QHeaderView,
    QMessageBox,
)
from PyQt6.QtCore import Qt, QThread, pyqtSignal, QTimer
from PyQt6.QtGui import QColor, QFont


# -------------------- Worker Thread with Asyncio --------------------
class ScanWorker(QThread):
    # Signals to update UI
    progress_update = pyqtSignal(int, int)  # scanned, total
    new_result = pyqtSignal(float, str, str)  # latency, ip, socks_url
    scan_finished = pyqtSignal(int)  # total open ports
    log_message = pyqtSignal(str)  # status messages

    def __init__(
        self,
        cidr: str,
        port: int,
        concurrency: int,
        timeout: float,
        top_n: int,
        username: str,
        password: str,
    ):
        super().__init__()
        self.cidr = cidr
        self.port = port
        self.concurrency = concurrency
        self.timeout = timeout
        self.top_n = top_n
        self.auth_str = f"{username}:{password}"
        self.auth_b64 = b64encode(self.auth_str.encode()).decode()
        self.is_running = True
        self.results: List[Tuple[float, str, str]] = []  # (latency, ip, socks_url)
        self.scanned = 0
        self.open_ports = 0
        self.total_ips = 0

    def run(self):
        """Main entry point for QThread."""
        try:
            asyncio.run(self.scan())
        except Exception as e:
            self.log_message.emit(f"Error: {e}")

    async def check_ip(self, ip: str, semaphore: asyncio.Semaphore):
        """Test TCP connection and measure latency."""
        if not self.is_running:
            return
        async with semaphore:
            try:
                start = time.monotonic()
                reader, writer = await asyncio.wait_for(
                    asyncio.open_connection(ip, self.port), timeout=self.timeout
                )
                latency = (time.monotonic() - start) * 1000  # ms
                writer.close()
                await writer.wait_closed()

                # Build SOCKS URL
                socks_url = (
                    f"socks://{self.auth_b64}@{ip}:{self.port}#{self.open_ports + 1}"
                )
                self.open_ports += 1
                self.new_result.emit(latency, ip, socks_url)

                # Keep top N results sorted by latency
                self.results.append((latency, ip, socks_url))
                self.results.sort(key=lambda x: x[0])
                if len(self.results) > self.top_n:
                    self.results.pop()

            except (asyncio.TimeoutError, OSError, ConnectionRefusedError):
                pass
            finally:
                self.scanned += 1
                self.progress_update.emit(self.scanned, self.total_ips)

    async def scan(self):
        """Asyncio scanning coroutine."""
        try:
            network = ipaddress.ip_network(self.cidr, strict=False)
        except ValueError as e:
            self.log_message.emit(f"Invalid CIDR: {e}")
            return

        self.total_ips = network.num_addresses
        self.log_message.emit(
            f"Starting scan of {self.total_ips} IPs on port {self.port}..."
        )

        semaphore = asyncio.Semaphore(self.concurrency)
        tasks = []
        for ip in network.hosts():
            if not self.is_running:
                break
            tasks.append(asyncio.create_task(self.check_ip(str(ip), semaphore)))

        # Wait for all tasks to complete (or until stopped)
        await asyncio.gather(*tasks, return_exceptions=True)
        self.scan_finished.emit(self.open_ports)
        self.log_message.emit(f"Scan finished. Open ports found: {self.open_ports}")

    def stop(self):
        """Stop scanning gracefully."""
        self.is_running = False
        self.log_message.emit("Stopping scan...")


# -------------------- Main GUI Window --------------------
class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("🚀 TCP Port Scanner - Fast SOCKS Finder")
        self.setGeometry(100, 100, 1100, 700)
        self.worker: Optional[ScanWorker] = None

        # Central widget and main layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)

        # --- Settings Group ---
        settings_group = QGroupBox("⚙️ Scan Settings")
        form_layout = QFormLayout()

        self.cidr_edit = QLineEdit("2.144.0.0/16")
        self.cidr_edit.setPlaceholderText("e.g., 192.168.1.0/24")
        form_layout.addRow("CIDR Range:", self.cidr_edit)

        self.port_spin = QSpinBox()
        self.port_spin.setRange(1, 65535)
        self.port_spin.setValue(5201)
        form_layout.addRow("Port:", self.port_spin)

        self.username_edit = QLineEdit("admin")
        self.username_edit.setPlaceholderText("SOCKS username")
        form_layout.addRow("Username:", self.username_edit)

        self.password_edit = QLineEdit("admin")
        self.password_edit.setPlaceholderText("SOCKS password")
        self.password_edit.setEchoMode(QLineEdit.EchoMode.Password)
        form_layout.addRow("Password:", self.password_edit)

        self.concurrency_spin = QSpinBox()
        self.concurrency_spin.setRange(10, 1000)
        self.concurrency_spin.setValue(300)
        self.concurrency_spin.setSingleStep(50)
        form_layout.addRow("Concurrent Connections:", self.concurrency_spin)

        self.timeout_spin = QDoubleSpinBox()
        self.timeout_spin.setRange(0.5, 10.0)
        self.timeout_spin.setValue(2.0)
        self.timeout_spin.setSingleStep(0.5)
        self.timeout_spin.setSuffix(" sec")
        form_layout.addRow("Timeout:", self.timeout_spin)

        self.top_spin = QSpinBox()
        self.top_spin.setRange(5, 100)
        self.top_spin.setValue(20)
        form_layout.addRow("Keep Top N Results:", self.top_spin)

        settings_group.setLayout(form_layout)
        main_layout.addWidget(settings_group)

        # --- Control Buttons ---
        btn_layout = QHBoxLayout()
        self.start_btn = QPushButton("▶ Start Scan")
        self.start_btn.setStyleSheet(
            "QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }"
        )
        self.start_btn.clicked.connect(self.start_scan)

        self.stop_btn = QPushButton("⏹ Stop")
        self.stop_btn.setEnabled(False)
        self.stop_btn.setStyleSheet(
            "QPushButton { background-color: #f44336; color: white; font-weight: bold; padding: 8px; }"
        )
        self.stop_btn.clicked.connect(self.stop_scan)

        self.save_btn = QPushButton("💾 Save Best to File")
        self.save_btn.clicked.connect(self.save_results)

        btn_layout.addWidget(self.start_btn)
        btn_layout.addWidget(self.stop_btn)
        btn_layout.addWidget(self.save_btn)
        btn_layout.addStretch()
        main_layout.addLayout(btn_layout)

        # --- Progress Bar ---
        self.progress_bar = QProgressBar()
        self.progress_bar.setFormat("Scanned %v of %m IPs")
        main_layout.addWidget(self.progress_bar)

        # --- Results Table ---
        self.table = QTableWidget()
        self.table.setColumnCount(3)
        self.table.setHorizontalHeaderLabels(
            ["IP Address", "Latency (ms)", "SOCKS URL"]
        )
        self.table.horizontalHeader().setSectionResizeMode(
            0, QHeaderView.ResizeMode.ResizeToContents
        )
        self.table.horizontalHeader().setSectionResizeMode(
            1, QHeaderView.ResizeMode.ResizeToContents
        )
        self.table.horizontalHeader().setSectionResizeMode(
            2, QHeaderView.ResizeMode.Stretch
        )
        self.table.setAlternatingRowColors(True)
        self.table.setSortingEnabled(False)
        main_layout.addWidget(self.table)

        # --- Log Console ---
        self.log_console = QTextEdit()
        self.log_console.setReadOnly(True)
        self.log_console.setMaximumHeight(100)
        self.log_console.setFont(QFont("Courier", 9))
        main_layout.addWidget(self.log_console)

        # Internal storage for current results
        self.current_results: List[Tuple[float, str, str]] = []

    def log(self, message: str):
        """Append message to log console."""
        self.log_console.append(message)

    def start_scan(self):
        """Validate inputs and start scanning thread."""
        cidr = self.cidr_edit.text().strip()
        if not cidr:
            QMessageBox.warning(self, "Input Error", "Please enter a CIDR range.")
            return

        try:
            ipaddress.ip_network(cidr, strict=False)
        except ValueError as e:
            QMessageBox.critical(self, "Invalid CIDR", str(e))
            return

        # Disable controls during scan
        self.start_btn.setEnabled(False)
        self.stop_btn.setEnabled(True)
        self.save_btn.setEnabled(False)

        # Clear previous data
        self.current_results.clear()
        self.table.setRowCount(0)
        self.progress_bar.setValue(0)
        self.progress_bar.setMaximum(100)  # temporary, will be set by worker
        self.log_console.clear()

        # Create and start worker
        self.worker = ScanWorker(
            cidr=cidr,
            port=self.port_spin.value(),
            concurrency=self.concurrency_spin.value(),
            timeout=self.timeout_spin.value(),
            top_n=self.top_spin.value(),
            username=self.username_edit.text(),
            password=self.password_edit.text(),
        )
        self.worker.progress_update.connect(self.on_progress_update)
        self.worker.new_result.connect(self.on_new_result)
        self.worker.scan_finished.connect(self.on_scan_finished)
        self.worker.log_message.connect(self.log)

        self.worker.start()

    def stop_scan(self):
        """Stop the scanning thread."""
        if self.worker and self.worker.isRunning():
            self.worker.stop()
            self.stop_btn.setEnabled(False)
            self.log("Scan stopped by user.")

    def on_progress_update(self, scanned: int, total: int):
        """Update progress bar."""
        if total > 0:
            self.progress_bar.setMaximum(total)
            self.progress_bar.setValue(scanned)
            self.progress_bar.setFormat(f"Scanned {scanned} of {total} IPs")

    def on_new_result(self, latency: float, ip: str, socks_url: str):
        """Add new result to table (keeps table sorted by latency)."""
        # Insert into current_results sorted list
        self.current_results.append((latency, ip, socks_url))
        self.current_results.sort(key=lambda x: x[0])
        # Keep only top N (though worker already limits, just to be safe)
        top_n = self.top_spin.value()
        if len(self.current_results) > top_n:
            self.current_results.pop()

        # Refresh entire table from sorted list
        self.table.setRowCount(len(self.current_results))
        for row, (lat, ip_addr, url) in enumerate(self.current_results):
            ip_item = QTableWidgetItem(ip_addr)
            lat_item = QTableWidgetItem(f"{lat:.2f}")

            # Color code latency
            if lat < 50:
                lat_item.setForeground(QColor("green"))
            elif lat < 150:
                lat_item.setForeground(QColor("#FFA500"))  # orange
            else:
                lat_item.setForeground(QColor("red"))

            url_item = QTableWidgetItem(url)

            self.table.setItem(row, 0, ip_item)
            self.table.setItem(row, 1, lat_item)
            self.table.setItem(row, 2, url_item)

    def on_scan_finished(self, open_ports: int):
        """Re-enable UI after scan completes."""
        self.start_btn.setEnabled(True)
        self.stop_btn.setEnabled(False)
        self.save_btn.setEnabled(True)
        self.log(f"✅ Scan completed. Found {open_ports} open ports.")

    def save_results(self):
        """Save current best results to best_socks.txt."""
        if not self.current_results:
            QMessageBox.information(self, "No Results", "No results to save.")
            return

        filename = "best_socks.txt"
        try:
            with open(filename, "w", encoding="utf-8") as f:
                for _, _, url in self.current_results:
                    f.write(url + "\n")
            self.log(f"📁 Saved {len(self.current_results)} SOCKS URLs to {filename}")
            QMessageBox.information(self, "Saved", f"Results saved to {filename}")
        except Exception as e:
            QMessageBox.critical(self, "Save Error", str(e))

    def closeEvent(self, event):
        """Ensure worker thread stops on window close."""
        if self.worker and self.worker.isRunning():
            self.worker.stop()
            self.worker.wait(1000)
        event.accept()


# -------------------- Main Entry --------------------
# def main():
#     app = QApplication(sys.argv)
#     window = MainWindow()
#     window.show()
#     sys.exit(app.exec())


def icmp_ping(ip):
    result = subprocess.run(["ping", ip], capture_output=True)
    # Parse output for RTT...
    return result


print(icmp_ping("2.144.21.44"))


# if __name__ == "__main__":
#     main()
