void setupMode(){

      
    if(timeOUT==true){//if timeout bool true
      timeOUT=false;//boolean so time is not set again
      timeStart=millis();//timestamp
      Serial.println("Time out timer begins");
    }
    
    if((timeStart+90000<millis()) ){//times out after 90sec
      endSetup();
  Serial.println("Setup time out");
      return; //comes out if endpoint setup loop    
    }
      endpointSetup();//checks for client in a void, if no timeout

      
//     if(!setupEndpointGO){ //this is just a loop simulating an ISR
// Serial.println("Restarting setup mode");
// delay(2500);
// setupEndpointGO=true;  }
}


void endpointSetup(){//endpoint setup function
 WiFiClient client = wifiserver.available();//checks for client
 if (!client) {return;}//doesnot continue till client, exits endpointSetup()
//maybe add nointerrupt ()?, needs testing
   digitalWrite(LED_BUILTIN, LOW);//connection idicator
   String endPointINFO = client.readStringUntil('\r');//read sent data
   Serial.println("********************************");
   Serial.println("From the station: " + endPointINFO);
   client.flush();
   
   //decoding json payload from endpoint client
   const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
      DynamicJsonDocument jsonBuffer(capacity);//creates buufer
   // Parse JSON object
    auto error = deserializeJson(jsonBuffer, endPointINFO);
    if (error) {//deserialize package error
      Serial.println(F("Parsing failed!"));
      return;//exits endpointSetup() if parsing error
    }
     if (jsonBuffer["ESPAUTH"].as<int>()==42){//if esp authenticated, password is 42
      Serial.print("Bytes sent to the station: ");
      Serial.println(client.println(wifiINFO + '\r'));//send pre-made json with wifi info
          //^lets you know how many bytes were sent without showing json payload
          //action is in client.println()
      //setupEndpointGO = false;//pairing mode successful on main side
      Serial.println("Authenticated Client");
      Serial.print("Information sending to the station: ");
      Serial.println(wifiINFO);//only for debugging purposes   
      EPSave = true;     
     }
     if (EPSave = true) {
endSetup();
  //    SaveDevice(jsonBuffer["type"].as<byte>(), strncpy(tempName, jsonBuffer["Name"].as<char*>(), sizeof(tempName)),jsonBuffer["fp"].as<byte>());
      }

    Serial.println(F("Info Payload:"));
    Serial.println(jsonBuffer["type"].as<char*>());
    Serial.println(jsonBuffer["Name"].as<char*>());
    Serial.println(jsonBuffer["ESPAUTH"].as<int>());
        Serial.println(jsonBuffer["fp"].as<byte>());
    digitalWrite(LED_BUILTIN, HIGH);
 
// if (jsonBuffer["name"].as<int>()==42)//future login/identifying save
}//end setup mode loop

//===================================
void endSetup(){
        setupEndpointGO = false;//comes out of setup 'if' in main loop
      timeOUT=true;//reset new timer  
      Serial.println("Setup mode reset");
      digitalWrite(antiInterrupt, 0); //lets the setup pin be used again
  }

  
void loadWifiInfo() {//creates a json package

  Serial.println("Creating JSON payload with wifi info");
  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<500> jsonBuf;
 
  // Create the root object
  DynamicJsonDocument root(1024);
 String mainIP = WiFi.localIP().toString();
   Serial.println("mainIP before remove ="+mainIP);
      mainIP.remove(0,8);
         Serial.println("mainIP after remove ="+mainIP);
         if(mainIP=="t)"){mainIP ="unset";}
  root["SSID"] = ssid; //ssid wifi info
  root["PASS"] = password; //password wifi info
  root["MIP"] =   mainIP;
  root["KEY"] = inWifiKey; //if connected to wifi, flag
  serializeJson(root,wifiINFO);  //Store JSON in String variable
  }
//===================================================================//
