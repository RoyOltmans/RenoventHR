/* REVISION
 v0.5 Roy Oltmans
 - Minified HTML
 - Further restructured and optimized code
 - Added manual config
 - Seperated Core, Data, Interfacing and tools (not fully)
 - Removed intmode/pinmode (secondary in total)
 - Added/rebuild button, startup, filter functions
 v0.4 Roy Oltmans
 - Restrucutered code
 - Changed MCP calls intMode to pinMode
 - Added manual WIFI configuration trough config file
 v0.3 Ingmaer Verheij
 - Added MQTT interface
 v0.2 Ingmaer Verheij
 - Filter change was read from ESP8266 instead of MCP23008
 - Swapped STATE2 with STATE3
 - Removed "Master" in naming
 - Moved interrupts to last action in setup
 - Removed legacy code
 v0.1 Ingmaer Verheij
 - Unkown
 - First release 
*/

/*-------------------Include Standard libraries----------------*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SPI.h>                                                         // Required for OLED
#include <Wire.h>                                                        // Required for OLED
#include <Adafruit_GFX.h>                                                // Required for OLED
#include <Adafruit_SSD1306.h>                                            // Required for OLED
#include <Adafruit_MCP23008.h>                                           // Port Expander

/*-------------------Loading User Configuration----------------*/
#include "config.h"

/*-------------------Var preperation----------------*/
String eepromSSID;
String eepromPassword;
String eepromMQTT;
String content;
int statusCode;
bool ButtonPinUpState;
bool ButtonPinDownState;
bool FilterState;
int FilterChangePinState = 0;
bool startupMode;                       // false = Normal mode, true = Config mode
const char* NodeName = "Renovent HR";
const char* SSIDsetup = "RenoventHR";
/*-------------------Var WTW state----------------*/
volatile int currentState;   
volatile int changeFilter = false;
volatile int buttonState = 0;
#define buttonStateMin 0
#define buttonStateMax 3
/*-------------------Var GPIO on ESP8266----------------*/
#define PIN_OLED_RESET    20            // Something which isn't connected
#define PIN_PE_int        12            // Interrupt for port extender
#define PIN_startupmode   14            // Startup mode (setup or normal)
#define PIN_reset         16            // Connected to reset pin
/*-------------------Port extender----------------*/
#define PIN_relay1         0            // On port expander 
#define PIN_relay2         1            // On port expander
#define PIN_button_up      2            // Button up
#define PIN_button_down    3            // Button down
#define PIN_FilterChange   4            // Filter change indicator
/*-------------------Var EEPROM----------------*/
#define eepromSSIDstart    0            //  0 - 31  = SSID
#define eepromSSIDend     31            //
#define eepromPASSstart   32            // 32 - 95  = Passprhase
#define eepromPASSend     95            //
#define eepromWTWaddr     96            // 96       = wtwState
#define eepromMQTTstart   97            // 97 - 352 = MQTT broker
#define eepromMQTTend    352            //
/*-------------------Var MQTT----------------*/
#define MQTTpublishInterval  60         // MQTT Interval in seconds to publish value
#define MQTTretryInterval    60         // MQTT Interval in seconds to retry MQTT connection
long lastMQTTmsg   = millis() + (MQTTpublishInterval * 1000); 
long lastMQTTretry = millis() + (MQTTretryInterval   * 1000); 

/*-------------------Loading Libs----------------*/
MDNSResponder mdns;
ESP8266WebServer server(80);
WiFiClient espClient;
Adafruit_SSD1306 display(PIN_OLED_RESET);           // OLED 0.96 128x64 i2C (at 0x3C)
PubSubClient mqttClient(espClient);                 // Loading PubSubClient
Adafruit_MCP23008 mcp;                              // Port expander MCP23008

/*-------------------Loading custom----------------*/
#include "data.h"
#include "toollib.h"
#include "corelib.h"
#include "interface.h"

void setup() {
  // Begin Serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Booting...");
  
  // Setup pins
  Serial.print(F("GPIO...")); delay(100);
  mcp.begin(); // Start port expander
  Serial.print(F("Port expander started"));
  pinMode(PIN_reset, OUTPUT);
  digitalWrite(PIN_reset, HIGH); // Pull-up resistor
  pinMode(PIN_PE_int, INPUT);
  pinMode(PIN_startupmode, INPUT); // Determine startup mode 
  mcp.pinMode(PIN_relay1, OUTPUT);
  mcp.pinMode(PIN_relay2, OUTPUT);
  mcp.pinMode(PIN_button_up, INPUT); // Button up
  mcp.pinMode(PIN_button_down, INPUT); // Button down
  mcp.pinMode(PIN_FilterChange, INPUT); // Filter change indicator
  mcp.digitalWrite (PIN_relay1, HIGH);
  delay(2000);
  mcp.digitalWrite (PIN_relay1, LOW);
  Serial.println("[DONE]");
  
  // OLED
  Serial.print(F("OLED..."));
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);                          // Initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(NodeName);
  display.print(F("   Ver:"));
  display.println(Version);
  display.println(F(""));
  display.println(F("   ...starting..."));
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  Serial.println(F("initialized")); delay(100);
  
  // Determine startup mode
  startupMode = (bool)digitalRead(PIN_startupmode);   
  readEEPROM();                                                       // Reading out EEPROM
  setupWIFI(startupMode);                                             // Start WiFi
  Serial.print(F("Starting webserver..."));            
  server.begin();                                   
  Serial.println(F("[DONE]"));  
  
  Serial.print(F("Booting in ")); 
  if (startupMode) {                                                  // Determine startup mode
    Serial.println(F("SETUP mode"));                                  // Setup mode
    Serial.print(F("Setting up webserver..."));             // Setup Webserver
    WebserverSetupMode();
    Serial.println(F("[DONE]"));          
    display.invertDisplay(true);
  } else {
    Serial.println(F("NORMAL mode"));   // Normal mode
    Serial.print(F("Setting up webserver..."));             // Setup Webserver
    WebserverNormalMode();  delay(100);            // Webserver Normal Mode
    Serial.println(F("[DONE]"));
    connectMQTT();       // MQTT connecting
    display.print(F("Configure WTW..."));  display.display();         // WTW state and Change Filter
    int wtwState = EEPROM.read(eepromWTWaddr);
    if (wtwState == 255) {
      wtwState = 0;
    }
    Serial.println(F("[DONE]"));
    doFilterChange();
    updateWTWstate(wtwState);           // Push initial state to the WTW device (stored in EEPROM)
    display.display();
    delay(2000);
    updateDisplay();
    Serial.println(F("[DONE]"));
    Serial.print(F("MCP Interrupts loading..."));    //MCP Interrupts
    attachInterrupt(PIN_PE_int, doPortExtender, FALLING);
    Serial.println(F("[DONE]"));
  }
  display.display();
  Serial.println(F("initialized")); delay(100);
}

void loop() {
  server.handleClient();               // Handle HTTP (REST) 
  if ((bool)digitalRead(PIN_startupmode) != startupMode) doStartupMode();  // Reboot device is startup mode is changed
  doStartup();
}

void doPortExtender() {
  if (mcp.digitalRead(PIN_button_up)) {buttonUp();  updateWTWstate(buttonState);};
  if (mcp.digitalRead(PIN_button_down)) {buttonDown();  updateWTWstate(buttonState);};
  doFilterChange();
}

void doStartup() {
   if (!startupMode) {   // Handle MQTT    
     if (!mqttClient.connected()) { reconnectMQTT(); } // Connect MQTT if necessary
     if (mqttClient.connected()) {      // Only receive/send if MQTT is actually connected
        mqttClient.loop();              // Receive 
        if ( millis() - lastMQTTmsg > MQTTpublishInterval * 1000) {   // Publish new values on defined interval
          lastMQTTmsg =  millis();
          Serial.println(F("Publish MQTT message"));
          mqttClient.publish(MQTT_State_Topic, String(currentState).c_str());
          mqttClient.publish(MQTT_FilterState_Topic, String(changeFilter).c_str());
        }
     }
     doButtonChange();
     doFilterChange();
  }
}

void doButtonChange() {
  ButtonPinUpState = (bool)mcp.digitalRead(PIN_button_up); 
  ButtonPinDownState = (bool)mcp.digitalRead(PIN_button_down);
  if (ButtonPinUpState && buttonState < buttonStateMax) {
     buttonUp();  
     updateWTWstate(buttonState);
     Serial.print(F("Pin Button up state: "));
     Serial.println(ButtonPinUpState);
  }
  if (ButtonPinDownState && buttonState > buttonStateMin) {
     buttonDown();  
     updateWTWstate(buttonState);
     Serial.print(F("Pin Button Down state: "));
     Serial.println(ButtonPinDownState);
  }
}

void doFilterChange() {
  FilterState = (bool)mcp.digitalRead(PIN_FilterChange);
  if (FilterState) {
    int changeFilter = mcp.digitalRead(PIN_FilterChange);  // Update value
    Serial.print  ("changefilter : ");  
    Serial.println(changeFilter);
    updateDisplay();          //Update display
  }
}

void buttonUp() {
  if (buttonState < buttonStateMax) buttonState = buttonState + 1;       // CW - Clock Wise
}

void buttonDown() {
  if (buttonState > buttonStateMin) buttonState = buttonState - 1;       // CCW - Counter Clock Wise
}

boolean IsNumeric(String str) {
    for(char i = 0; i < str.length(); i++) {
        if ( !(isDigit(str.charAt(i)) || str.charAt(i) == '.' )) {
            return false;
        }
    }
    return true;
}
