import serial
import time
import serial.tools.list_ports


class MotorDriver:
    def __init__(self):
        self.ser = None

    @staticmethod
    def get_available_ports():
        """Возвращает список доступных COM-портов"""
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def connect(self, port_name):
        """Подключение к Arduino"""
        try:
            self.ser = serial.Serial(port_name, 9600, timeout=1)
            time.sleep(2)
            # Инициализация: переключаем в Линейный режим (из меню Arduino)
            # Посылаем 'm' (меню), ждем, посылаем '2' (Linear Mode)
            self.ser.write(b'm')
            time.sleep(0.5)
            self.ser.write(b'2')
            time.sleep(0.5)
            self.ser.reset_input_buffer()
            return True
        except serial.SerialException as e:
            print(f"Ошибка подключения: {e}")
            return False

    def disconnect(self):
        """Отключение от Arduino"""
        if self.ser and self.ser.is_open:
            self.ser.close()

    def set_zero(self):
        """Отправляет команду установки нуля"""
        if self.ser and self.ser.is_open:
            self.ser.write(b'z')

    def move_to(self, target_position_mm):
        """
        Отправляет команду движения к абсолютной позиции
        Arduino ждет: '1' -> (пауза/prompt) -> число
        """
        if self.ser and self.ser.is_open:
            self.ser.write(b'1')
            time.sleep(0.1)
            command = f"{target_position_mm}\n"
            self.ser.write(command.encode('utf-8'))

    def emergency_stop(self):
        """Вынужденная остановка"""
        if self.ser and self.ser.is_open:
            self.ser.close()
