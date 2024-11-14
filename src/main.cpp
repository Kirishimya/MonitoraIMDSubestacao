#include <Arduino.h>
#include "ESP32ZabbixSender.h"
#include "M5StickCPlusUtils.h"
#include <ESPmDNS.h>
#define ESPHOSTNAME "monitorasubestacao0imd"
ESP32ZabbixSender zSender;

/* conf WiFi */
String ssid = "ssid";
String pass = "pass";

/* conf do servidor Zabbix */
#define SERVERADDR 10, 7, 8, 19 // endereço ip do servidor Zabbix
#define ZABBIXPORT 10051		// porta do servidor Zabbix
#define ZABBIXAGHOST "node"		// nome do host do item Zabbix

boolean checkConnection();
void configuraMDNS();
void zabbixTask(float rms, int8_t doorState);
void setup()
{
	Serial.begin(115200);
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);
	WiFi.begin(ssid.c_str(), pass.c_str());
	while (!checkConnection())
	{
	}
	configuraMDNS();
	Serial.println("ESP32 IP: " + WiFi.localIP().toString());
	Serial.println("ESP32 gateway IP: " + WiFi.gatewayIP().toString());

	zSender.Init(IPAddress(SERVERADDR), ZABBIXPORT, ZABBIXAGHOST); // Inicia a informação do Zabbix
	setupM5StickCPlus(((void *)zabbixTask));
}

void loop()
{
	vTaskDelay(1000 / portTICK_PERIOD_MS);
}
void configuraMDNS()
{
	if (!MDNS.begin(ESPHOSTNAME))
	{
		Serial.println("Falha em resolver mDNS");
		while (1)
		{
		}
	}
}
void zabbixTask(float rms, int8_t doorState)
{
	unsigned long tries = 0;
	do
	{
		if (tries > 15)
			break;
		Serial.printf("Try (%d):\n", 1 + tries++);
		zSender.ClearItem();					   // Limpa a lista de items do zabbix
		zSender.AddItem("sound", rms);			   // som
		zSender.AddItem("door", (float)doorState); // porta
	} while (zSender.Send() != EXIT_SUCCESS);
	Serial.println("ZABBIX SEND: OK");
	// zSender.ClearItem();

	// if (zSender.Send() == EXIT_SUCCESS) {		  // Envia os items para o zabbix
	// 	Serial.println("ZABBIX SEND: OK");
	// }
}
boolean checkConnection()
{
	int count = 0;
	Serial.print("Waiting for Wi-Fi connection");
	while (count < 300)
	{
		if (WiFi.status() == WL_CONNECTED)
		{
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
