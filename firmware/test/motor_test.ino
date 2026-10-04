// Пины управления CNC Shield v3 для Arduino Uno
#define EN_PIN    8   // Пин включения всех драйверов (низкий уровень = ВКЛ)

#define X_STEP    2   // Шаг оси X
#define X_DIR     5   // Направление оси X

#define Y_STEP    3   // Шаг оси Y
#define Y_DIR     6   // Направление оси Y

void setup() {
  // Настраиваем пины на выход
  pinMode(EN_PIN, OUTPUT);
  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);
  pinMode(Y_STEP, OUTPUT);
  pinMode(Y_DIR, OUTPUT);
  
  // Включаем драйверы (подаем LOW)
  digitalWrite(EN_PIN, LOW); 
  
  delay(1000); // Пауза 1 секунда перед стартом движений

  // --- ТЕСТ ОСИ X ---
  digitalWrite(X_DIR, HIGH); // Направление 1
  for(int i = 0; i < 400; i++) { // Сделать 400 микрошагов
    digitalWrite(X_STEP, HIGH);
    delayMicroseconds(800); // Скорость
    digitalWrite(X_STEP, LOW);
    delayMicroseconds(800);
  }
  delay(1000); // Пауза 1 секунда
  
  digitalWrite(X_DIR, LOW);  // Направление 2 (обратно)
  for(int i = 0; i < 400; i++) {
    digitalWrite(X_STEP, HIGH);
    delayMicroseconds(800);
    digitalWrite(X_STEP, LOW);
    delayMicroseconds(800);
  }
  delay(1000); // Пауза 1 секунда

  // --- ТЕСТ ОСИ Y ---
  digitalWrite(Y_DIR, HIGH); // Направление 1
  for(int i = 0; i < 400; i++) {
    digitalWrite(Y_STEP, HIGH);
    delayMicroseconds(800);
    digitalWrite(Y_STEP, LOW);
    delayMicroseconds(800);
  }
  delay(1000); // Пауза 1 секунда
  
  digitalWrite(Y_DIR, LOW);  // Направление 2 (обратно)
  for(int i = 0; i < 400; i++) {
    digitalWrite(Y_STEP, HIGH);
    delayMicroseconds(800);
    digitalWrite(Y_STEP, LOW);
    delayMicroseconds(800);
  }
}

void loop() {
  // Тут пусто, чтобы тест выполнился один раз до нажатия кнопки RESET
}
