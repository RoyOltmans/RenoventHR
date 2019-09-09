/*-------------------General----------------------*/
const char* Version = "0.5";

/*-------------------WIFI Settings----------------*/
String APssid = "SSID WIFI";
String APpassword = "PASSWORD WIFI";

/*-------------------MQTT Settings-------------------*/
String mqtt_user = "your_username"; // not compulsory only if your broker needs authentication
String mqtt_pass = "your_password"; // not compulsory only if your broker needs authentication
String mqtt_server = "MQTT SERVER NAME or IP";
long mqtt_port = strtol("1883",NULL,10);

/*-------------------MQTT topics-------------------*/
#define Gateway_Name "RenoventHR"
#define Base_Topic "home/"
#define MQTT_GetState_Topic Base_Topic Gateway_Name "/getWTWstate"
#define MQTT_State_Topic Base_Topic Gateway_Name "/wtwState"
#define MQTT_FilterState_Topic Base_Topic Gateway_Name "/changeFilter"
#define MQTT_SetState_Topic Base_Topic Gateway_Name "/setWTWstate"

#define MQTT_Standard_Topic Base_Topic Gateway_Name

/*-------------------Pin Layout-------------------*/
// GPIOA0 - ADC/A0                     OLED RESET (N/C)
// GPIO00 - RESERVED, must be HIGH     Boot mode (HIGH = normal, LOW = program)
// GPIO02 - RESERVED, must be HIGH
// GPIO04 -                            SDA
// GPIO05 -                            SCL
// GPIO12 -
// GPIO13 -
// GPIO14 -                            Startup mode
// GPIO15 - RESERVED, must be LOW
// GPIO16 - (no interrupt)             RESET pin
// GP00   -                            Relay 1
// GP01   -                            Relay 2
// GP02   -                            PIN_button_up
// GP03   -                            PIN_button_down
// GP04   -                            Filter indicator 
// GP05   -
// GP06   -
// GP07   - 
//GPIO on ESP8266
