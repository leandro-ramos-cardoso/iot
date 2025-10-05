// ===================================
// 1. Definição de Pinos
// ===================================

// Pino GPIO do ESP32 conectado à saída analógica (A0) do sensor MQ-9.
// GPIO 36 é um pino de leitura analógica comum (ADC1 CH0) no ESP32.
const int MQ9_PIN = 36; 

// ===================================
// 2. Função setup()
// ===================================
// Executa uma vez no início
void setup() {
  // Inicia a comunicação serial para exibir os dados do sensor
  Serial.begin(115200); 
  Serial.println("=========================================");
  Serial.println("Iniciando Leitura do Sensor MQ-9 no ESP32");
  Serial.println("=========================================");
  
  // Configura o pino como entrada. 
  // Nota: A função analogRead() já configura o pino internamente,
  // mas incluímos para clareza.
  pinMode(MQ9_PIN, INPUT); 
}

// ===================================
// 3. Função loop()
// ===================================
// Executa repetidamente
void loop() {
  // A. Faz a leitura analógica do pino. 
  // O ESP32 utiliza um ADC de 12 bits, então o valor varia de 0 a 4095.
  int sensorValue = analogRead(MQ9_PIN); 
  
  // B. Imprime o valor lido
  Serial.print("Leitura Bruta (0-4095): ");
  Serial.println(sensorValue);
  
  // C. Espera 500 milissegundos antes da próxima leitura
  delay(500); 
}