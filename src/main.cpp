/**
 * @file main.cpp
 * @brief Código para ESP32 monitorar parâmetros e enviar dados para servidor Zabbix.
 * 
 * Este código configura um dispositivo ESP32 para se conectar a uma rede Wi-Fi, 
 * configurar mDNS, coletar dados de sensores (som e estado de porta) e enviá-los 
 * para um servidor Zabbix para monitoramento remoto.
 * 
 * @author Gabriel Rossine
 * @date 2024-11-14
 */

#include <Arduino.h>
#include "ESP32ZabbixSender.h"
#include "M5StickCPlusUtils.h"
#include <ESPmDNS.h>

#define ESPHOSTNAME "monitorasubestacao0imd" ///< Nome do dispositivo para mDNS
ESP32ZabbixSender zSender; ///< Instância do objeto para enviar dados ao Zabbix

/* Configurações da rede Wi-Fi */
String ssid = "ssid"; ///< SSID da rede Wi-Fi
String pass = "pass"; ///< Senha da rede Wi-Fi

/* Configuração do servidor Zabbix */
#define SERVERADDR 10, 7, 8, 19 ///< Endereço IP do servidor Zabbix
#define ZABBIXPORT 10051 ///< Porta do servidor Zabbix
#define ZABBIXAGHOST "node" ///< Nome do host do item Zabbix

/**
 * @brief Função para verificar a conexão Wi-Fi.
 * 
 * A função tenta se conectar à rede Wi-Fi até 300 tentativas (150 segundos).
 * 
 * @return true Se a conexão Wi-Fi for bem-sucedida.
 * @return false Se a conexão falhar após 300 tentativas.
 */
boolean checkConnection() {
    int count = 0;
    Serial.print("Waiting for Wi-Fi connection");
    while (count < 300) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println();
            Serial.println("Connected!");
            return true;
        }
        delay(500);
        Serial.print(".");
        count++;
    }
    Serial.println("Timed out.");
    return false;
}

/**
 * @brief Função para configurar o mDNS.
 * 
 * Configura o nome de host do dispositivo para que ele possa ser acessado via nome 
 * na rede local, sem precisar saber o IP exato.
 * 
 * @note Esta função entra em loop infinito em caso de falha.
 */
void configuraMDNS() {
    if (!MDNS.begin(ESPHOSTNAME)) {
        Serial.println("Falha em resolver mDNS");
        while (1) { }  ///< Loop infinito em caso de falha no mDNS
    }
}

/**
 * @brief Função para enviar dados ao servidor Zabbix.
 * 
 * Envia os valores de som (rms) e estado da porta (doorState) para o servidor Zabbix.
 * Tenta enviar até 15 vezes em caso de falha.
 * 
 * @param rms Valor do som (Root Mean Square).
 * @param doorState Estado da porta (0 para fechada, 1 para aberta).
 */
void zabbixTask(float rms, int8_t doorState) {
    unsigned long tries = 0;
    do {
        if (tries > 15) break;  ///< Limite de tentativas
        Serial.printf("Try (%d):\n", 1 + tries++);
        zSender.ClearItem();  ///< Limpa a lista de itens do Zabbix
        zSender.AddItem("sound", rms);  ///< Adiciona o item "sound" com o valor rms
        zSender.AddItem("door", (float)doorState);  ///< Adiciona o item "door" com o estado da porta
    } while (zSender.Send() != EXIT_SUCCESS);  ///< Tenta enviar até obter sucesso
    Serial.println("ZABBIX SEND: OK");
}

/**
 * @brief Função de configuração inicial.
 * 
 * Executada uma vez ao iniciar o ESP32. Conecta-se à rede Wi-Fi, configura o mDNS, 
 * inicializa a comunicação com o servidor Zabbix e configura o dispositivo M5StickCPlus.
 */
void setup() {
    Serial.begin(115200);  ///< Inicializa a comunicação serial
    WiFi.mode(WIFI_STA);    ///< Configura o ESP32 para atuar como estação Wi-Fi (cliente)
    WiFi.disconnect();      ///< Desconecta de qualquer rede Wi-Fi anterior
    delay(100); 
    WiFi.begin(ssid.c_str(), pass.c_str());  ///< Tenta conectar à rede Wi-Fi
    while (!checkConnection()) { }  ///< Aguarda a conexão Wi-Fi
    configuraMDNS();  ///< Configura o mDNS para acesso via nome na rede local
    Serial.println("ESP32 IP: " + WiFi.localIP().toString());  ///< Exibe o IP local do ESP32
    Serial.println("ESP32 gateway IP: " + WiFi.gatewayIP().toString());  ///< Exibe o IP do gateway
    
    zSender.Init(IPAddress(SERVERADDR), ZABBIXPORT, ZABBIXAGHOST);  ///< Inicializa o ZabbixSender
    setupM5StickCPlus(((void *)zabbixTask));  ///< Configura o dispositivo M5StickCPlus
}

/**
 * @brief Função principal de execução.
 * 
 * O loop principal apenas mantém o dispositivo ativo e esperando.
 * Neste código, a tarefa de monitoramento e envio dos dados ocorre em outras funções.
 */
void loop() {
    vTaskDelay(1000 / portTICK_PERIOD_MS);  ///< Aguarda 1 segundo (1000 ms)
}
