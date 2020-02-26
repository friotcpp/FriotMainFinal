void handleCommand() {
  digitalWrite(led, 1);
  String command = server.arg("Command");
// String  device = findDeviceName(server.arg("OutputDevice"));//used to find individual ep names
String device = server.arg("OutputDevice");
  String color = checkColor(device,command);
  server.send(200, "text/plain", "Successfully Received Code: [" + command + "]\nColor: [" + color + "]");
if(color=="error") return;
  if(device!="led1"||device =="All led")
   toEP(device,command);
  Serial.println("Handling Command: {" + command + "} For: {" + device + "}...");
  //Serial.println("The Led is now: " + color + "!");
 // digitalWrite(led, 0);
}

void toEP(String device, String EPCommand){



 //String epreq = "http://192.168."+EPIP+"/recieve";
      Serial.println("Sending command to http://192.168." + EPIP +"/receive");

   http.begin("http://192.168."+EPIP+"/receive");      //Specify request destination
   http.addHeader("Content-Type", "application/x-www-form-urlencoded");

   int httpCode = http.POST("Command="+EPCommand+"&OutputDevice="+device);   //Send the request
   String payload = http.getString();          //Get the response payload
   Serial.println("Code from EP");
   Serial.println(httpCode);   //Print HTTP return code
   Serial.println(payload);    //Print request response payload
 //  http.end();  //Close connection
  
 
  
  }
