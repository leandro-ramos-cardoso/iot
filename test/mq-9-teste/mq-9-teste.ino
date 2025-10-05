#include <math.h>    // Necessário para a função pow()

// ===================================
// 1. Definições de Pinos e Constantes
// ===================================

// Pino GPIO do ESP32 conectado à saída analógica (A0) do sensor MQ-9.
const int MQ9_PIN = 4; 

// O valor máximo do ADC do ESP32 (12 bits)
const int ADC_MAX = 4095; 

// ===================================
// 2. Variáveis de Calibração
// ===================================

// R0 (Resistência em Ar Limpo) - Armazenará a média ADC bruta de base.
float R0_Bruto = 0; 

// ===================================
// Funções Auxiliares
// ===================================

// ----------------------------------------------------
// A. CORREÇÃO DA LEITURA INVERTIDA
// ----------------------------------------------------
// Mapeia o valor bruto invertido (0-4095) para que o gás ALTO resulte
// em um número ALTO, corrigindo o comportamento do seu módulo.
int mapInvertedValue(int raw_value) {
  // A inversão: Valor Normalizado = (Valor Máximo) - (Valor Bruto Atual)
  // Ex: 4095 - 550 (ar limpo) = 3545 (sinal baixo)
  // Ex: 4095 - 250 (com gás) = 3845 (sinal alto)
  int inverted_value = ADC_MAX - raw_value;
  
  return inverted_value;
}


// ----------------------------------------------------
// B. CALIBRAÇÃO (Cálculo do R0 de Base)
// ----------------------------------------------------
// Executa no setup para encontrar o R0 (valor ADC bruto em ar limpo)
void CalibrateR0() {
  Serial.println("=========================================");
  Serial.println("Calibrando R0... Mantenha o sensor em ar limpo por 30s.");
  
  int sample_count = 50;
  long sum_adc = 0;
  
  // Coleta 50 amostras do sensor a cada 500ms
  for (int i = 0; i < sample_count; i++) {
    sum_adc += analogRead(MQ9_PIN);
    Serial.print(".");
    delay(500); 
  }

  // Calcula a média das leituras brutas
  R0_Bruto = (float)sum_adc / sample_count;
  
  Serial.print("\n>>> Calibração Concluída. Média ADC Bruta (R0): ");
  Serial.println(R0_Bruto);
  Serial.println("=========================================");
}


// ----------------------------------------------------
// C. CONVERSÃO PARA PPM (ESTIMADO)
// ----------------------------------------------------
// Calcula o PPM usando o valor normalizado e a base (R0)
float calculatePPM(int normalized_adc) {
  
  // Normaliza o valor de R0 para ter a base na mesma escala
  float R0_Normalizado = (float)mapInvertedValue(R0_Bruto); 

  // Garante que o R0 não seja zero, o que causaria erro de divisão
  if (R0_Normalizado < 1.0) R0_Normalizado = 1.0; 
  
  // Razão: (Normalizado Atual / Normalizado Base)
  // Se > 1.0, o nível de gás está acima do ar limpo (base)
  float ratio = (float)normalized_adc / R0_Normalizado; 
  
  float PPM = 0;
  
  if (ratio > 1.0) { 
      // Fórmula de Calibração Empírica (Apenas para teste de lógica!):
      // Esta fórmula faz o PPM aumentar com o gás.
      PPM = 5.0 * pow(ratio, 2.5);
      
      // Limite máximo para evitar números estranhos em picos
      if (PPM > 50000.0) PPM = 50000.0;
  }
  
  return PPM;
}


// ===================================
// Função setup()
// ===================================
void setup() {
  Serial.begin(115200); 
  delay(100);

  pinMode(MQ9_PIN, INPUT); 
  
  // 1. Executa a calibração da base
  CalibrateR0();
}

// ===================================
// Função loop()
// ===================================
void loop() {
  // 1. Faz a leitura bruta
  int raw_adc = analogRead(MQ9_PIN);
  
  // 2. CORRIGE: Normaliza o sinal para que ele aumente com o gás
  int normalized_adc = mapInvertedValue(raw_adc);
  
  // 3. Converte o sinal normalizado para PPM estimado
  float PPM = calculatePPM(normalized_adc);
  
  // 4. Exibe os resultados
  Serial.print("ADC Bruto: ");
  Serial.print(raw_adc);
  Serial.print(" | Sinal Normalizado: ");
  Serial.print(normalized_adc);
  Serial.print(" | PPM Estimado: ");
  Serial.println(PPM, 2); 

  delay(500); 
}