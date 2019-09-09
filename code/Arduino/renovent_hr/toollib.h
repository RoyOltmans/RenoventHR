void readEEPROM() {
  // EEPROM
  EEPROM.begin(4096);
  Serial.println(F("Reading memory (EEPROM)"));
  if (APpassword == "") {
    for (int i = eepromSSIDstart; i <= eepromSSIDend; ++i) {
      //Serial.println(EEPROM.read(i));
      if (EEPROM.read(i) == 0) {
        break;
      }
      if (EEPROM.read(i) == 255) {
        break;
      }
      eepromSSID += char(EEPROM.read(i));
    }
    APssid = eepromSSID.c_str();
    Serial.print(F("   SSID: ")); Serial.println(APssid);
  }
  if (APpassword == "") {
    for (int i = eepromPASSstart; i <= eepromPASSend; ++i) {
      //Serial.println(EEPROM.read(i));
      if (EEPROM.read(i) == 0) {
        break;
      }
      if (EEPROM.read(i) == 255) {
        break;
    }
      eepromPassword += char(EEPROM.read(i));
    }
    APpassword = eepromPassword.c_str();
    // Serial.print(F("   PASS: ")); Serial.println(APpassword);  
  }
  if (mqtt_server == "") {
    for (int i = eepromMQTTstart; i <= eepromMQTTend; ++i) {
      //Serial.println(EEPROM.read(i));
      if (EEPROM.read(i) == 0) {
        break;
      }
      if (EEPROM.read(i) == 255) {
        break;
      }
      eepromMQTT += char(EEPROM.read(i));
    }
    mqtt_server = eepromMQTT.c_str();
    Serial.print(F("   MQTT broker: ")); Serial.println(mqtt_server);  
  }
}


bool testWifi(void) {
  int c = 0;
  while ( c < 20 ) {
    Serial.print(".");
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    c++;
  }
  return false;
}

void setupWIFI(int webtype) {
  //Determine startup mode
  if (webtype || APssid == "")
  {
    display.print(F("SSID: "));
    display.println(SSIDsetup);
    Serial.println(F("No SSID found!"));
    display.println(F("No SSID found!"));
    delay(2000);
    // -- Setup mode --
    // Confiure WiFi
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
     // Start WiFi as accesspoint
    Serial.print("Start accesspoint...");
    WiFi.softAP(SSIDsetup);
    Serial.println("done");
    display.display();
    // Read IP address
    Serial.print(F("SoftAP IP: ")); Serial.println(WiFi.softAPIP());
    display.print(F("IP  : "));
    display.println(WiFi.softAPIP());
    display.display();
  } else {
    //-- Normal mode --
    // Connect to WiFi access point
    display.println(F("Connecting to AP..."));
    Serial.print("Connect to access point " + APssid);
    WiFi.begin(APssid.c_str(), APpassword.c_str());
    if (testWifi()) {
      Serial.println(" OK");
    } else {
      Serial.println(" FAIL");
    }
    display.display();
        // Read IP address
    Serial.print(F("Local IP: ")); Serial.println(WiFi.localIP());

    // Setup mDNS
    Serial.print(F("Setup mDNS..."));
    if (mdns.begin("RenoventHR", WiFi.localIP())) {
      Serial.println(" OK");
    } else {
      Serial.println(" FAIL");
    }
    display.display();
  }
}

void updateDisplay() {
  //Clear
  display.clearDisplay();

  //Header
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(NodeName);

  //WTW state
  display.setTextSize(2);
  display.print("     ");
  if (currentState == 0) {
    display.write(15);
  } else {
    display.println(currentState);
  }

  //Change Filter
  display.setTextSize(1);
  if (changeFilter == 1) {
    display.setTextColor(BLACK);
    display.fillRect(78, 8, 50, 25, WHITE);
    display.println("        ");
    display.setCursor(80, 11);
    display.println(" CHANGE ");
    display.setCursor(80, 21);
    display.println(" FILTER ");
  }

  //IP address
  display.setTextColor(WHITE);
  display.setCursor(0, 25);
  display.println(WiFi.localIP());

  //Show content on LCD
  display.display();
}
