String getStatus() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& jsonRoot = jsonBuffer.createObject();
  String Status;
  jsonRoot["wtwState"] = currentState;
  jsonRoot["changeFilter"] = changeFilter;
  jsonRoot["uptime"]=millis()/1000;
  jsonRoot.prettyPrintTo(Status);   //printTo
  return Status;
}

String getNetworks(){
  DynamicJsonBuffer jsonDynamicBuffer;
  String Networks;
  JsonObject& jsonNetworks = jsonDynamicBuffer.createObject();
  JsonArray& jsonSSIDarray = jsonNetworks.createNestedArray("Networks");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i)
  {
     JsonObject& jsonSSID = jsonDynamicBuffer.createObject();
     jsonSSID["ID"]   = i;
     jsonSSID["SSID"] = WiFi.SSID(i);
     jsonSSID["RSSI"] = WiFi.RSSI(i);
     jsonSSID["encryptionType"] = WiFi.encryptionType(i);
     jsonSSIDarray.add(jsonSSID);
   }
  jsonNetworks.prettyPrintTo(Networks); 
  return Networks;
}

void WebserverNormalMode() {
  // -- Normal mode --
    server.on("/", []() {
      server.send(200,"text/html",indexNormalHTML);
    });

    // Address: /newState
    server.on("/newState", [](){
       int wtwState = server.arg("wtwtState").toInt();
       updateWTWstate(wtwState);
       server.send(200,"text/html","");
    });

    // Address: /status
    server.on("/status", []() {
      server.send(200, "application/json", getStatus());
    });

    // Address: /changeState
    server.on("/changeState", []() {
      int wtwState = server.arg("state").toInt();
      updateWTWstate(wtwState);
      server.send(200, "application/json", getStatus());
    });

    // Address: 404
    //server.onNotFound(handleNotFound);
    server.onNotFound([]() {
      String message = "File Not Found\n\n";
      message += "URI: ";
      message += server.uri();
      message += "\nMethod: ";
      message += (server.method() == HTTP_GET) ? "GET" : "POST";
      message += "\nArguments: ";
      message += server.args();
      message += "\n";
      for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      }
      server.send(404, "text/plain", message);
    });
}


void WebserverSetupMode() {
  // -- Setup Mode -- //
    server.on("/", []() {
      server.send(200,"text/html",indexSetupHTML);
    });
    
    // Address: /networks
    server.on("/networks", []() {
       server.send(200, "application/json", getNetworks());
    });
  
    // Address: /save
    server.on("/save", []()
    {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qmqtt = server.arg("mqtt");
      if (qsid.length() > 0 ) {
        //noInterrupts();
        Serial.println("Clearing memory...");
        for (int i = eepromSSIDstart; i <= eepromSSIDend; ++i) {
          EEPROM.write(i, 0);
        }
        for (int i = eepromPASSstart; i <= eepromPASSend; ++i) {
          EEPROM.write(i, 0);
        }
        for (int i = eepromMQTTstart; i <= eepromMQTTend; ++i) {
          EEPROM.write(i, 0);
        }
        //for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }

        Serial.print("Writing SSID in memory       : ");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(eepromSSIDstart + i, qsid[i]);
          Serial.print(qsid[i]);
        }
        Serial.println("");
        Serial.print("Writing passphrase in memory : ");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(eepromPASSstart + i, qpass[i]);
          Serial.print(qpass[i]);
        }
        Serial.println("");
        Serial.print("Writing MQTT broker in memory : ");
        for (int i = 0; i < qmqtt.length(); ++i)
        {
          EEPROM.write(eepromMQTTstart + i, qmqtt[i]);
          Serial.print(qmqtt[i]);
        }
        Serial.println("");        
        yield();
        EEPROM.commit();
        //interrupts();
        server.send(200, "application/json", "{\"Success\":\"Succesfully stored new wireless settings to memory. Restart to boot into normal mode\"}");

      } else {

        //Unkown page
        server.send(404, "application/json", "{\"Error\"}");
      }
      });
}
