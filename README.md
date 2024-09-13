# README

## Visão Geral

Este projeto utiliza um microcontrolador ESP32 e o módulo M5StickC Plus para monitorar e enviar dados para um servidor Zabbix. O sistema mede a intensidade do som e o estado de uma porta, e envia essas informações para o servidor Zabbix. O código também exibe as informações no display do M5StickC Plus e acende um LED com base na intensidade do som.

## Funcionalidades

- **Conexão Wi-Fi**: Conecta-se a uma rede Wi-Fi especificada.
- **Integração com Zabbix**: Envia dados para um servidor Zabbix para monitoramento.
- **Medição e Exibição de Dados**: Mede a intensidade do som e o estado da porta, e exibe essas informações no display LCD do M5StickC Plus.
- **Monitoramento de Hardware**: Monitora o sinal de áudio e o estado da porta, e acende um LED baseado na intensidade do som.
- **Comunicação Serial**: Fornece feedback via monitor serial sobre o valor RMS do som e o estado da porta.

## Bibliotecas Utilizadas

- `ESP32ZabbixSender`: Para envio de dados para um servidor Zabbix.
- `M5StickCPlusUtils`: Para inicialização e gerenciamento do dispositivo M5StickC Plus, além de funções auxiliares para processamento de áudio.

## Arquivos do Projeto

### `ESP32ZabbixSender`

Este arquivo configura a conexão com o servidor Zabbix e define funções para enviar dados relacionados ao som e ao estado da porta.

### `M5StickCPlusUtils.h`

Define as funções e constantes usadas para configurar e gerenciar o M5StickC Plus, incluindo leitura de dados de áudio e exibição no LCD.

### `M5StickCPlusUtils.cpp`

Implementa as funções declaradas em `M5StickCPlusUtils.h`, incluindo:
- **`showSignal()`**: Exibe o sinal de áudio no display LCD.
- **`calculateRMS()`**: Calcula o valor RMS do áudio a partir do buffer de ADC.
- **`showData()`**: Exibe a intensidade do som e o estado da porta no display LCD.
- **`monitor_task()`**: Tarefa de monitoramento que lê os dados de áudio, calcula o RMS, e envia as informações para o Zabbix.
- **`i2sInit()`**: Inicializa o módulo I2S para captura de áudio.
- **`setupM5StickCPlus()`**: Configura o M5StickC Plus e inicia a tarefa de monitoramento.

## Configuração

### Credenciais Wi-Fi

Atualize as seguintes linhas com suas credenciais de rede Wi-Fi:

```cpp
String ssid = "SEU_SSID";
String pass = "SUA_SENHA";
```

### Configuração do Servidor Zabbix

Atualize o endereço IP, a porta e o nome do host do servidor Zabbix:

```cpp
#define SERVERADDR 10,7,221,242 // Endereço IP do servidor Zabbix
#define ZABBIXPORT 10051        // Porta do servidor Zabbix
#define ZABBIXAGHOST "node"     // Nome do host do item Zabbix
```

## Descrição do Código

- **`setup()`**: Inicializa a comunicação serial, configura o M5StickC Plus, conecta-se à rede Wi-Fi e inicializa o sender do Zabbix.
- **`loop()`**: Contém um atraso para evitar que o loop execute muito frequentemente. O status da conexão é verificado a cada segundo.
- **`zabbixTask(float rms, int8_t doorState)`**: Envia dados para o servidor Zabbix. Envia os níveis de som (`rms`) e o estado da porta (`doorState`) como itens separados e registra o resultado no monitor serial.
- **`checkConnection()`**: Verifica o status da conexão Wi-Fi e imprime mensagens de status no monitor serial.
- **Funções de `M5StickCPlusUtils.h` e `M5StickCPlusUtils.cpp`**:
  - **`showSignal()`**: Atualiza a visualização do sinal de áudio no LCD.
  - **`calculateRMS()`**: Calcula o valor RMS a partir do buffer de áudio.
  - **`showData()`**: Exibe a intensidade do som e o estado da porta no LCD.
  - **`monitor_task(void *arg)`**: Tarefa que coleta dados de áudio, calcula o RMS, e envia as informações para o Zabbix.
  - **`i2sInit()`**: Configura o módulo I2S para leitura de áudio.
  - **`setupM5StickCPlus(void *zab_s)`**: Inicializa o M5StickC Plus e cria a tarefa de monitoramento.

## Uso

1. **Faça o upload** do código para a sua placa ESP32.
2. **Abra** o Monitor Serial para visualizar o status da conexão, o valor RMS do som e o estado da porta.
3. **Monitore** o servidor Zabbix para verificar os dados recebidos.
4. **Observe** o display LCD do M5StickC Plus para ver a intensidade do som e o estado da porta. O LED acenderá ou apagará com base na intensidade do som.

## Solução de Problemas

- Verifique se as credenciais Wi-Fi e a configuração do servidor Zabbix estão corretas.
- Certifique-se de que o servidor Zabbix é acessível a partir do ESP32.
- Verifique se o servidor Zabbix está configurado corretamente para aceitar dados do ESP32.
- Confirme que o hardware (M5StickC Plus e sensores) está corretamente conectado e funcionando.

## Licença

Este projeto é licenciado sob a Licença MIT. Consulte o arquivo [LICENSE](LICENSE) para mais detalhes.

## Contato

Para mais perguntas ou suporte, entre em contato com [girossine123@gmail.com].

---
