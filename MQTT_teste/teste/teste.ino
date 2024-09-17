#include <WiFi.h>
#include <PubSubClient.h>

#define WIFI_SSID "apgeovi_5G" // Substitua pelo SSID da sua rede Wi-Fi
#define WIFI_PASSWORD "redeapgeovi" // Substitua pela senha da sua rede Wi-Fi
#define MQTT_SERVER "192.168.0.134" // Substitua pelo IP ou domínio do seu broker MQTT
#define MQTT_PORT 1883 // Substitua pela porta do broker MQTT se for diferente
#define MQTT_TOPIC "test/topic" // Substitua pelo tópico desejado

WiFiClient espClient;
PubSubClient client(espClient);

// Função de callback para eventos MQTT
void callback(char* topic, byte* message, unsigned int length) {
    Serial.print("Mensagem recebida no tópico: ");
    Serial.println(topic);

    Serial.print("Mensagem: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)message[i]);
    }
    Serial.println();
}

// Função para conectar-se ao MQTT
void reconnect() {
    while (!client.connected()) {
        Serial.print("Tentando conectar ao MQTT...");
        if (client.connect("ESP32Client")) {
            Serial.println("Conectado ao broker MQTT");
            client.subscribe(MQTT_TOPIC);
        } else {
            Serial.print("Falha ao conectar, rc=");
            Serial.print(client.state());
            Serial.println(" Tentar novamente em 5 segundos");
            delay(5000);
        }
    }
}

// Função para conectar-se ao Wi-Fi
void wifi_init() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando ao Wi-Fi...");
    }
    Serial.println("Wi-Fi conectado!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
}

// Função de setup
void setup() {
    Serial.begin(115200);

    // Inicializa o Wi-Fi
    wifi_init();

    // Configura o servidor MQTT
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback);
}

// Função de loop principal
void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}
