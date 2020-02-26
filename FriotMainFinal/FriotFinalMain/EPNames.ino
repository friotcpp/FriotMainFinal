void SaveDevice(byte EPcode, char EPName[8], byte EPFP) {

  switch (EPFP) {
    case 0:    //if EPFP = 0
      Serial.println("checking for next available, no break");

    case 1: 
      Serial.println("save device ip");
      break;

  }
   
  EPSave = false;
}





byte saveDeviceIP(char EPIP[8], byte EPFP) {//handlehttprecicveediop
    EPSave = false;
     Serial.print("fpip:");
     Serial.println(EPFP);
 if(EPFP>0&EPFP<7) 
  {   //if EPFP = 0
    LIP[EPFP-1]=EPIP;

    return 0;
  }
 if(EPFP<1||EPFP>6)
   EPFP = 1;
  
  while(EPFP<7)
  { 
    if(LIP[EPFP-1]=="")
    {
      LIP[EPFP-1]=EPIP;
      break;
    }
    
    EPFP++;
  }
  
  return EPFP;
}

void updateAIP(){
 Serial.println("IP update");
                  sendFP =48+saveDeviceIP(tempEPIP,tempEPPlace);
                  Serial.println(sendFP);
                if(sendFP=='0'){

                 Serial.println("nofp update");
                 EPFPUP =false;
                  return;
                }
eek[3]=sendFP;
Serial.println(eek);
  HTTPClient upID;  //Declare an object of class HTTPClient
  upID.begin("http://192.168."+ EPIP +"/place");  //Specify request destination
   upID.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
 
 int httpCode =  upID.POST(eek);   //Send the request
  Serial.print("code:");
   Serial.println(httpCode);

 
  if (httpCode > 0) 
      { //Check the returning code
 
        String payload =  upID.getString();   //Get the request response payload
        Serial.println(payload);                     //Print the response payload

      } 
      EPFPUP =false;
    //  else {Serial.println("no payload from setup");}
}


void decypherType(int tc){
  bool tcc = true; //logic to continue ep type code analysis
  while(tcc){//take last digit of code
    int st=tc%10;
    if(tc<10) tcc=false;
    tc/=10;
  }
}

void handleDeviceName (char EPID, String NfromPhone){
  //epid to int
  int spaceName= 0;
//  setNames[spaceName]= NfromPhone;
}
String findDeviceName(String fromPhone){
  //if fromPhone = setNames[0];
  //return epid[0];
}
