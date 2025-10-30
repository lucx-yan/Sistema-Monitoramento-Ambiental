# Sistema de Monitoramento Completo para Vinheria

Sistema IoT desenvolvido para monitoramento em tempo real de **temperatura, umidade e luminosidade** em ambientes de armazenamento de vinhos, com data logger em EEPROM e interface visual avançada.

## 📋 Descrição do Projeto

Este projeto expande o sistema anterior de monitoramento de luminosidade, adicionando sensores de temperatura e umidade, além de um sistema completo de registro de dados (data logger) com RTC e EEPROM.

## 🎯 Funcionalidades

### Monitoramento Completo
- ✅ **Temperatura**: Monitoramento via DHT22 (12-18°C ideal)
- ✅ **Umidade**: Controle de umidade relativa (60-80% ideal)
- ✅ **Luminosidade**: Sensor LDR com média móvel de 10 leituras (0-40% ideal)

### Sistema de Alertas Inteligente
- 🟢 **Estado OK**: Todos os parâmetros dentro dos limites ideais
- 🟡 **Estado ALERTA**: Parâmetros próximos aos limites críticos (LED amarelo + buzzer)
- 🔴 **Estado CRÍTICO**: Parâmetros muito fora dos limites (LED vermelho + buzzer)

### Data Logger com EEPROM
- 📝 Registro automático a cada 30 segundos quando em alerta ou crítico
- 💾 Armazena até 100 registros com timestamp completo
- 🔄 Sistema de buffer circular (sobrescreve registros mais antigos)
- 📊 Leitura de logs na inicialização

### Interface Visual Avançada (Diferencial)
- 🎨 **Ícones dinâmicos no LCD**: 
  - 🌡️ Termômetro que enche conforme a temperatura sobe
  - 💧 Gota que enche conforme a umidade aumenta
  - 💡 Lâmpada que acende conforme a luminosidade aumenta
- 😊 **Emojis de estado**: Feliz (OK), Sério (Alerta), Triste (Crítico)
- ✨ Animação de inicialização personalizada com logo da empresa

### Comunicação e Timestamp
- 🕐 RTC DS1307 com ajuste de fuso horário (UTC-3)
- 📡 Monitor Serial com logs formatados e alinhados
- 📅 Timestamps precisos em todos os registros

## 🔧 Componentes Utilizados

### Hardware
| Componente | Quantidade | Função |
|------------|------------|--------|
| Arduino Uno R3 | 1 | Microcontrolador principal |
| DHT22 | 1 | Sensor de temperatura e umidade |
| LDR (Fotoresistor) | 1 | Sensor de luminosidade |
| RTC DS1307 | 1 | Relógio em tempo real |
| LCD 16x2 I2C | 1 | Display de informações |
| LED Verde | 1 | Indicador estado OK |
| LED Amarelo | 1 | Indicador estado alerta |
| LED Vermelho | 1 | Indicador estado crítico |
| Buzzer | 1 | Alarme sonoro |
| Resistor 220Ω | 3 | Limitador de corrente dos LEDs |
| Protoboard | 1 | Montagem do circuito |
| Jumpers | Vários | Conexões |

### Software
- Arduino IDE
- Bibliotecas:
  - `Wire.h` - Comunicação I2C
  - `LiquidCrystal_I2C.h` - Controle do LCD I2C
  - `DHT.h` - Leitura do sensor DHT22
  - `RTClib.h` - Controle do RTC
  - `EEPROM.h` - Armazenamento de dados

## 📸 Visualização do Projeto
## Circuito no Wokwi

<img src="imgs/circuitoCompleto.png" alt="Circuito" width="400">
<br>
Vista geral do circuito montado no Wokwi

## Sistema em Funcionamento

<img src="imgs/estadoOk.png" alt="Circuito" width="400">
<br>
Sistema em estado OK - Todos os parâmetros dentro dos limites ideais
<hr>
<img src="imgs/estadoAlerta.png" alt="Circuito" width="400">
<br>
Sistema em estado de alerta - Parâmetros próximos aos limites críticos (LED amarelo + buzzer)
<hr>
<img src="imgs/estadoCritico.png" alt="Circuito" width="400">
<br>
Sistema em estado crítico - Parâmetros muito fora dos limites (LED vermelho + buzzer)

## Serial Monitor

<img src="imgs/monitorIniciado.png" alt="Serial Monitor" width="400">
<br>
Serial monitor iniciado

<img src="imgs/serialMonitor.png" alt="Serial Monitor" width="400">
<br>
Serial monitor salvando na EEPROM

## 📐 Diagrama de Conexões

### Pinagem do Arduino

**Sensores:**
```
DHT22:
- VCC → 5V
- GND → GND
- DATA → Pino 2

LDR:
- Um lado → 5V
- Outro lado → A0

RTC DS1307:
- VCC → 5V
- GND → GND
- SDA → A4
- SCL → A5
```

**Display LCD I2C:**
```
- VCC → 5V
- GND → GND
- SDA → A4 (compartilhado com RTC)
- SCL → A5 (compartilhado com RTC)
```

**LEDs:**
```
- LED Verde → Pino 8 (+ resistor 220Ω para GND)
- LED Amarelo → Pino 7 (+ resistor 220Ω para GND)
- LED Vermelho → Pino 6 (+ resistor 220Ω para GND)
```

**Buzzer:**
```
- Positivo → Pino 13
- Negativo → GND
```

## 🚀 Como Executar

### No Simulador Wokwi
1. Acesse: [Link da Simulação no Wokwi](https://wokwi.com/projects/446065191320190977)
2. Clique em **"Start Simulation"**
3. Ajuste os valores dos sensores:
   - DHT22: Temperatura e umidade
   - LDR: Luminosidade
4. Observe o comportamento dos LEDs, buzzer e display
5. Abra o **Serial Monitor** para ver os logs detalhados

### No Arduino Físico
1. Baixe os arquivos do repositório
2. Instale as bibliotecas necessárias (via Library Manager)
3. Abra o arquivo `sketch.ino` no Arduino IDE
4. Monte o circuito conforme o diagrama
5. **IMPORTANTE**: Na primeira vez, deixe esta linha ativa:
   ```cpp
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   ```
6. Faça o upload do código
7. **Após o primeiro upload**, comente a linha acima e faça upload novamente
8. Abra o Monitor Serial (9600 baud) para ver os logs

## 💡 Como Funciona

### Sistema de Média Móvel
O sensor LDR utiliza um algoritmo de **média móvel** com 10 leituras para eliminar ruídos e variações momentâneas:
```cpp
// A cada leitura:
1. Remove a leitura mais antiga do array
2. Adiciona a nova leitura
3. Calcula a média das 10 leituras
4. Converte para porcentagem (0-100%) usando map()
```

### Limites e Triggers

| Parâmetro | Ideal | Alerta | Crítico |
|-----------|-------|--------|---------|
| **Temperatura** | 12-18°C | 9-12°C ou 18-21°C | <9°C ou >21°C |
| **Umidade** | 60-80% | 50-60% ou 80-90% | <50% ou >90% |
| **Luminosidade** | 0-40% | 41-65% | >65% |

### Sistema de Data Logger
```
1. Verifica se passou 30 segundos desde último registro
2. Se estiver em ALERTA ou CRÍTICO:
   → Salva na EEPROM: timestamp + temperatura + umidade + luminosidade
   → Avança para próxima posição (buffer circular)
3. Na inicialização, lê e exibe todos os logs armazenados
```

### Ícones Dinâmicos (Diferencial)
O grande diferencial deste projeto são os **ícones que mudam dinamicamente** conforme os valores:

**Termômetro 🌡️:**
- Vazio (abaixo de 12°C)
- Meio cheio (12-18°C) 
- Cheio (acima de 18°C)

**Gota 💧:**
- Vazia (abaixo de 60%)
- Meio cheia (60-80%)
- Cheia (acima de 80%)

**Lâmpada 💡:**
- Apagada (0-40%)
- Meia luz (41-65%)
- Acesa (acima de 65%)

## 📊 Saída do Monitor Serial

```
=== LOGS ARMAZENADOS NA EEPROM ===
Timestamp                    Temp    Umid    Luz
---------------------------------------------------
2024-11-01T16:31:10         20.5C   65.2%   45%
2024-11-01T16:32:05         22.3C   68.0%   78%
===================================

Sistema iniciado!
Timestamp           Temp    Umid    Luz    Status
2024/11/01 16:31:29  15.0C   61.0%   8%    OK
2024/11/01 16:31:31  15.0C   61.0%  45%    ALERTA
 --> REGISTRADO NA EEPROM
2024/11/01 16:31:33  22.0C   61.0%  78%    CRITICO
 --> REGISTRADO NA EEPROM
```

## 🎨 Interface LCD

**Tela de Boas-vindas:**
```
🍃 Vinharia  🍃
🍇  Agnello  🍇
```

**Tela de Monitoramento:**
```
🌡️15.5C 💧70% 💡35%
😊 Tudo Certo!
```

**Estados Possíveis:**
- `😊 Tudo Certo!` - Ambiente OK
- `😐 Atencao!` - Alerta
- `☹️ Critico!` - Situação crítica

## 🔄 Diferenças entre Fase 1 e Fase 2

| Aspecto | Fase 1 | Fase 2 |
|---------|--------|--------|
| Sensores | LDR apenas | LDR + DHT22 (temp/umid) |
| Display | LCD 16x2 padrão | LCD 16x2 I2C |
| Ícones | Estáticos | Dinâmicos (mudam conforme valores) |
| Data Logger | Não | Sim (EEPROM + RTC) |
| Timestamp | Não | Sim (UTC-3) |
| Faixas de alerta | 3 níveis simples | 3 níveis com margem |
| Armazenamento | Não | Buffer circular de 100 registros |
| Comunicação I2C | Não | Sim (LCD e RTC) |

## 📚 Recursos Adicionais

- [Documentação DHT22](https://www.arduino.cc/reference/en/libraries/dht-sensor-library/)
- [Biblioteca RTClib](https://github.com/adafruit/RTClib)
- [LCD I2C Tutorial](https://www.arduino.cc/reference/en/libraries/liquidcrystal-i2c/)
- [EEPROM Arduino](https://www.arduino.cc/en/Reference/EEPROM)

## 👥 Autores

- **João Victor (RM: 566640)**
- **Gustavo Macedo (RM: 567594)**
- **Gustavo Hiruo (RM: 567625)**
- **Yan Lucas (RM: 567046)**

Projeto FIAP - Vinheria Agnello

## 📄 Licença

Este projeto foi desenvolvido para fins educacionais como parte do curso da FIAP.

## 🔗 Links

- [Simulação no Wokwi](https://wokwi.com/projects/446065191320190977)
- [Vídeo explicativo]()

---

**Data de desenvolvimento**: Outubro 2024
