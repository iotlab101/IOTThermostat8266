#include <Arduino.h>
#include <IO7F8266.h>
#include <Wire.h>
#include <DHTesp.h>
#include <SSD1306.h>

SSD1306             display(0x3c, 4, 5, GEOMETRY_128_32);
                    //      |     |  5 -> i2c scl
                    //      |     4 -> i2c sda
                    //      0x32 -> i2c address

String user_html = "";

char* ssid_pfix = (char*)"IOTThermostat";
unsigned long lastPublishMillis = -pubInterval;
const int           pulseA = 12;
const int           pulseB = 13;
volatile int        lastEncoded = 0;
volatile long       encoderValue = 70;

// DHT22 is attached to GPIO14 on this Kit
#define             DHTPIN  14
DHTesp              dht;
float               humidity;
float               temp_f;
unsigned long       lastDHTReadMillis = 0;
#define             interval  2000   
float               tgtT;

IRAM_ATTR void handleRotary() {
    // Never put any long instruction
    int MSB = digitalRead(pulseA); //MSB = most significant bit
    int LSB = digitalRead(pulseB); //LSB = least significant bit

    int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
    int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
    lastEncoded = encoded; //store this value for next time
    if (encoderValue > 255) {
        encoderValue = 255;
    } else if (encoderValue < 0 ) {
        encoderValue = 0;
    }
    lastPublishMillis = millis() - pubInterval + 200;              // send and publish immediately
}

void gettemperature() {
    unsigned long currentMillis = millis();

    if(currentMillis - lastDHTReadMillis >= interval) {
        lastDHTReadMillis = currentMillis;

        humidity = dht.getHumidity();               // Read humidity (percent)
        temp_f = dht.getTemperature();              // Read temperature as Fahrenheit
    }
}

void publishData() {
    StaticJsonDocument<512> root;
    JsonObject data = root.createNestedObject("d");
    char dht_buffer[10];
    char dht_buffer2[10];

    gettemperature();
    display.setColor(BLACK);
    display.fillRect(80, 11, 100, 33);
    display.setColor(WHITE);
    sprintf(dht_buffer, "%2.1f", temp_f);
    display.drawString(80, 11, dht_buffer);
    data["temperature"] = dht_buffer;

    tgtT = map(encoderValue, 0, 255, 10, 50);
    sprintf(dht_buffer2, "%2.1f", tgtT);
    data["target"] = dht_buffer2;
    display.drawString(80, 22, dht_buffer2);
    display.display();

    serializeJson(root, msgBuffer);
    client.publish(evtTopic, msgBuffer);
}

void handleUserCommand(char* topic, JsonDocument* root) {
    JsonObject d = (*root)["d"];

    if(d.containsKey("target")) {
        tgtT = d["target"];
        encoderValue = map(tgtT, 10, 50, 0, 255);
        lastPublishMillis = - pubInterval;
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(pulseA, INPUT_PULLUP);
    pinMode(pulseB, INPUT_PULLUP);
    attachInterrupt(pulseA, handleRotary, CHANGE);
    attachInterrupt(pulseB, handleRotary, CHANGE);
    dht.setup(DHTPIN, DHTesp::DHT22);
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.drawString(35, 0, "IOT Thermostat");
    display.drawString(20, 11, "Current : ");
    display.drawString(20, 22, "Target : ");
    display.display();

    initDevice();
    JsonObject meta = cfg["meta"];
    pubInterval = meta.containsKey("pubInterval") ? meta["pubInterval"] : 0;
    lastPublishMillis = -pubInterval;

    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.printf("\nIP address : ");
    Serial.println(WiFi.localIP());

    userCommand = handleUserCommand;
    set_iot_server();
    iot_connect();
}

void loop() {
    if (!client.connected()) {
        iot_connect();
    }

    client.loop();
    if ((pubInterval != 0) && (millis() - lastPublishMillis > pubInterval)) {
        publishData();
        lastPublishMillis = millis();
    }
}