// Sistema de Monitoramento Completo para Vinheria
// Temperatura, Umidade e Luminosidade com Data Logger

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <RTClib.h>
#include <EEPROM.h>

// Opções de configuração
#define LOG_OPTION 1      // Leitura do log ativada
#define SERIAL_OPTION 1   // Comunicação serial ligada
#define UTC_OFFSET -3     // Define fuso horário UTC-3 (Caso deseje utilizar outro fuso, alterar o fuso no código)

// Configuração do DHT22 (DHT11 no arduino físico em sala)
#define DHTPIN 2
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);

// Configuração do LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Configuração do RTC
RTC_DS1307 rtc;

// Definindo as variáveis
const int pinoLDR = A0;
const int ledVerde = 8;
const int ledAmarelo = 7;
const int ledVermelho = 6;
const int buzzer = 13;

// Configurações da EEPROM para data logger
const int maxRecords = 100;
const int recordSize = 10;  // 4 (timestamp) + 2 (temp) + 2 (umid) + 2 (lumi)
int startAddress = 0;
int endAddress = maxRecords * recordSize;
int currentAddress = 0;
unsigned long ultimoRegistroEEPROM = 0;  // Controla intervalo de 30s

// Triggers - Limites ideais para o armazenamento do vinho
float trigger_t_min = 12.0;  // Temperatura mínima ideal (°C)
float trigger_t_max = 18.0;  // Temperatura máxima ideal (°C)
float trigger_u_min = 60.0;  // Umidade mínima ideal (%)
float trigger_u_max = 80.0;  // Umidade máxima ideal (%)
int trigger_l_max = 40;      // Luminosidade máxima ideal (%)

// Variáveis de controle do buzzer
unsigned long tempoInicioBuzzer = 0;
bool buzzerAtivo = false;

// Sistema de média móvel para LDR (média de 10 leituras)
const int numLeituras = 10;
int leituras[numLeituras];
int indiceLeitura = 0;
int total = 0;
int media = 0;

// Variáveis de leitura
float temperatura = 0;
float umidade = 0;
int luminosidade = 0;

// Caracteres customizados para o LCD
// Caracteres de inicialização
byte uva[8] = {
  B01110,
  B11111,
  B11111,
  B01110,
  B11111,
  B11111,
  B11111,
  B01110
};

byte folha[8] = {
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
  B00100,
  B00000
};

// Ícones de temperatura (3 níveis)
byte termoBaixo[8] = {  // Temperatura baixa - termômetro vazio
  B00100,
  B01010,
  B01010,
  B01010,
  B01010,
  B10001,
  B10001,
  B01110
};

byte termoMedio[8] = {  // Temperatura média - termômetro meio cheio
  B00100,
  B01010,
  B01010,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110
};

byte termoAlto[8] = {   // Temperatura alta - termômetro cheio
  B00100,
  B01110,
  B01110,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110
};

// Ícones de umidade (3 níveis)
byte gotaBaixa[8] = {   // Umidade baixa - gota vazia
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B10001,
  B10001,
  B01110
};

byte gotaMedia[8] = {   // Umidade média - gota meio cheia
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B11111,
  B11111,
  B01110
};

byte gotaCheia[8] = {   // Umidade alta - gota cheia
  B00100,
  B00100,
  B01110,
  B01110,
  B11111,
  B11111,
  B11111,
  B01110
};

// Ícones de luminosidade (3 níveis)
byte lampadaApagada[8] = {  // Luminosidade baixa - lâmpada apagada
  B01110,
  B10001,
  B10001,
  B10001,
  B01110,
  B01110,
  B00100,
  B00100
};

byte lampadaMeia[8] = {     // Luminosidade média - lâmpada meia
  B01110,
  B10001,
  B10101,
  B10101,
  B01110,
  B01110,
  B00100,
  B00100
};

byte lampadaAcesa[8] = {    // Luminosidade alta - lâmpada acesa
  B01110,
  B11111,
  B11111,
  B11111,
  B01110,
  B01110,
  B00100,
  B00100
};

// Emojis para os estados
byte emojiFeliz[8] = {  // Estado OK
  B00000,
  B01010,
  B01010,
  B00000,
  B10001,
  B10001,
  B01110,
  B00000
};

byte emojiSerio[8] = {  // Estado Alerta
  B00000,
  B01010,
  B01010,
  B00000,
  B00000,
  B11111,
  B00000,
  B00000
};

byte emojiTriste[8] = {  // Estado Crítico
  B00000,
  B01010,
  B01010,
  B00000,
  B01110,
  B10001,
  B10001,
  B00000
};

void setup() {
  // Inicializa pinos e define o seu tipo
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(pinoLDR, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Inicializa Serial
  Serial.begin(9600);
  
  // Inicializa DHT22
  dht.begin();
  
  // Inicializa RTC
  rtc.begin();
  
  // Ajusta RTC apenas na primeira vez (comentar depois)
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  // Inicializa LCD I2C
  lcd.init();
  lcd.backlight();
  
  // Cria caracteres customizados com logo da empresa
  lcd.createChar(0, uva);
  lcd.createChar(1, folha);
  
  // Tela de boas-vindas com animação gráfica
  telaBoasVindas();
  
  // Inicializa array de leituras do LDR
  for (int i = 0; i < numLeituras; i++) {
    leituras[i] = 0;
  }
  
  // Lê logs da EEPROM se LOG_OPTION estiver ativado
  if (LOG_OPTION) {
    get_log();
  }
  
  Serial.println("Sistema iniciado!");
  Serial.println("Timestamp\t\tTemp\tUmid\tLuz\tStatus");
}

void loop() {
  // Obtém timestamp
  DateTime now = rtc.now();
  int offsetSeconds = UTC_OFFSET * 3600;
  DateTime adjustedTime = DateTime(now.unixtime() + offsetSeconds);
  
  // Lê todos os sensores
  lerSensores();
  
  // Exibe dados no LCD
  exibirNoLCD();
  
  // Exibe dados no Serial com timestamp
  if (SERIAL_OPTION) {
    exibirNoSerial(adjustedTime);
  }
  
  // Verifica condições e aciona alertas
  verificarCondicoes();
  
  // Controla buzzer
  controlarBuzzer();
  
  // Data logger: registra na EEPROM uma vez a cada 30 segundos se estiver em ALERTA ou CRÍTICO
  if (millis() - ultimoRegistroEEPROM >= 30000) {  // 30 segundos = 30000 milissegundos
    if (isAmbienteAlerta() || isAmbienteCritico()) {
      salvarNaEEPROM(adjustedTime);
      Serial.println(" --> REGISTRADO NA EEPROM");
      ultimoRegistroEEPROM = millis();  // Atualiza o tempo do último registro
    }
  }
  
  delay(2000); // Atualiza a cada 2 segundos
}

void telaBoasVindas() {
  // Tela 1: Logo da empresa com animação
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(1)); // Folha
  lcd.print(" Vinharia  ");
  lcd.write(byte(1)); // Folha
  
  lcd.setCursor(0, 1);
  lcd.write(byte(0)); // Uva
  lcd.print("  Agnello  ");
  lcd.write(byte(0)); // Uva
  
  delay(3000);
  
  // Animação de carregamento com caracteres gráficos
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inicializando...");
  
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.write(byte(0xFF)); // Bloco cheio
    delay(100);
  }
  
  delay(500);
  lcd.clear();
}

void lerSensores() {
  // Lê temperatura e umidade do DHT22
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();
  
  // Verifica se a leitura falhou
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler DHT22!");
    temperatura = 0;
    umidade = 0;
  }
  
  // Lê LDR com média móvel (média de 10 leituras)
  total = total - leituras[indiceLeitura];
  int valorLDR = analogRead(pinoLDR);
  leituras[indiceLeitura] = valorLDR;
  total = total + leituras[indiceLeitura];
  indiceLeitura = (indiceLeitura + 1) % numLeituras;
  
  media = total / numLeituras;
  
  // Mapeamento do LDR usando map() - média de 10s
  luminosidade = map(media, 0, 1023, 0, 100);
}

void exibirNoLCD() {
  // Cria ícones dinâmicos baseados nos valores
  
  // Ícone de temperatura (slot 2)
  if (temperatura < trigger_t_min) {
    lcd.createChar(2, termoBaixo);  // Frio - vazio
  } else if (temperatura >= trigger_t_min && temperatura <= trigger_t_max) {
    lcd.createChar(2, termoMedio);  // Ideal - meio cheio
  } else {
    lcd.createChar(2, termoAlto);   // Quente - cheio
  }
  
  // Ícone de umidade (slot 3)
  if (umidade < trigger_u_min) {
    lcd.createChar(3, gotaBaixa);   // Seco - vazia
  } else if (umidade >= trigger_u_min && umidade <= trigger_u_max) {
    lcd.createChar(3, gotaMedia);   // Ideal - meio cheia
  } else {
    lcd.createChar(3, gotaCheia);   // Úmido - cheia
  }
  
  // Ícone de luminosidade (slot 4)
  if (luminosidade <= trigger_l_max) {
    lcd.createChar(4, lampadaApagada);  // Escuro - apagada
  } else if (luminosidade > trigger_l_max && luminosidade <= 65) {
    lcd.createChar(4, lampadaMeia);     // Médio - meia
  } else {
    lcd.createChar(4, lampadaAcesa);    // Claro - acesa
  }
  
  // Emoji do estado (slot 5)
  if (isAmbienteOK()) {
    lcd.createChar(5, emojiFeliz);
  } else if (isAmbienteAlerta()) {
    lcd.createChar(5, emojiSerio);
  } else {
    lcd.createChar(5, emojiTriste);
  }
  
  lcd.clear();
  
  // Primeira linha do display: Temperatura, Umidade e Luminosidade
  lcd.setCursor(0, 0);
  
  // Temperatura
  lcd.write(byte(2)); // Termômetro
  lcd.print(temperatura, 1); // 1 casa decimal
  lcd.print("C ");
  
  // Umidade
  lcd.write(byte(3)); // Gota
  lcd.print(umidade, 0); // Sem decimal
  lcd.print("% ");
  
  // Luminosidade
  lcd.write(byte(4)); // Lâmpada
  lcd.print(luminosidade);
  lcd.print("%");
  
  // Segunda linha do display: Mensagem do estado com emoji
  lcd.setCursor(0, 1);
  
  if (isAmbienteOK()) {
    lcd.write(byte(5)); // Emoji feliz
    lcd.print(" Tudo Certo!");
  } else if (isAmbienteAlerta()) {
    lcd.write(byte(5)); // Emoji sério
    lcd.print(" Atencao!");
  } else {
    lcd.write(byte(5)); // Emoji triste
    lcd.print(" Critico!");
  }
}

void exibirNoSerial(DateTime ajustado) {
  // Timestamp formatado
  Serial.print(ajustado.year());
  Serial.print("/");
  if (ajustado.month() < 10) Serial.print("0");
  Serial.print(ajustado.month());
  Serial.print("/");
  if (ajustado.day() < 10) Serial.print("0");
  Serial.print(ajustado.day());
  Serial.print(" ");
  
  if (ajustado.hour() < 10) Serial.print("0");
  Serial.print(ajustado.hour());
  Serial.print(":");
  if (ajustado.minute() < 10) Serial.print("0");
  Serial.print(ajustado.minute());
  Serial.print(":");
  if (ajustado.second() < 10) Serial.print("0");
  Serial.print(ajustado.second());
  Serial.print("\t");
  
  // Temperatura com espaçamento fixo
  if (temperatura < 10) Serial.print(" ");  // Adiciona espaço se menor que 10
  Serial.print(temperatura, 1);
  Serial.print("C\t");
  
  // Umidade com espaçamento fixo
  if (umidade < 10) Serial.print(" ");  // Adiciona espaço se menor que 10
  Serial.print(umidade, 1);
  Serial.print("%\t");
  
  // Luminosidade com espaçamento fixo
  if (luminosidade < 10) Serial.print(" ");  // Adiciona espaço se menor que 10
  Serial.print(luminosidade);
  Serial.print("%\t");
  
  // Status
  if (isAmbienteOK()) {
    Serial.println("OK");
  } else if (isAmbienteAlerta()) {
    Serial.println("ALERTA");
  } else {
    Serial.println("CRITICO");
  }
}

bool isAmbienteOK() {
  return (temperatura >= trigger_t_min && temperatura <= trigger_t_max &&
          umidade >= trigger_u_min && umidade <= trigger_u_max &&
          luminosidade <= trigger_l_max);
}

bool isAmbienteAlerta() {
  // Alerta se está próximo dos limites críticos
  bool tempAlerta = (temperatura < trigger_t_min || temperatura > trigger_t_max) &&
                    (temperatura >= trigger_t_min - 3 && temperatura <= trigger_t_max + 3);
  bool umidAlerta = (umidade < trigger_u_min || umidade > trigger_u_max) &&
                    (umidade >= trigger_u_min - 10 && umidade <= trigger_u_max + 10);
  bool lumAlerta = (luminosidade > trigger_l_max && luminosidade <= 65);
  
  return (tempAlerta || umidAlerta || lumAlerta) && !isAmbienteCritico();
}

bool isAmbienteCritico() {
  // Crítico se está muito fora dos limites
  bool tempCritica = (temperatura < trigger_t_min - 3 || temperatura > trigger_t_max + 3);
  bool umidCritica = (umidade < trigger_u_min - 10 || umidade > trigger_u_max + 10);
  bool lumCritica = (luminosidade > 65);
  
  return (tempCritica || umidCritica || lumCritica);
}

void verificarCondicoes() {
  apagarTodosLEDs();
  
  if (isAmbienteOK()) {
    // Ambiente OK - LED verde
    digitalWrite(ledVerde, HIGH);
    
  } else if (isAmbienteAlerta()) {
    // Alerta - LED amarelo + buzzer
    digitalWrite(ledAmarelo, HIGH);
    ativarBuzzer();
    
  } else if (isAmbienteCritico()) {
    // Crítico - LED vermelho + buzzer
    digitalWrite(ledVermelho, HIGH);
    ativarBuzzer();
  }
}

void ativarBuzzer() {
  if (!buzzerAtivo) {
    buzzerAtivo = true;
    tempoInicioBuzzer = millis();
    digitalWrite(buzzer, HIGH);
  }
}

void controlarBuzzer() {
  // Desliga buzzer após 3 segundos
  if (buzzerAtivo && (millis() - tempoInicioBuzzer >= 3000)) {
    digitalWrite(buzzer, LOW);
    buzzerAtivo = false;
  }
}

void apagarTodosLEDs() {
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledVermelho, LOW);
}

void salvarNaEEPROM(DateTime timestamp) {
  // Converte valores float para int para economia de espaço
  int tempInt = (int)(temperatura * 100);
  int umidInt = (int)(umidade * 100);
  int lumInt = luminosidade;
  
  // Escreve na EEPROM
  EEPROM.put(currentAddress, timestamp.unixtime());
  EEPROM.put(currentAddress + 4, tempInt);
  EEPROM.put(currentAddress + 6, umidInt);
  EEPROM.put(currentAddress + 8, lumInt);
  
  // Atualiza endereço para próximo registro
  getNextAddress();
}

void getNextAddress() {
  currentAddress += recordSize;
  if (currentAddress >= endAddress) {
    currentAddress = 0; // Volta ao início (buffer circular)
  }
}

void get_log() {
  Serial.println("\n=== LOGS ARMAZENADOS NA EEPROM ===");
  Serial.println("Timestamp\t\t\tTemp\tUmid\tLuz");
  Serial.println("---------------------------------------------------");
  
  for (int address = startAddress; address < endAddress; address += recordSize) {
    unsigned long timeStamp;
    int tempInt, umidInt, lumInt;
    
    // Lê dados da EEPROM
    EEPROM.get(address, timeStamp);
    EEPROM.get(address + 4, tempInt);
    EEPROM.get(address + 6, umidInt);
    EEPROM.get(address + 8, lumInt);
    
    // Verifica se é um registro válido
    if (timeStamp != 0xFFFFFFFF && timeStamp != 0) {
      // Converte de volta para float
      float temperature = tempInt / 100.0;
      float humidity = umidInt / 100.0;
      
      // Exibe timestamp formatado
      DateTime dt = DateTime(timeStamp);
      Serial.print(dt.timestamp(DateTime::TIMESTAMP_FULL));
      Serial.print("\t");
      Serial.print(temperature, 1);
      Serial.print("C\t");
      Serial.print(humidity, 1);
      Serial.print("%\t");
      Serial.print(lumInt);
      Serial.println("%");
    }
  }
  
  Serial.println("===================================\n");
}
