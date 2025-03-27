#include <FastBot.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <EEPROM.h>

// Конфигурация пинов

#define REL_PIN 19
#define DHTPIN 18
#define DHTTYPE DHT11
#define LED_PIN 4

// Конфигурация Wi-Fi и Telegram бота
#define WIFI_SSID "SSID"
#define WIFI_PASS "PASS"
#define BOT_TOKEN "BOTTOKIN"
#define AUTHORIZED_CHAT_ID "TELEGRAM CHAT ID" 
// Инициализация объектов
DHT dht(DHTPIN, DHTTYPE);
FastBot bot(BOT_TOKEN);
Adafruit_BMP085 bmp;

// Глобальные переменные
bool waitingForHumidity = false;
int humidityValue = 0;
bool autoMode = false;
bool smartMode = false;
float lastPressure = 0;

#define EEPROM_HUMIDITY_ADDR 0

// Буферы для хранения данных
const int GRAPH_POINTS = 10; // Количество точек на графике
float tempHistory[GRAPH_POINTS] = {0};
float humHistory[GRAPH_POINTS] = {0};
float pressHistory[GRAPH_POINTS] = {0};
int dataIndex = 0;

void setup() {
  // Настройка пинов
  pinMode(REL_PIN, OUTPUT);
  digitalWrite(REL_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Инициализация Serial для отладки
  Serial.begin(9600);
  Serial.println("Starting...");

  // Инициализация EEPROM
  EEPROM.begin(512);
  humidityValue = EEPROM.read(EEPROM_HUMIDITY_ADDR);
  if (humidityValue > 100) {
      humidityValue = 0;
      EEPROM.write(EEPROM_HUMIDITY_ADDR, humidityValue);
      EEPROM.commit();
  }

  // Подключение к Wi-Fi
  connectWiFi();

  // Инициализация бота и датчиков
  bot.attach(newMsg);
  dht.begin();
  Serial.println("DHT11 initialized");

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    while (1) {}
  }
  Serial.println("BMP180 initialized");

  bot.sendMessage("Устройство запущено и готово к работе.");
}

void newMsg(FB_msg& msg) {
  // Проверка авторизации пользователя
  if (msg.chatID != AUTHORIZED_CHAT_ID) {
      bot.sendMessage("Вы не авторизованы для управления этим устройством.", msg.chatID);
      return;
  }

  // Обработка команд
  if (msg.text == "/start" || msg.text == "Меню" || msg.text == "Назад в главное меню") {
      showMainMenu(msg.chatID);
  } else if (msg.text == "Ручное управление") {
      showManualMenu(msg.chatID);
  } else if (msg.text == "Включить реле") {
      digitalWrite(REL_PIN, HIGH);
      bot.sendMessage("Реле включено", msg.chatID);
  } else if (msg.text == "Выключить реле") {
      digitalWrite(REL_PIN, LOW);
      bot.sendMessage("Реле выключено", msg.chatID);
  } else if (msg.text == "Метеостанция") {
      sendSensorData(msg.chatID);
  } else if (msg.text == "Настраиваемый режим") {
      showCustomModeMenu(msg.chatID);
  } else if (msg.text == "Автоматический режим") {
      smartMode = !smartMode;
      autoMode = false; // Отключаем настраиваемый режим при включении автоматического
      bot.sendMessage(smartMode ? "Автоматический режим включен." : "Автоматический режим отключен.", msg.chatID);
  } else if (msg.text == "Установить влажность") {
      bot.sendMessage("Введите значение влажности (от 0 до 100):", msg.chatID);
      waitingForHumidity = true;
  } else if (msg.text == "Включить настраиваемый режим") {
      autoMode = true;
      smartMode = false; // Отключаем автоматический режим при включении настраиваемого
      bot.sendMessage("Настраиваемый режим включен.", msg.chatID);
  } else if (msg.text == "Отключить настраиваемый режим") {
      autoMode = false;
      bot.sendMessage("Настраиваемый режим отключен.", msg.chatID);
  } else if (msg.text == "Статус системы") {
      sendSystemStatus(msg.chatID);
  } else if (msg.text == "Графики") {
      sendSensorGraphs(msg.chatID);
  } else if (waitingForHumidity) {
      handleHumidityInput(msg);
  } else {
      bot.sendMessage("Неизвестная команда. Используйте меню.", msg.chatID);
  }
}

void showMainMenu(String chatID) {
  String menu = "Выберите режим:\n"
                "Ручное управление\n"
                "Настраиваемый режим\n"
                "Автоматический режим\n"
                "Метеостанция\n"
                "Статус системы\n"
                "Графики";
  bot.showMenu(menu, chatID);
}

void showManualMenu(String chatID) {
  String menu = "Включить реле\n"
                "Выключить реле\n"
                "Назад в главное меню";
  bot.showMenu(menu, chatID);
}

void showCustomModeMenu(String chatID) {
  String menu = "Установить влажность\n"
                "Включить настраиваемый режим\n"
                "Отключить настраиваемый режим\n"
                "Назад в главное меню";
  bot.showMenu(menu, chatID);
}

void sendSensorData(String chatID) {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float pressure = bmp.readPressure() / 133.322;

  if (isnan(humidity) || isnan(temperature)) {
    bot.sendMessage("Ошибка чтения данных с DHT11", chatID);
    return;
  }
  if (isnan(pressure)) {
    bot.sendMessage("Ошибка чтения данных с BMP180", chatID);
    return;
  }

  String pressureForecast = "Стабильное давление";
  if (lastPressure > 0) {
    if (pressure > lastPressure + 1) {
      pressureForecast = "Давление растет — ожидается улучшение погоды.";
    } else if (pressure < lastPressure - 1) {
      pressureForecast = "Давление падает — ожидается ухудшение погоды.";
    }
  }
  lastPressure = pressure;

  String message = "Метеостанция:\n"
                   "Влажность: " + String(humidity) + " %\n"
                   "Температура: " + String(temperature) + " *C\n"
                   "Атмосферное давление: " + String(pressure) + " мм рт. ст.\n"
                   "Прогноз: " + pressureForecast;
  bot.sendMessage(message, chatID);
}

void handleHumidityInput(FB_msg& msg) {
  int value = msg.text.toInt();
  if (msg.text.length() > 0 && value >= 0 && value <= 100) {
    humidityValue = value;
    autoMode = true;

    EEPROM.write(EEPROM_HUMIDITY_ADDR, humidityValue);
    EEPROM.commit();

    bot.sendMessage("Установлено значение влажности: " + String(humidityValue) + " %. Настраиваемый режим включен.", msg.chatID);
    waitingForHumidity = false;
  } else {
    bot.sendMessage("Некорректное значение. Введите число от 0 до 100.", msg.chatID);
  }
}

void sendSystemStatus(String chatID) {
  String status = "Статус системы:\n";
  status += "Реле: " + String(digitalRead(REL_PIN) ? "ВКЛ" : "ВЫКЛ") + "\n";
  status += "Настраиваемый режим: " + String(autoMode ? "ВКЛ" : "ВЫКЛ") + "\n";
  status += "Автоматический режим: " + String(smartMode ? "ВКЛ" : "ВЫКЛ") + "\n";
  status += "Установленная влажность: " + String(humidityValue) + " %\n";
  bot.sendMessage(status, chatID);
}

void sendSensorGraphs(String chatID) {
  String graph = "`\n"; // Используем markdown-форматирование

  // График температуры
  graph += "Температура:\n";
  graph += buildTextGraph(tempHistory, GRAPH_POINTS, -20, 50);
  graph += "\n";

  // График влажности
  graph += "Влажность:\n";
  graph += buildTextGraph(humHistory, GRAPH_POINTS, 0, 100);
  graph += "\n";

  // График давления
  graph += "Давление:\n";
  graph += buildTextGraph(pressHistory, GRAPH_POINTS, 700, 800);
  graph += "`"; // Закрываем markdown

  bot.sendMessage(graph, chatID); // Параметры: сообщение, chatID, уведомление, markdown
}

String buildTextGraph(float* data, int size, float minVal, float maxVal) {
  const int width = 20; // Ширина графика в символах
  String result = "";
  
  for(int i = 0; i < size; i++) {
    float value = constrain(data[i], minVal, maxVal);
    int bars = map(value * 10, minVal * 10, maxVal * 10, 0, width);
    
    // Добавляем номер точки
    if(i < 9) result += " ";
    result += String(i+1) + " |";
    
    // Рисуем бары
    for(int j = 0; j < bars; j++) result += "█";
    for(int j = bars; j < width; j++) result += " ";
    
    // Добавляем значение
    result += "| " + String(data[i], 1) + "\n";
  }
  return result;
}

void loop() {
  bot.tick();

  // Умный автоматический режим
  if (smartMode) {
    float currentHumidity = dht.readHumidity();
    float currentTemperature = dht.readTemperature();
    float currentPressure = bmp.readPressure() / 133.322;

    if (!isnan(currentHumidity) && !isnan(currentTemperature) && !isnan(currentPressure)) {
      int targetHumidity = 50;
      if (currentTemperature < 10) {
        targetHumidity = 40;
      } else if (currentTemperature > 25) {
        targetHumidity = 60;
      }

      if (currentPressure < 750) {
        targetHumidity -= 5;
      } else if (currentPressure > 770) {
        targetHumidity += 5;
      }

      if (currentHumidity < targetHumidity - 5) {
        digitalWrite(REL_PIN, HIGH);
      } else if (currentHumidity > targetHumidity + 5) {
        digitalWrite(REL_PIN, LOW);
      }
    }
  }

  // Настраиваемый режим
  if (autoMode) {
    float currentHumidity = dht.readHumidity();
    if (!isnan(currentHumidity)) {
      if (currentHumidity < humidityValue) {
        digitalWrite(REL_PIN, HIGH);
      } else {
        digitalWrite(REL_PIN, LOW);
      }
    }
  }

  // Сбор данных для графиков каждые 5 минут
  static unsigned long lastDataUpdate = 0;
  if(millis() - lastDataUpdate > 300000) { // 5 минут
    lastDataUpdate = millis();
    
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    float p = bmp.readPressure() / 133.322;

    if(!isnan(t) && !isnan(h) && !isnan(p)) {
      tempHistory[dataIndex] = t;
      humHistory[dataIndex] = h;
      pressHistory[dataIndex] = p;
      
      dataIndex = (dataIndex + 1) % GRAPH_POINTS;
    }
  }

  delay(100);
}

void connectWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 30) {
      Serial.println("Failed to connect. Restarting...");
      ESP.restart();
    }
  }

  Serial.println("\nWi-Fi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
  digitalWrite(LED_PIN, HIGH);
}
