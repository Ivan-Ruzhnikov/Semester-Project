// Управление шаговым двигателем NEMA17 с драйвером A4988
// Линейное перемещение

// Константы подключения
const int STEP_PIN = 3;
const int DIR_PIN = 2;
const int ENABLE_PIN = 4;

// Параметры двигателя и механики
const int STEPS_PER_REV = 3200;  // Шагов на оборот для NEMA17
const float SCREW_PITCH = 2;  // Шаг резьбы шпильки в мм
const float MM_PER_STEP = SCREW_PITCH / STEPS_PER_REV;


// Текущее состояние
float currentPosition = 0.0;  // Текущая позиция в мм
int speedRPM = 60;           // Скорость вращения (RPM)


float zeroPosition = 0.0;    // Нулевая позиция

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
  Serial.begin(9600);
  Serial.println("Stepper Motor Control System");
  Serial.println("Режим линейного перемещения");
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    switch (command) {
      
      case 's':  // Установка скорости
        setSpeed();
        return;
        
      case 'h':  // Вывод меню
        printMainMenu();
        return;

      case 'z':  // Установка текущей позиции как нулевой
        setCurrentPositionAsZero();
        return;

      case 'r':  // Возврат в нулевое положение
        returnToZero();
        return;
    }
    handleLinearMode(command);
    }
}

void handleLinearMode(char command) {
  switch (command) {
    case '1':  // Перемещение на заданное расстояние
      Serial.println("Введите целевую позицию в мм:");
      while (!Serial.available()) {}
      float targetPosition = Serial.parseFloat();
      moveToPosition(targetPosition + zeroPosition);
      break;
  }
}

void setSpeed() {
  Serial.println("Введите скорость (RPM):");
  while (!Serial.available()) {}
  speedRPM = Serial.parseInt();
  Serial.println("\n>> Установлена новая скорость: " + String(speedRPM) + " RPM");
}

void setCurrentPositionAsZero() {

  zeroPosition = currentPosition;
  Serial.println("\n>> Установлена новая нулевая точка для позиции");
  Serial.println(">> Текущая позиция: 0мм");
  Serial.println(">> Абсолютное значение: " + String(currentPosition) + "мм");
}

void returnToZero() {
  Serial.println("\n>> Возврат в нулевую позицию...");
  moveToPosition(zeroPosition);
}

void moveToPosition(float targetPosition) {
  float distanceMM = targetPosition - currentPosition;
  bool forward = (distanceMM > 0);
  digitalWrite(DIR_PIN, forward ? HIGH : LOW);
  
  long steps = abs(round(distanceMM / MM_PER_STEP));
  float stepsPerSecond = (speedRPM * STEPS_PER_REV) / 60.0;
  int delayBetweenSteps = (100000 / stepsPerSecond) / 2;
  
  Serial.println("\n>> Перемещение...");
  Serial.println(">> Из: " + String(currentPosition - zeroPosition) + "мм в: " + String(targetPosition - zeroPosition) + "мм");
  
  for (long i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(delayBetweenSteps);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(delayBetweenSteps);
  }
  
  currentPosition = targetPosition;
  Serial.println(">> Перемещение завершено");
  Serial.println(">> Текущая позиция: " + String(currentPosition - zeroPosition) + "мм");
  Serial.println(">> Абсолютное значение: " + String(currentPosition) + "мм");
}

void printMainMenu() {
  Serial.println("\n=== Система управления шаговым двигателем ===");

  Serial.println("1: Переместить на заданное расстояние");
  Serial.println("Текущая позиция: " + String(currentPosition - zeroPosition) + "мм");
  Serial.println("Нулевая точка установлена на: " + String(zeroPosition) + "мм");
  
  
  Serial.println("\nОбщие команды:");
  Serial.println("z: Установить текущую позицию как нулевую");
  Serial.println("r: Вернуться в нулевое положение");
  Serial.println("s: Установка скорости (текущая: " + String(speedRPM) + " RPM)");
  Serial.println("h: Показать это меню");
  Serial.println("=====================================");
}

