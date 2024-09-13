#include <Arduino.h>
#include "ESP32ZabbixSender.h"
#include "M5StickCPlusUtils.h"

ESP32ZabbixSender zSender;

/* conf WiFi */
String ssid = "SEU_SSID";
String pass = "SUA_SENHA";

/* conf do servidor Zabbix */
#define SERVERADDR 10,7,221,242// endereço ip do servidor Zabbix
#define ZABBIXPORT 10051			// porta do servidor Zabbix
#define ZABBIXAGHOST "node"  // nome do host do item Zabbix

boolean checkConnection();
void zabbixTask(float rms, int8_t doorState);

void setup() {
	Serial.begin(115200);
	setupM5StickCPlus(((void*)zabbixTask));
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);
	WiFi.begin(ssid.c_str(), pass.c_str());
	while (!checkConnection()) {
	}
	zSender.Init(IPAddress(SERVERADDR), ZABBIXPORT, ZABBIXAGHOST); // Inicia a informação do Zabbix
	
}

void loop() {
	vTaskDelay(1000/portTICK_RATE_MS);
}

void zabbixTask(float rms, int8_t doorState) {
	zSender.ClearItem();						  // Limpa a lista de items do zabbix
	zSender.AddItem("sound", rms); 	  // som
	if (zSender.Send() == EXIT_SUCCESS) {		  // Envia os items para o zabbix
		Serial.println("ZABBIX SEND: OK");
	} else {
		Serial.println("ZABBIX SEND: NG");
	}
	zSender.ClearItem();
	zSender.AddItem("door", (float)doorState); 	  // porta
	if (zSender.Send() == EXIT_SUCCESS) {		  // Envia os items para o zabbix
		Serial.println("ZABBIX SEND: OK");
	} else {
		Serial.println("ZABBIX SEND: NG");
	}
}
boolean checkConnection() {
	int count = 0;
	Serial.print("Waiting for Wi-Fi connection");
	while (count < 300) {
		if (WiFi.status() == WL_CONNECTED) {
			Serial.println();
			Serial.println("Connected!");
			return (true);
		}
		delay(500);
		Serial.print(".");
		count++;
	}
	Serial.println("Timed out.");
	return false;
}