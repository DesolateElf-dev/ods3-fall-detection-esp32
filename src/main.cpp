/**
 * Sistema de Detecção de Quedas - Projeto ODS3
 * 
 * Descrição:
 * Sistema IoT para detecção de quedas em idosos utilizando ESP32,
 * acelerômetro MPU6050 e comunicação via protocolo MQTT.
 * 
 * Componentes:
 * - ESP32 DevKit C V4
 * - MPU6050 (acelerômetro/giroscópio I2C)
 * - Buzzer KY-012 (alerta sonoro)
 * - Botão 6mm (botão de pânico)
 * 
 * Autor: Eduardo Maciel Sanchez
 * Data: Novembro 2025
 * Licença: MIT
 */

// ============================================================================
// BIBLIOTECAS
// ============================================================================

#include <WiFi.h>                // Conexão WiFi do ESP32
#include <PubSubClient.h>        // Cliente MQTT para publicação/subscrição
#include <Wire.h>                // Comunicação I2C
#include <Adafruit_MPU6050.h>    // Driver do sensor MPU6050
#include <Adafruit_Sensor.h>     // Biblioteca base de sensores Adafruit

// ============================================================================
// DEFINIÇÕES DE HARDWARE
// ============================================================================

#define BUZZER_PIN 25      // Pino do buzzer piezoelétrico (saída PWM)
#define BUTTON_PIN 18      // Pino do botão de pânico (entrada pull-up)

// Pinos I2C (padrão ESP32):
// SDA = GPIO 21
// SCL = GPIO 22

// ============================================================================
// CONFIGURAÇÕES DE REDE E MQTT
// ============================================================================

// Credenciais WiFi (Wokwi usa rede padrão sem senha)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configurações do broker MQTT público
const char* mqtt_server = "test.mosquitto.org";  // Broker público Eclipse
const int mqtt_port = 1883;                       // Porta MQTT padrão (sem TLS)
const char* mqtt_user = "";                       // Conexão anônima
const char* mqtt_password = "";                   // Sem autenticação

// Tópicos MQTT (personalize "usuarioX" para identificar dispositivos)
const char* topic_alert = "ods3/fall/usuarioX/alert";    // Publicação de alertas
const char* topic_status = "ods3/fall/usuarioX/status";  // Status do dispositivo

// ============================================================================
// OBJETOS GLOBAIS
// ============================================================================

WiFiClient espClient;              // Cliente WiFi
PubSubClient client(espClient);     // Cliente MQTT
Adafruit_MPU6050 mpu;               // Objeto do sensor MPU6050

// ============================================================================
// FUNÇÃO: Reconectar ao broker MQTT
// ============================================================================
/**
 * Tenta reconectar ao broker MQTT caso a conexão seja perdida.
 * Loop bloqueante até estabelecer conexão.
 */
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    
    // Tenta conectar com ID único
    if (client.connect("esp32-client-ods3")) {
      Serial.println(" conectado!");
      
      // Subscreve ao tópico de comandos (para controle remoto futuro)
      client.subscribe("ods3/fall/usuarioX/cmd");
    } else {
      Serial.print(" falhou, rc=");
      Serial.println(client.state());
      delay(2000);  // Aguarda antes de tentar novamente
    }
  }
}

// ============================================================================
// FUNÇÃO: Setup (inicialização)
// ============================================================================
void setup() {
  // Inicializa comunicação serial para debug
  Serial.begin(115200);
  
  // Configura pinos de entrada/saída
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Pull-up interno ativado
  
  // -------------------------------------------------------------------------
  // Conexão WiFi
  // -------------------------------------------------------------------------
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // -------------------------------------------------------------------------
  // Configuração do cliente MQTT
  // -------------------------------------------------------------------------
  client.setServer(mqtt_server, mqtt_port);
  
  // -------------------------------------------------------------------------
  // Inicialização do MPU6050
  // -------------------------------------------------------------------------
  Wire.begin(21, 22);  // Inicializa I2C nos pinos padrão
  Serial.print("Inicializando MPU6050...");
  
  if (!mpu.begin()) {
    Serial.println(" falhou!");
    Serial.println("Verifique as conexões do sensor.");
    while (1) { delay(1000); }  // Trava execução se sensor não for encontrado
  }
  Serial.println(" OK!");
  
  // Configurações do sensor (ranges padrão)
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  Serial.println("Sistema iniciado!");
}

// ============================================================================
// FUNÇÃO: Emitir beep de alerta
// ============================================================================
/**
 * Emite um beep curto no buzzer (2 kHz por 200ms)
 */
void beep() {
  tone(BUZZER_PIN, 2000, 200);
}

// ============================================================================
// FUNÇÃO: Loop principal
// ============================================================================
void loop() {
  // Garante conexão MQTT ativa
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();  // Processa mensagens MQTT
  
  // -------------------------------------------------------------------------
  // LEITURA DO ACELERÔMETRO MPU6050
  // -------------------------------------------------------------------------
  sensors_event_t a, g, temp;  // Eventos de aceleração, giroscópio e temperatura
  mpu.getEvent(&a, &g, &temp);
  
  // Calcula magnitude da aceleração (m/s²)
  // Formula: sqrt(x² + y² + z²)
  float mag = sqrt(a.acceleration.x * a.acceleration.x +
                   a.acceleration.y * a.acceleration.y +
                   a.acceleration.z * a.acceleration.z);
  
  // Debug: imprime valores em tempo real (comente para reduçir overhead)
  Serial.printf("Aceleração: X=%.2f Y=%.2f Z=%.2f | Magnitude=%.2f m/s²\n",
                a.acceleration.x, a.acceleration.y, a.acceleration.z, mag);
  
  // -------------------------------------------------------------------------
  // DETECÇÃO DE BOTÃO DE PÂNICO (com debounce por borda)
  // -------------------------------------------------------------------------
  static bool lastButtonState = HIGH;  // Estado anterior do botão
  bool buttonState = digitalRead(BUTTON_PIN);
  
  // Detecta borda de descida (pressão do botão)
  bool panic = (buttonState == LOW && lastButtonState == HIGH);
  lastButtonState = buttonState;
  
  // -------------------------------------------------------------------------
  // PROCESSAMENTO DE ALERTAS
  // -------------------------------------------------------------------------
  
  // Caso 1: Botão de pânico pressionado
  if (panic) {
    beep();
    String payload = "{\"device_id\":\"usuarioX\",\"event\":\"panic\"}";
    client.publish(topic_alert, payload.c_str());
    Serial.println("[ALERTA DE PÂNICO] " + payload);
    delay(1000);  // Evita múltiplos envios
  }
  
  // Caso 2: Queda detectada (magnitude > 25 m/s²)
  // Valor de referência: gravidade = 9.81 m/s²
  // Quedas geram picos > 2.5G (~25 m/s²)
  else if (mag > 25.0) {
    beep();
    String payload = "{\"device_id\":\"usuarioX\",\"event\":\"fall\"}";
    client.publish(topic_alert, payload.c_str());
    Serial.println("[QUEDA DETECTADA] " + payload);
    delay(1000);  // Evita múltiplos envios consecutivos
  }
  
  delay(50);  // Taxa de amostragem: 20Hz (50ms entre leituras)
}
