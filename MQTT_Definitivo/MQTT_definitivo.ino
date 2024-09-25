#include <WiFi.h>
#include <PubSubClient.h>
#include "HX711.h"  // Biblioteca para o sensor de peso HX711
#include <ArduinoJson.h>  // Biblioteca para manipulação de JSON

// Configurações Wi-Fi
const char* ssid = "apgeovi";
const char* password = "redeapgeovi72";

// Configurações do broker MQTT
const char* mqtt_server = "187.35.47.129";
const int mqtt_port = 1883;
const char* mqtt_user = "arthur";
const char* mqtt_password = "IYZFD10eq7ns1M2CDkiW1xuo38KRId6bE";

// Pinos do sensor HX711
const int LOADCELL_DOUT_PIN = 4;  // Pino DOUT do HX711
const int LOADCELL_SCK_PIN = 5;   // Pino SCK do HX711

WiFiClient espClient;
PubSubClient client(espClient);
HX711 scale;  // Instância do sensor HX711

long lastMsg = 0;
char msg[200];  // Buffer para enviar mensagens MQTT

// Configura o Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Reconexão ao broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando se conectar ao broker MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado");
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void scanWiFi() {
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("Nenhuma rede Wi-Fi encontrada.");
  } else {
    Serial.printf("%d redes encontradas:\n", n);
    
    // Buffer para armazenar a mensagem JSON
    String json = "[";
    
    for (int i = 0; i < (n > 3 ? 3 : n); ++i) {  // Limitar a 3 redes
      Serial.printf("SSID: %s | RSSI: %d\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
      
      // Adiciona o SSID e RSSI ao JSON
      json += String("{\"SSID\":\"") + WiFi.SSID(i).c_str() + "\", \"RSSI\":" + String(WiFi.RSSI(i)) + "}";
      if (i < (n > 3 ? 3 : n) - 1) {
        json += ",";  // Adiciona vírgula entre os objetos
      }
    }
    
    json += "]";  // Fecha o array JSON
    
    // Publica os dados da rede Wi-Fi no MQTT
    snprintf(msg, 200, "%s", json.c_str());
    client.publish("wifi/topico", msg);
  }
}

// Função para medir o peso com HX711
void measureWeight() {
  if (scale.is_ready()) {
    long weight = scale.get_units(5);
    Serial.print("Peso detectado: ");
    Serial.println(weight);

    // Cria um objeto JSON para a mensagem
    StaticJsonDocument<200> doc;
    doc["status"] = (weight > 100) ? "Ocupado" : "Livre";
    
    // Serializa o objeto JSON para string
    serializeJson(doc, msg);
    
    client.publish("balanca/topico", msg);  // Publica o estado da balança no MQTT
  } else {
    Serial.println("HX711 não está pronto.");
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  // Configuração do HX711
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();    // Defina a escala de calibração
  scale.tare();         // Zera a balança
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000) {  // Envia a cada 10 segundos
    lastMsg = now;
    
    // Escanear redes Wi-Fi
    scanWiFi();
    
    // Medir o peso da balança
    measureWeight();
  }
}
