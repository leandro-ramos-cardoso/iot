#include <WiFi.h>     
#include <HTTPClient.h> 
#include <MQUnifiedsensor.h>
#include <math.h> 

const char* ssid = "Starlink IlhaTech 2.4";    
const char* password = "Bemvind@s1"; 
String macAddress;
const char* api_url = "http://192.168.2.152:8080/device/data";

void connectToWiFi() {
  Serial.print("Conectando a ");                
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);                          
  WiFi.begin(ssid, password);                   

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) { 
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {       
    Serial.println("\nWiFi conectado!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha na conexão Wi-Fi.");
  }
}

void getMacAddress() {
  macAddress = WiFi.macAddress();            
  Serial.print("MAC Address: ");
  Serial.println(macAddress);
}

void sendDataToAPI(float ppm_value) {
  if (WiFi.status() != WL_CONNECTED) {          // Verifica se o Wi-Fi está ativo.
    Serial.println("ERRO: Wi-Fi desconectado. Pulando envio.");
    return;
  }
  
  HTTPClient http; 
  http.begin(api_url);                          // Configura a URL de destino.
  http.addHeader("Content-Type", "application/json"); // Informa que o corpo da mensagem é JSON.

 
  String payload = "{";
  payload += "\"mac\": \"" + macAddress + "\","; 
  payload += "\"sensorValue\": " + String(ppm_value, 2) + ","; 
  payload += "\"gasType\": \"" + String(GAS_TYPE) + "\""; 
  payload += "}";                   

  Serial.print("Enviando Payload: ");
  Serial.println(payload);

  int httpResponseCode = http.POST(payload);    // Envia a requisição HTTP POST.

  if (httpResponseCode > 0) {                   // Se a resposta for positiva, mostra o código e o corpo.
    Serial.print("Resposta da API (");
    Serial.print(httpResponseCode);
    Serial.print("): ");
    Serial.println(http.getString());
  } else {
    Serial.print("Erro HTTP. Código: ");        // Se houver erro, mostra o código de erro.
    Serial.println(httpResponseCode);
  }

  http.end();                                   
}

// ===============================================
// 1. CONFIGURAÇÕES DO HARDWARE
// ===============================================

const char* Board_Type = "ESP-32";         
const char* Sensor_Type = "MQ-9"; 
const char* GAS_TYPE = "CO";        

// Hardware e Tensões
#define V_ADC_MAX_READ         3.3            
#define V_CIRCUITO_MQ9         5.0            
#define ADC_RESOLUTION_BITS    12             
#define analogPin              36             

// Fórmulas e Calibração
#define RLOAD                  10.0           
// R0 OTIMIZADO PARA CO: Ajustado para que Rs/R0 caia para ~0.35, que é detectável pela curva CO
//#define TEST_R0_VALUE          4000.0         // R0 de Teste Fixo em kOhm.
// OTIMIZAÇÃO FINAL: Aumenta R0 para 20000 kOhm para forçar o resultado para ~5 ppm em ar limpo.
#define TEST_R0_VALUE          150.0        // R0 de Teste Fixo em kOhm.

// MUDANÇA: Coeficientes da Curva de Monóxido de Carbono (CO)
#define CO_a                   58.74        
#define CO_b                   -1.6         


// ===============================================
// 2. CRIAÇÃO DO OBJETO SENSOR E SETUP
// ===============================================

MQUnifiedsensor MQ9(Board_Type, V_ADC_MAX_READ, ADC_RESOLUTION_BITS, analogPin, Sensor_Type);

void setup() {

  connectToWiFi();
  getMacAddress();

  Serial.begin(115200);
  Serial.println("--- MQ-9: CALCULO MANUAL FORCADO (USANDO CURVA CO) ---");
  
  MQ9.init();
  MQ9.setRL(RLOAD);
  
  // 1. CONFIGURA A CURVA PARA CO
  MQ9.setRegressionMethod(1); 
  MQ9.setA(CO_a);  
  MQ9.setB(CO_b);  

  // 2. APLICA O R0 FORÇADO
  MQ9.setR0(TEST_R0_VALUE); 
  
  Serial.print("R0 de Teste (Para CO): ");
  Serial.print(MQ9.getR0());
  Serial.println(" kOhm.");
  Serial.println("--- Iniciando Leituras de CO em PPM ---");
}


void loop() {
  // 1. Leitura da Tensão Analógica (Vout)
  float raw_Vout = MQ9.getVoltage();
  
  // 2. Cálculo de Rs (Resistência do Sensor)
  float rs_value = (V_CIRCUITO_MQ9 / raw_Vout - 1) * RLOAD;

  // 3. Cálculo da Razão Rs/R0
  float rs_r0_ratio = rs_value / MQ9.getR0();
  
  // 4. CALCULO DO PPM MANUALMENTE (Para CO)
  float log_Rs_R0 = log10(rs_r0_ratio);
  float log_PPM = (log_Rs_R0 - log10(CO_a)) / CO_b;
  float ppm_value = pow(10, log_PPM);
  
  // Limites de segurança
  if (ppm_value > 1000) ppm_value = 1000;
  if (ppm_value < 0) ppm_value = 0;


  Serial.print("Vout: ");
  Serial.print(raw_Vout, 3);
  Serial.print(" V | Rs/R0: ");
  Serial.print(rs_r0_ratio, 3);
  Serial.print(" | CO: ");
  Serial.print(ppm_value, 2); 
  Serial.println(" ppm");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(" -> Enviando...");
    Serial.println();

    sendDataToAPI(ppm_value);
  } else {
    Serial.println("Wi-Fi desconectado. Tentando reconectar...");
    connectToWiFi();
  }

  

  delay(300000); 
}