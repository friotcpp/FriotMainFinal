void SaveDevice(int EPcode, char EPName) {
  
  if (EPName ==epid[0])
  epcodes[0]= EPcode;
  if (EPName ==epid[1])
  epcodes[1]= EPcode;
  if (EPName ==epid[2])
  epcodes[2]= EPcode;
  if (EPName ==epid[3])
  epcodes[3]= EPcode;
  if (EPName ==epid[4])
  epcodes[4]= EPcode;
   
  EPSave = false;
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
  setNames[spaceName]= NfromPhone;
}
String findDeviceName(String fromPhone){
  //if fromPhone = setNames[0];
  //return epid[0];
}
