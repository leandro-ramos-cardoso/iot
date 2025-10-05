#include <WiFi.h> // A biblioteca essencial para gerenciar o Wi-Fi no ESP32

// ===================================
// 1. Definições de Credenciais
// ===================================
// SUBSTITUA estas variáveis com as suas credenciais de rede!
const char* ssid = "Starlink IlhaTech 2.4"; 
const char* password = "Bemvind@s1"; 

// ===================================
// 2. Função setup()
// ===================================
void setup() {
  // Inicia a comunicação serial a 115200 bps
  Serial.begin(115200); 
  delay(100);

  Serial.println("===================================");
  Serial.println("  Teste de Conexão WiFi do ESP32   ");
  Serial.println("===================================");
  Serial.print("Tentando conectar a rede: ");
  Serial.println(ssid);

  // Configura o ESP32 para operar como uma Estação (cliente)
  WiFi.mode(WIFI_STA); 
  // Inicia a tentativa de conexão
  WiFi.begin(ssid, password); 

  // Loop de espera
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // Espera 500 milissegundos
    Serial.print("."); // Imprime um ponto para mostrar que está tentando
  }

  // Se o loop for quebrado, significa que a conexão foi estabelecida
  Serial.println();
  Serial.println(">>> WiFi Conectado com Sucesso! <<<");
  Serial.println("-----------------------------------");
  
  // Imprime informações importantes
  Serial.print("Endereço IP Local: ");
  Serial.println(WiFi.localIP()); // Exibe o IP que o roteador atribuiu

  Serial.print("MAC Address do ESP32: ");
  Serial.println(WiFi.macAddress()); // Exibe o identificador único da placa
  Serial.println("-----------------------------------");
}

// ===================================
// 3. Função loop()
// ===================================
// Não precisamos fazer nada continuamente, então o loop fica vazio.
void loop() {
  // O teste termina no setup. O LED embutido pode ser usado como indicador:
  // Se o LED da placa (GPIO 2) acender aqui, o WiFi está conectado.
  // digitalWrite(2, HIGH); 
}