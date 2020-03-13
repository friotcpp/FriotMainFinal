void handleCommand() {

    loopLock++;
    String command = server.arg("Command");
  // String  device = findDeviceName(server.arg("OutputDevice"));//used to find individual ep names
  String device = server.arg("OutputDevice");
  checkColor(device,command);//for main
  epGCI=command[4]-48;//sets Group command indicator from recived command
   if (command[4]=='1'||device=="all"){
    if(command[4]=='1'){ 
      ledFlag =true;
    command.remove(4,1);
    }
    Gcommand = command;
    server.send(200, "text/plain", "Successfully completing Main command: [" + Gcommand + "]");
    loopLock = loopLock +maxfp;
    Serial.print("LoopLock is" + loopLock);
    Serial.println(loopLock);

   

    return;
    
    }

    char devfp = device.charAt(3);
      if(0<devfp-48<maxfp+1)
    {
    //swap with if ip
      server.send(200, "text/plain", "Passing EP command: [" + command + "] to:" + device );
           toEP(devfp,command);
           loopLock--;
      Serial.println("Handling Command: {" + command + "} For: {" + device + "}...");
      return;    
    }
   String color = checkColor(device,command);
    if(color=="error") 
    {  
      server.send(200, "text/plain", "Color Error");
      loopLock--;
      return;
    
    }
      
  
    else server.send(200, "text/plain", "Invaild request or error");
    loopLock--;

}



void toEP(char devfp, String EPCommand){
  setEPIP(devfp);
  if(EPIP==""){
    Serial.print("no ip for device: ");
    Serial.println(devfp);
      return;
   }
   
   Serial.println("Sending command to http://192.168." + EPIP +"/receive");
   http.begin("http://192.168."+EPIP+"/receive");      //Specify request destination
   http.addHeader("Content-Type", "application/x-www-form-urlencoded");
   int httpCode = http.POST("Command="+EPCommand+"&OutputDevice=led"+devfp);   //Send the request
   String payload = http.getString();          //Get the response payload
   Serial.println("Code from EP");
   Serial.println(httpCode);   //Print HTTP return code
   Serial.println(payload);    //Print request response payload
   delay(20);
  
   if(httpCode>0){
        Firebase.setString(firebaseData,  "/Device"+devfp , payload );
    
    
    }

  
  }// end toEP
