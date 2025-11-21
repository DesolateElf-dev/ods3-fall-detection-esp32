# 🚨 Sistema de Detecção de Quedas - ODS3

![ESP32](https://img.shields.io/badge/ESP32-DevKit%20C-blue)
![MQTT](https://img.shields.io/badge/Protocol-MQTT-orange)
![Status](https://img.shields.io/badge/Status-Funcional-success)
![License](https://img.shields.io/badge/License-MIT-green)

Sistema IoT para **detecção automática de quedas** em idosos, alinhado aos Objetivos de Desenvolvimento Sustentável (ODS3 - Saúde e Bem-Estar) da ONU.

<img width="621" height="564" alt="Diagrama do Circuito" src="https://github.com/user-attachments/assets/ff404eda-b206-4d5e-8520-a11bfb294903" />

## 📚 Sobre o Projeto

Este projeto foi desenvolvido como trabalho acadêmico para demonstrar o uso de tecnologias IoT na promoção da saúde e segurança de idosos. O dispositivo monitora continuamente movimentos através de um acelerômetro e envia alertas via MQTT quando detecta quedas ou quando o usuário aciona o botão de pânico.

### ✨ Funcionalidades

- ✅ **Detecção automática de quedas** via acelerômetro MPU6050
- ✅ **Botão de pânico manual** para solicitação de ajuda
- ✅ **Alerta sonoro local** com buzzer piezoelétrico
- ✅ **Comunicação MQTT** para integração com sistemas de monitoramento
- ✅ **Código documentado** e de fácil personalização
- ✅ **Simulação online** disponível no Wokwi

## 🔧 Hardware Necessário

| Componente | Descrição | Quantidade |
|------------|-------------|------------|
| ESP32 DevKit C V4 | Microcontrolador com WiFi/BLE | 1 |
| MPU6050 | Acelerômetro + Giroscópio (I2C) | 1 |
| Buzzer KY-012 | Buzzer piezoelétrico ativo | 1 |
| Botão Push Button | Botão momentâneo 6mm | 1 |
| Protoboard | 400 ou 830 pontos | 1 |
| Jumpers | Fios de conexão macho-macho | ~10 |

### 🔌 Pinagem

```
ESP32          MPU6050
-----          -------
3V3     ---    VCC
GND     ---    GND
GPIO21  ---    SDA
GPIO22  ---    SCL

ESP32          Buzzer KY-012
-----          -------------
GPIO25  ---    Signal (Pino 2)
GND     ---    GND (Pino 1)

ESP32          Botão
-----          -----
GPIO18  ---    Pino 1
GND     ---    Pino 2
```

## 💻 Software e Bibliotecas

### Dependências (Arduino IDE / PlatformIO)

```cpp
// Instale via Library Manager:
- WiFi (nativa ESP32)
- PubSubClient (Nick O'Leary)
- Adafruit MPU6050
- Adafruit Unified Sensor
```

### Configuração do Ambiente

1. **Arduino IDE**:
   - Adicione suporte ao ESP32: File > Preferences > Additional Board URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Tools > Board > ESP32 Arduino > ESP32 Dev Module

2. **PlatformIO** (platformio.ini):
   ```ini
   [env:esp32dev]
   platform = espressif32
   board = esp32dev
   framework = arduino
   lib_deps = 
       knolleary/PubSubClient@^2.8
       adafruit/Adafruit MPU6050@^2.2.4
       adafruit/Adafruit Unified Sensor@^1.1.9
   ```

## 🚀 Como Usar

### 1️⃣ Montagem Física

- Conecte os componentes conforme o diagrama `hardware/diagram.json`
- Importe o arquivo no [Wokwi](https://wokwi.com) para simulação online

### 2️⃣ Configuração do Código

Edite em `src/main.cpp`:

```cpp
// Credenciais WiFi
const char* ssid = "SEU_SSID";          // Troque pelo nome da sua rede
const char* password = "SUA_SENHA";     // Troque pela senha

// Personalização de tópicos MQTT
const char* topic_alert = "ods3/fall/SEU_ID/alert";   // Troque "SEU_ID"
```

### 3️⃣ Upload e Testes

```bash
# Arduino IDE:
# 1. Abra src/main.cpp
# 2. Selecione a porta COM do ESP32
# 3. Clique em Upload

# PlatformIO:
pio run --target upload
pio device monitor  # Abre monitor serial
```

### 4️⃣ Monitoramento MQTT

Para visualizar mensagens publicadas:

```bash
# Usando mosquitto_sub (Linux/Mac/Windows com WSL)
mosquitto_sub -h test.mosquitto.org -t "ods3/fall/#" -v

# Ou use clientes gráficos:
# - MQTT Explorer (https://mqtt-explorer.com/)
# - HiveMQ Web Client (https://www.hivemq.com/demos/websocket-client/)
```

## 📊 Mensagens MQTT

### Formato JSON das Mensagens

**Queda detectada:**
```json
{
  "device_id": "usuarioX",
  "event": "fall"
}
```

**Botão de pânico acionado:**
```json
{
  "device_id": "usuarioX",
  "event": "panic"
}
```

## ⚙️ Parâmetros de Detecção

### Ajuste de Sensibilidade

No arquivo `src/main.cpp`, linha ~180:

```cpp
// Threshold atual: 25 m/s² (~2.5G)
else if (mag > 25.0) {  
    // Detecção de queda
}

// Ajustes sugeridos:
// - Mais sensível (detecta movimentos menores): mag > 20.0
// - Menos sensível (só quedas bruscas): mag > 30.0
```

### Valores de Referência

| Atividade | Aceleração Típica |
|-----------|----------------------|
| Repouso | ~9.81 m/s² (1G) |
| Caminhada | 12-15 m/s² |
| Corrida | 15-20 m/s² |
| **Queda** | **> 25 m/s² (2.5G)** |

## 📝 Estrutura do Projeto

```
ods3-fall-detection-esp32/
├── src/
│   └── main.cpp              # Código principal (comentado)
├── hardware/
│   └── diagram.json          # Diagrama Wokwi do circuito
├── README.md                 # Este arquivo
└── LICENSE                   # Licença MIT
```

## 🔗 Links Úteis

- **Simulação Online**: [Wokwi](https://wokwi.com) (importe `hardware/diagram.json`)
- **Broker MQTT Público**: [test.mosquitto.org](https://test.mosquitto.org)
- **Documentação MPU6050**: [Adafruit MPU6050](https://learn.adafruit.com/mpu6050-6-dof-accelerometer-and-gyro)
- **MQTT Client**: [MQTT Explorer](https://mqtt-explorer.com/)

## 🛡️ Limitações e Considerações

⚠️ **Importante:**
- Este é um **projeto educacional/protótipo**
- **Não deve ser usado como substituto** de dispositivos médicos certificados
- Brokers MQTT públicos **não garantem privacidade** de dados
- Para uso real, implemente:
  - Autenticação MQTT (TLS)
  - Broker privado
  - Algoritmos mais robustos (machine learning)
  - Notificações móveis (Telegram/WhatsApp)

## 👥 Contribuindo

Contribuições são bem-vindas! Sinta-se à vontade para:

1. Fazer fork do projeto
2. Criar uma branch para sua feature (`git checkout -b feature/MinhaFeature`)
3. Commit suas mudanças (`git commit -m 'Add: MinhaFeature'`)
4. Push para a branch (`git push origin feature/MinhaFeature`)
5. Abrir um Pull Request

## 📄 Licença

Este projeto está sob a licença **MIT**. Veja o arquivo `LICENSE` para mais detalhes.

## ✍️ Autor

**Eduardo Maciel Sanchez**
- GitHub: [@DesolateElf-dev](https://github.com/DesolateElf-dev)
- Projeto: Trabalho acadêmico - Sistemas Embarcados / IoT
- Data: Novembro 2025

---

<div align="center">
  
### 🌐 Alinhado aos Objetivos de Desenvolvimento Sustentável da ONU

**ODS 3**: Saúde e Bem-Estar - Assegurar uma vida saudável e promover o bem-estar para todos

<img src="https://upload.wikimedia.org/wikipedia/commons/thumb/c/c8/Sustainable_Development_Goal_03.png/150px-Sustainable_Development_Goal_03.png" alt="ODS 3" width="100">

</div>
