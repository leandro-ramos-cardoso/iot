#include <WiFi.h>       // 1. Inclui a biblioteca para gerenciar a conexão Wi-Fi.
#include <HTTPClient.h> // 2. Inclui a biblioteca para fazer requisições HTTP (POST).
#include <math.h>       // 3. Inclui a biblioteca matemática, necessária para a função pow() (cálculo do PPM).

// ===================================
// 1. Definições de Credenciais e Pinos
// ===================================

const char* ssid = "Starlink IlhaTech 2.4";          // 4. Sua rede Wi-Fi (SSID). SUBSTITUA!
const char* password = "Bemvind@s1";        // 5. Sua senha do Wi-Fi. SUBSTITUA!

// 6. URL de destino da API (exato formato solicitado).
const char* api_url = "http://192.168.2.152:8080/device/data"; 

const int MQ9_PIN = 4;                         // 7. Pino GPIO do ESP32 conectado à saída analógica do MQ-9.
const int ADC_MAX = 4095;                       // 8. O valor máximo da leitura analógica do ESP32 (12 bits). 

// 9. Intervalo entre envios (10000 ms = 10 segundos).
const long SEND_INTERVAL = 10000; 

// 10. Valor fixo para o campo "gasType" no JSON.
const char* GAS_TYPE = "CO_CH4_Estimado"; 

// ===================================
// 2. Variáveis Globais e de Calibração
// ===================================
String macAddress;                              // 11. Variável para armazenar o endereço MAC da placa.
unsigned long lastSendTime = 0;                 // 12. Variável para rastrear o tempo do último envio (uso com millis()).
float R0_Bruto = 0;                             // 13. Média ADC bruta em ar limpo (base da calibração).


// ===================================
// Funções de Conectividade
// ===================================

void connectToWiFi() {
  Serial.print("Conectando a ");                // 14. Inicia a tentativa de conexão.
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);                          // 15. Define o modo Wi-Fi como Estação (cliente).
  WiFi.begin(ssid, password);                   // 16. Inicia a conexão com credenciais.

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) { // 17. Espera a conexão por até 20 segundos.
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {          // 18. Se conectado, imprime o sucesso e o IP.
    Serial.println("\nWiFi conectado!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha na conexão Wi-Fi."); // 19. Mensagem de erro.
  }
}

void getMacAddress() {
  macAddress = WiFi.macAddress();               // 20. Pega o MAC address.
  Serial.print("MAC Address: ");
  Serial.println(macAddress);
}


// ===================================
// Funções de Leitura e Calibração do Sensor MQ-9
// ===================================

int mapInvertedValue(int raw_value) {
  // 21. FUNÇÃO CHAVE: Corrige a leitura invertida (ADC Bruto Alto = Gás Baixo).
  // 22. Transforma: Gás Alto (ADC baixo) -> Sinal Normalizado Alto.
  return ADC_MAX - raw_value;
}

void CalibrateR0() {
  Serial.println("=========================================");
  Serial.println("Calibrando R0... Mantenha o sensor em ar limpo por 30s.");
  
  int sample_count = 60; // 23. Define o número de amostras (60 * 500ms = 30s).
  long sum_adc = 0;

  for (int i = 0; i < sample_count; i++) {
    sum_adc += analogRead(MQ9_PIN);             // 24. Soma as leituras brutas.
    Serial.print(".");
    delay(500); 
  }

  R0_Bruto = (float)sum_adc / sample_count;     // 25. Calcula a média (o R0 de base).
  
  Serial.print("\n>>> Calibração Concluída. Média ADC Bruta (R0): ");
  Serial.println(R0_Bruto);
  Serial.println("=========================================");
}

float calculatePPM(int normalized_adc) {
  
  float R0_Normalizado = (float)mapInvertedValue(R0_Bruto); // 26. Normaliza a base R0.
  if (R0_Normalizado < 1.0) R0_Normalizado = 1.0; 
  
  float ratio = (float)normalized_adc / R0_Normalizado; // 27. Calcula a razão (Atual/Base).
  float PPM = 0;
  
  if (ratio > 1.0) { 
      // 28. Fórmula empírica de PPM: faz o PPM aumentar exponencialmente com a razão.
      PPM = 5.0 * pow(ratio, 2.5);
      if (PPM > 50000.0) PPM = 50000.0;
  }
  
  return PPM;                                   // 29. Retorna o valor estimado em PPM.
}


// ===================================
// Função de Envio HTTP
// ===================================

void sendDataToAPI(float ppm_value) {
  if (WiFi.status() != WL_CONNECTED) {          // 30. Verifica se o Wi-Fi está ativo.
    Serial.println("ERRO: Wi-Fi desconectado. Pulando envio.");
    return;
  }
  
  HTTPClient http; 
  http.begin(api_url);                          // 31. Configura a URL de destino.
  http.addHeader("Content-Type", "application/json"); // 32. Informa que o corpo da mensagem é JSON.

  // 33. Início da construção do payload JSON.
  String payload = "{";
  // 34. MAC Address.
  payload += "\"mac\": \"" + macAddress + "\","; 
  // 35. Valor PPM no campo "sensorValue" (com 2 casas decimais).
  payload += "\"sensorValue\": " + String(ppm_value, 2) + ","; 
  // 36. Tipo de gás no campo "gasType".
  payload += "\"gasType\": \"" + String(GAS_TYPE) + "\""; 
  payload += "}";                               // 37. Fim do payload.

  Serial.print("Enviando Payload: ");
  Serial.println(payload);

  int httpResponseCode = http.POST(payload);    // 38. Envia a requisição HTTP POST.

  if (httpResponseCode > 0) {                   // 39. Se a resposta for positiva, mostra o código e o corpo.
    Serial.print("Resposta da API (");
    Serial.print(httpResponseCode);
    Serial.print("): ");
    Serial.println(http.getString());
  } else {
    Serial.print("Erro HTTP. Código: ");        // 40. Se houver erro, mostra o código de erro.
    Serial.println(httpResponseCode);
  }

  http.end();                                   // 41. Encerra a conexão HTTP.
}


// ===================================
// Função setup()
// ===================================

void setup() {
  Serial.begin(115200); 
  delay(100);

  pinMode(MQ9_PIN, INPUT);                      // 42. Define o pino do sensor como entrada.
  
  // 43. Executa as etapas de inicialização na ordem:
  connectToWiFi();
  getMacAddress();
  CalibrateR0();
}

// ===================================
// Função loop()
// ===================================

void loop() {
  // 44. Lógica de controle de tempo (envia apenas se o intervalo SEND_INTERVAL passou).
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = millis(); 

    if (WiFi.status() == WL_CONNECTED) {        // 45. Só processa se estiver conectado.
      
      int raw_adc = analogRead(MQ9_PIN);        // 46. Faz a leitura bruta.
      int normalized_adc = mapInvertedValue(raw_adc); // 47. Normaliza a leitura.
      float ppm = calculatePPM(normalized_adc); // 48. Calcula o PPM.
      
      Serial.print("Leitura (PPM): ");          // 49. Mostra o resultado final.
      Serial.print(ppm, 2);
      Serial.print(" -> Enviando...");
      Serial.println();
      
      sendDataToAPI(ppm);                       // 50. Envia os dados para a API.
      
    } else {
      Serial.println("Wi-Fi desconectado. Tentando reconectar...");
      connectToWiFi();                          // 51. Tenta restabelecer a conexão.
    }
  }
}