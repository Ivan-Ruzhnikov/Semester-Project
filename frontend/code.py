import sys
import os
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget,
    QVBoxLayout, QHBoxLayout, QGridLayout,
    QPushButton, QDoubleSpinBox, QRadioButton,
    QButtonGroup, QLabel, QComboBox, QMenu,
    QMessageBox
)
from PyQt6.QtGui import QDesktopServices, QAction
from PyQt6.QtCore import QSize, QUrl

from backend.motor_driver import MotorDriver


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Control Panel")
        self.setFixedSize(QSize(700, 500))
        self.driver = MotorDriver()
        self.current_position = 0.0
        self.zero_position = 0.0
        self.readme = os.path.join(os.path.dirname(os.path.abspath(__file__)), "README.txt")
        self.init_ui()

    def init_ui(self):
        """Верхняя панель"""
        top_layout = QHBoxLayout() # Подключение

        sensor_label = QLabel("Порт:")
        self.sensor_combo = QComboBox()
        self.refresh_ports()  # Заполнить список портов

        self.refresh_btn = QPushButton("Обновить")
        self.refresh_btn.clicked.connect(self.refresh_ports)

        self.connect_btn = QPushButton("Подключить")
        self.connect_btn.setCheckable(True)
        self.connect_btn.clicked.connect(self.toggle_connection)

        self.position_label = QLabel("Позиция: 0.000 мм")
        self.position_label.setStyleSheet("font-weight: bold; font-size: 14px;")

        top_layout.addWidget(sensor_label)
        top_layout.addWidget(self.sensor_combo)
        top_layout.addWidget(self.refresh_btn)
        top_layout.addWidget(self.connect_btn)
        top_layout.addStretch()
        top_layout.addWidget(self.position_label)

        """Центральная часть"""
        center_layout = QGridLayout()

        distance_label = QLabel("Расстояние, мм:")
        self.distance_spin = QDoubleSpinBox()
        self.distance_spin.setDecimals(3)
        self.distance_spin.setRange(0, 1000)
        self.distance_spin.setSingleStep(1.0)
        self.distance_spin.setValue(10.0)

        center_layout.addWidget(distance_label, 0, 0)
        center_layout.addWidget(self.distance_spin, 0, 1)

        # Направление
        direction_label = QLabel("Направление:")
        self.forward_radio = QRadioButton("Вперед (+)")
        self.backward_radio = QRadioButton("Назад (-)")
        self.direction_group = QButtonGroup(self)
        self.direction_group.addButton(self.forward_radio)
        self.direction_group.addButton(self.backward_radio)
        self.forward_radio.setChecked(True)

        dir_layout = QHBoxLayout()
        dir_layout.addWidget(self.forward_radio)
        dir_layout.addWidget(self.backward_radio)

        center_layout.addWidget(direction_label, 1, 0)
        center_layout.addLayout(dir_layout, 1, 1)

        # Быстрые кнопки
        quick_label = QLabel("Быстрый выбор:")
        quick_layout = QHBoxLayout()
        quick_distances = [1.0, 5.0, 10.0, 20.0, 50.0]
        for d in quick_distances:
            btn = QPushButton(f"{d} мм")
            btn.clicked.connect(lambda _, value=d: self.set_quick_distance(value))
            quick_layout.addWidget(btn)

        center_layout.addWidget(quick_label, 2, 0)
        center_layout.addLayout(quick_layout, 2, 1)

        """Нижняя панель"""
        bottom_layout = QHBoxLayout()

        self.set_zero_btn = QPushButton("Установить Ноль")
        self.set_zero_btn.clicked.connect(self.set_zero)

        self.move_btn = QPushButton("ДВИГАТЬ")
        self.move_btn.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
        self.move_btn.setMinimumHeight(40)
        self.move_btn.clicked.connect(self.move_sensor)

        self.stop_btn = QPushButton("СТОП")
        self.stop_btn.setStyleSheet("background-color: #f44336; color: white; font-weight: bold;")
        self.stop_btn.setMinimumHeight(40)
        self.stop_btn.clicked.connect(self.emergency_stop)

        # Кнопка ТЕСТ
        self.test_btn = QPushButton("ТЕСТ")
        self.test_btn.setStyleSheet("background-color: #17a2b8; color: white; font-weight: bold;")
        self.test_btn.setMinimumHeight(40)
        self.test_btn.clicked.connect(self.toggle_test_mode)

        bottom_layout.addWidget(self.set_zero_btn)
        bottom_layout.addWidget(self.move_btn)
        bottom_layout.addWidget(self.stop_btn)
        bottom_layout.addWidget(self.test_btn)

        # Сборка
        main_layout = QVBoxLayout()
        main_layout.addLayout(top_layout)
        main_layout.addSpacing(15)
        main_layout.addLayout(center_layout)
        main_layout.addStretch()
        main_layout.addLayout(bottom_layout)

        container = QWidget()
        container.setLayout(main_layout)
        self.setCentralWidget(container)

        # Блокируем кнопки до подключения
        self.enable_controls(False)

    """Логика"""

    def refresh_ports(self):
        self.sensor_combo.clear()
        ports = self.driver.get_available_ports()
        if ports:
            self.sensor_combo.addItems(ports)
        else:
            self.sensor_combo.addItem("Нет устройств")

    def toggle_connection(self):
        if self.connect_btn.isChecked():
            # Попытка подключения
            port = self.sensor_combo.currentText()
            if port == "Нет устройств":
                self.connect_btn.setChecked(False)
                return

            if self.driver.connect(port):
                self.connect_btn.setText("Отключить")
                self.enable_controls(True)
                QMessageBox.information(self, "Успех", f"Подключено к {port}\nРежим: Линейный")
            else:
                self.connect_btn.setChecked(False)
                QMessageBox.critical(self, "Ошибка", "Не удалось открыть порт")
        else:
            # Отключение
            self.driver.disconnect()
            self.connect_btn.setText("Подключить")
            self.enable_controls(False)

    def enable_controls(self, enable):
        self.move_btn.setEnabled(enable)
        self.set_zero_btn.setEnabled(enable)
        self.stop_btn.setEnabled(enable)
        self.test_btn.setEnabled(True)  # Тест всегда активен

    def set_quick_distance(self, value: float):
        self.distance_spin.setValue(value)

    def set_zero(self):
        """Сбрасывает координаты в ноль."""
        self.driver.set_zero()  # Отправляем команду 'z' на Arduino
        self.current_position = 0.0  # Сбрасываем локальный счетчик
        self.update_position_label()

    def move_sensor(self):
        distance = self.distance_spin.value()
        if self.backward_radio.isChecked():
            distance = -abs(distance)
        else:
            distance = abs(distance)

        # Рассчитываем НОВУЮ абсолютную координату
        target_pos = self.current_position + distance

        # Отправляем команду на Arduino
        self.driver.move_to(target_pos)
        self.current_position = target_pos
        self.update_position_label()

    def emergency_stop(self):
        self.driver.emergency_stop()
        QMessageBox.warning(self, "Внимание","Команда остановки отправлена.\n")

    def toggle_test_mode(self):
        """Переключает режим теста"""
        if self.connect_btn.text() == "ТЕСТ":
            self.connect_btn.setText("Подключить")
            self.enable_controls(False)
            QMessageBox.information(self, "ТЕСТ", "Вышли из режима теста")
        else:
            self.connect_btn.setText("ТЕСТ")
            self.enable_controls(True)
            QMessageBox.information(self, "ТЕСТ", "Режим теста включён")

    def update_position_label(self):
        self.position_label.setText(f"Позиция: {self.current_position:.3f} мм")

    """ ReadMe """
    def contextMenuEvent(self, event):
        context = QMenu(self)
        readme_action = QAction("ReadMe", self)
        readme_action.triggered.connect(self.open_readme)
        context.addAction(readme_action)
        context.exec(event.globalPos())

    def open_readme(self):
        if not os.path.exists(self.readme):
            QMessageBox.warning(self, "ReadMe", "Файл инструкции не найден.")
            return
        url = QUrl.fromLocalFile(self.readme)
        QDesktopServices.openUrl(url)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    app.exec()
