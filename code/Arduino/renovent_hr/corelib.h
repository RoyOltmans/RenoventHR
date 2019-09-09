void doStartupMode() {
   Serial.println(F("Startup mode changed.")); // An intterupt is received
   display.clearDisplay();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0, 0);
   display.println(F("Startup mode CHANGED"));
   display.println(F(""));
   display.setTextSize(2);
   display.print("RESTARTING");  
   //Restart the device
   display.display();
   ESP.deepSleep(500000);     // .5 sec
}

void updateWTWstate(int newWTWstate) {
  
  // Remember new state
  currentState = newWTWstate;
  buttonState = currentState;
  
  // Display new state
  if (currentState != -1) {
    Serial.print(F("New WTW state: "));
    Serial.println(currentState);
    delay(100);
  }

  // Write WTW state to EEPROM
  EEPROM.write(eepromWTWaddr, currentState);
  EEPROM.commit();

  //Update display
  updateDisplay();

  // Push the WTW state to the relays
  switch (currentState) {                                                                       // Push state to relays
    case 0: mcp.digitalWrite (PIN_relay1, LOW);  mcp.digitalWrite (PIN_relay2, LOW); Serial.print("wtw state 0 ");  break;    // WTW state 0
    case 1: mcp.digitalWrite (PIN_relay1, LOW);  mcp.digitalWrite (PIN_relay2, HIGH); Serial.print("wtw state 1 "); break;    // WTW state 1
    case 2: mcp.digitalWrite (PIN_relay1, HIGH); mcp.digitalWrite (PIN_relay2, HIGH); Serial.print("wtw state 2 "); break;   // WTW state 2
    case 3: mcp.digitalWrite (PIN_relay1, HIGH); mcp.digitalWrite (PIN_relay2, LOW); Serial.print("wtw state 3 "); break;     // WTW state 3
  }

  // Make sure the new value is send via MQTT
  lastMQTTmsg   = millis() + (MQTTpublishInterval * 1000);
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
   String strTopic = topic;
          strTopic.toLowerCase();
   String strMQTTsetStateTopic =  MQTT_SetState_Topic;
          strMQTTsetStateTopic.toLowerCase();
   String strValue;
          
   Serial.print("Message arrived [");
   Serial.print(topic);
   Serial.print("] : ");
   
   for (int i = 0; i < length; i++) {
     Serial.print((char)payload[i]);
     strValue+=(char)payload[i];
   }
   Serial.println();
   
   if (strTopic == strMQTTsetStateTopic) {
      currentState = strValue.toInt();
      Serial.print("Writing State: ");
      Serial.println(currentState);
      updateWTWstate(currentState);
   } 
}

void connectMQTT(){
    //Startup MQTT Connection
    if (mqtt_server!= "") {
      randomSeed(micros());
      Serial.print(F("Setup MQTT..."));
      mqttClient.setServer(mqtt_server.c_str(), mqtt_port);
      mqttClient.setCallback(MQTTcallback);
      Serial.println(F("[DONE]"));
   }
}

void reconnectMQTT() {
   // Reconnect MQTT with defined interval
   if ( millis() - lastMQTTretry > MQTTretryInterval * 1000) {
       lastMQTTretry =  millis();
       Serial.print("Attempting MQTT connection...");
    
       // Create a random client ID
       String clientId = "RenoventHR-"; 
              clientId += String(random(0xffff), HEX);
           
       // Attempt to connect
       if (mqttClient.connect(clientId.c_str())) {
           Serial.println("connected");
           
           // (Re)subscribe
           mqttClient.subscribe(MQTT_SetState_Topic);
       } else {
          Serial.print("failed, rc=");
          Serial.println(mqttClient.state());
       }
   }
}
