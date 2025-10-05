// ===================================
// Definições de Pinos e Variáveis
// ===================================

// A maioria das placas ESP32 usa o pino GPIO 2 para o LED embutido.
// Se o seu LED não piscar, você pode precisar mudar este valor.
const int LED_PIN = 2;

// O tempo (em milissegundos) que o LED permanecerá aceso e apagado
const int DELAY_TIME = 100; // 1000 milissegundos = 1 segundo

// ===================================
// Função setup()
// ===================================
// Executa apenas uma vez quando o ESP32 é ligado ou reiniciado
void setup() {
  // Configura o pino do LED como uma SAÍDA (OUTPUT).
  // Isso permite que o ESP32 envie energia para este pino.
  pinMode(LED_PIN, OUTPUT);
}

// ===================================
// Função loop()
// ===================================
// Executa repetidamente (em loop) após o setup()
void loop() {
  // 1. Acende o LED:
  // Envia um nível ALTO (HIGH) para o pino, ligando o LED.
  digitalWrite(LED_PIN, HIGH);
  
  // 2. Espera:
  // Pausa a execução pelo tempo definido (1 segundo).
  delay(DELAY_TIME);
  
  // 3. Apaga o LED:
  // Envia um nível BAIXO (LOW) para o pino, desligando o LED.
  digitalWrite(LED_PIN, LOW);
  
  // 4. Espera:
  // Pausa a execução novamente pelo tempo definido (1 segundo).
  delay(DELAY_TIME);
}