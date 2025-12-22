// Управление шаговым двигателем NEMA17 с драйвером A4988
// Два режима работы: угловое позиционирование или линейное перемещение

// Константы подключения
const int STEP_PIN = 3;
const int DIR_PIN = 2;
const int ENABLE_PIN = 4;

// Параметры двигателя и механики
const int STEPS_PER_REV = 3200;  // Шагов на оборот для NEMA17
const float DEGREES_PER_STEP = 360.0 / STEPS_PER_REV;
const float SCREW_PITCH = 2;  // Шаг резьбы шпильки в мм
const float MM_PER_STEP = SCREW_PITCH / STEPS_PER_REV;

// Режимы работы
enum OperationMode {
  MODE_ANGLE,    // Режим углового позиционирования
//  MODE_LINEAR    // Режим линейного перемещения
};

// Текущее состояние
OperationMode currentMode = MODE_ANGLE;  // Текущий режим работы
float currentAngle = 0.0;    // Текущий угол в градусах
float currentPosition = 0.0;  // Текущая позиция в мм
int speedRPM = 60;           // Скорость вращения (RPM)

// Нулевые позиции для каждого режима
float zeroAngle = 0.0;       // Нулевой угол
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
  Serial.println("Выберите режим работы:");
  printModeMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    // Общие команды для обоих режимов
    switch (command) {
      case 'm':  // Выбор режима работы
        selectMode();
        return;
      
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
    
    // Команды специфичные для каждого режима
    if (currentMode == MODE_ANGLE) {
      handleAngleMode(command);
    } else {
      handleLinearMode(command);
    }
  }
}

void selectMode() {
  Serial.println("\nВыберите режим работы:");
  Serial.println("1: Режим углового позиционирования");
  Serial.println("2: Режим линейного перемещения");
  
  while (!Serial.available()) {}
  char mode = Serial.read();
  
  if (mode == '1') {
    currentMode = MODE_ANGLE;
    Serial.println("\n>> Выбран режим углового позиционирования");
    Serial.println(">> Текущий угол: " + String(currentAngle - zeroAngle) + "°");
    Serial.println(">> Нулевая позиция установлена на: " + String(zeroAngle) + "°");
  } else if (mode == '2') {
  //  currentMode = MODE_LINEAR;
  //  Serial.println("\n>> Выбран режим линейного перемещения");
  //  Serial.println(">> Текущая позиция: " + String(currentPosition - zeroPosition) + "мм");
 //  Serial.println(">> Нулевая позиция установлена на: " + String(zeroPosition) + "мм");
    
  }
  
  printMainMenu();
}

void handleAngleMode(char command) {
  switch (command) {
    case '1':  // Поворот на заданный угол
      Serial.println("Введите целевой угол в градусах:");
      while (!Serial.available()) {}
      float targetAngle = Serial.parseFloat();
      moveToAngle(targetAngle + zeroAngle);
      break;
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
  if (currentMode == MODE_ANGLE) {
    zeroAngle = currentAngle;
    Serial.println("\n>> Установлена новая нулевая точка для угла");
    Serial.println(">> Текущий угол: 0°");
    Serial.println(">> Абсолютное значение: " + String(currentAngle) + "°");
  } else {
    zeroPosition = currentPosition;
    Serial.println("\n>> Установлена новая нулевая точка для позиции");
    Serial.println(">> Текущая позиция: 0мм");
    Serial.println(">> Абсолютное значение: " + String(currentPosition) + "мм");
  }
}

void returnToZero() {
  if (currentMode == MODE_ANGLE) {
    Serial.println("\n>> Возврат в нулевое угловое положение...");
    moveToAngle(zeroAngle);
  } else {
    Serial.println("\n>> Возврат в нулевую позицию...");
    moveToPosition(zeroPosition);
  }
}

void moveToAngle(float targetAngle) {
  float angleDiff = targetAngle - currentAngle;
  bool clockwise = (angleDiff > 0);
  digitalWrite(DIR_PIN, clockwise ? HIGH : LOW);
  
  long steps = abs(round(angleDiff / DEGREES_PER_STEP));
  float stepsPerSecond = (speedRPM * STEPS_PER_REV) / 60.0;
  int delayBetweenSteps = (1000000 / stepsPerSecond) / 2;
  
  Serial.println("\n>> Перемещение...");
  Serial.println(">> Из: " + String(currentAngle - zeroAngle) + "° в: " + String(targetAngle - zeroAngle) + "°");
  
  for (long i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(delayBetweenSteps);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(delayBetweenSteps);
  }
  
  currentAngle = targetAngle;
  Serial.println(">> Перемещение завершено");
  Serial.println(">> Текущий угол: " + String(currentAngle - zeroAngle) + "°");
  Serial.println(">> Абсолютное значение: " + String(currentAngle) + "°");
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
  Serial.println("Текущий режим: " + String(currentMode == MODE_ANGLE ? "Угловой" : "Линейный"));
  
  if (currentMode == MODE_ANGLE) {
    Serial.println("1: Поворот на заданный угол");
    Serial.println("Текущий угол: " + String(currentAngle - zeroAngle) + "°");
    Serial.println("Нулевая точка установлена на: " + String(zeroAngle) + "°");
  } else {
    Serial.println("1: Переместить на заданное расстояние");
    Serial.println("Текущая позиция: " + String(currentPosition - zeroPosition) + "мм");
    Serial.println("Нулевая точка установлена на: " + String(zeroPosition) + "мм");
  }
  
  Serial.println("\nОбщие команды:");
  Serial.println("z: Установить текущую позицию как нулевую");
  Serial.println("r: Вернуться в нулевое положение");
  Serial.println("m: Выбор режима работы");
  Serial.println("s: Установка скорости (текущая: " + String(speedRPM) + " RPM)");
  Serial.println("h: Показать это меню");
  Serial.println("=====================================");
}

void printModeMenu() {
  Serial.println("\n=== Выбор режима работы ===");
  Serial.println("1: Режим углового позиционирования");
  Serial.println("2: Режим линейного перемещения");
  Serial.println("=====================================");
}