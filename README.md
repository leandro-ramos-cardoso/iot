# 🏭 Monitor de Qualidade do Ar com MQ-9 e ESP32

Este projeto utiliza um microcontrolador **ESP32** para ler os dados do sensor de gás **MQ-9** (Monóxido de Carbono - CO e Gás Inflamável) e enviá-los periodicamente para uma API através de uma conexão Wi-Fi.

O foco é fornecer leituras precisas de **Monóxido de Carbono (CO)** em partes por milhão (**ppm**), utilizando calibração manual e cálculos de regressão logarítmica.

---

## 🛠️ Tecnologias e Componentes

| Categoria | Item | Detalhes |
| :--- | :--- | :--- |
| **Hardware** | Placa Principal | **ESP32** (Generic) |
| **Sensor** | Sensor de Gás | **MQ-9** (Recomendado circuito de 5V) |
| **Conectividade** | Rede | Wi-Fi 2.4 GHz |
| **Bibliotecas** | Essenciais | `WiFi.h`, `HTTPClient.h`, `MQUnifiedsensor.h`, `math.h` |

---

## ⚙️ Configuração Inicial

### 1. Requisitos de Biblioteca

Certifique-se de que as seguintes bibliotecas estão instaladas no seu ambiente Arduino IDE:

* **MQUnifiedsensor:** Utilizada para facilitar a interface e manipulação do sensor MQ.
* **WiFi e HTTPClient:** Já inclusas no ESP32 Core.

### 2. Parâmetros de Conexão

Antes de carregar o código, você deve ajustar as variáveis de conexão Wi-Fi e o endereço da API.

| Variável | Descrição | Valor Atual no Código | Onde Alterar |
| :--- | :--- | :--- | :--- |
| `ssid` | Nome da rede Wi-Fi. | `"Starlink IlhaTech 2.4"` | Linha 6 |
| `password` | Senha da rede Wi-Fi. | `"Bemvind@s1"` | Linha 7 |
| `api_url` | Endpoint da API para envio dos dados. | `"http://192.168.2.60:8080/device/data"` | Linha 9 |

### 3. Configuração do Sensor

O código está configurado com valores de calibração específicos para a leitura de **CO** (Monóxido de Carbono):

| Parâmetro | Descrição | Valor | Linha |
| :--- | :--- | :--- | :--- |
| `analogPin` | Pino de entrada analógica do MQ-9 (AOUT). | `36` (VP) | Linha 40 |
| `RLOAD` | Resistência de Carga (RL) no circuito do MQ-9 (em kOhm). | `10.0` | Linha 43 |
| `TEST_R0_VALUE` | Valor de R0 (Resistência em ar limpo) **forçado** para o CO (em kOhm). | `90.0` | Linha 44 |
| `CO_a`, `CO_b` | Parâmetros da curva de regressão logarítmica do CO (`y = a * x^b`). | `58.74`, `-1.6` | Linhas 45-46 |

**NOTA:** O valor de `TEST_R0_VALUE` foi **forçado** no código (`MQ9.setR0(TEST_R0_VALUE);`). Para maior precisão, o valor de R0 deve ser determinado em ar limpo por um processo de calibração adequado.

---

## 💡 Funcionamento do Código

### Estrutura e Funções

O programa é dividido em funções lógicas para garantir clareza:

1.  **`connectToWiFi()`:**
    * Estabelece a conexão do ESP32 à rede Wi-Fi definida. Tenta reconectar a cada **500ms** por até 40 tentativas.
2.  **`getMacAddress()`:**
    * Obtém o endereço **MAC** do ESP32, que é usado como identificador único (`"mac"`) no payload de envio.
3.  **`sendDataToAPI(float ppm_value)`:**
    * Cria um objeto **JSON** com o MAC Address e a leitura do sensor (`ppm_value`).
    * Realiza uma requisição **HTTP POST** para o `api_url` e imprime o código de resposta e o corpo da resposta da API.
4.  **`setup()`:**
    * Inicializa a comunicação serial e a biblioteca do sensor.
    * Chama `connectToWiFi()` e `getMacAddress()`.
    * Configura o sensor MQ-9 com os parâmetros de calibração e o R0 forçado.
5.  **`loop()`:**
    * **Leitura:** Realiza a leitura da tensão (`raw_Vout`) do sensor.
    * **Cálculo:** Calcula a resistência do sensor (`rs_value`), a razão $\frac{Rs}{R0}$, e finalmente o valor em **ppm** usando a fórmula logarítmica com os parâmetros $a$ e $b$ do CO.
        $$ \text{PPM} = 10^{\left(\frac{\log_{10}(\frac{Rs}{R0}) - \log_{10}(a)}{b}\right)} $$
    * **Envio:** Se o Wi-Fi estiver conectado, chama `sendDataToAPI()` e pisca o LED da placa (pino 2) como indicador.
    * **Intervalo:** O ciclo se repete a cada **300 segundos (5 minutos)**, definido pelo `delay(300000)`.

---

## ⚠️ Monitoramento e Logs

Acompanhe o status da conexão, os cálculos e o envio dos dados através do **Monitor Serial** (Baud Rate: **115200**):