#include <ESP8266WiFi.h>                 
#include <FirebaseESP8266.h>         
#include <BH1750FVI.h>
#include <SoftwareSerial.h>
#define FIREBASE_HOST "embedded-lab-c677b-default-rtdb.asia-southeast1.firebasedatabase.app"      
#define FIREBASE_AUTH "XfhAd8Sok9wWN09dYZ8Yh3gHMZ5FACFW1NtJCpL0"            
#define WIFI_SSID "TP-Link_5476"                                  
#define WIFI_PASSWORD "51030615"            
                                                   
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);
SoftwareSerial mySerial;

const byte RX = D7;
const byte TX = D8;

bool isOn = false;
String btn_state, fireHumid, fireLight, append_text, test, humid;
uint16_t lux; 

//---------------- Read UART --------------------------- 
String st = "";
char inChar = ' ';
int A, B;

FirebaseData fData;

void Read_Uart();

void setup() {
  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);
  
  Serial.begin(115200);
  mySerial.begin(9600, SWSERIAL_8N1, RX, TX, false);
  LightSensor.begin();            
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                  
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
  Serial.println();
  Serial.print("Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); //prints local IP address
 
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // connect to the firebase
}

void loop() {
  Read_Uart();
  //---------------- Read light intensity value and push to firebase --------------------
  if (isnan(lux)) { // Checking sensor working                                  
    Serial.println(F("Failed to read from BH1750FVI sensor!"));
    return;
  } else {
    lux = LightSensor.GetLightIntensity();
    fireLight = String(lux);
    Firebase.pushString(fData,"/IOT/BH1750FVI/LIGHT", fireLight);
  }
//---------------- Receive humidity and push to firebase --------------------
  try {
    if (test == "") {
      throw(String("wait..."));
    } else {
      if (append_text == "") {
        fireHumid = String (humid);
        Firebase.pushString(fData, "/IOT/DHT11/HUMD", fireHumid);
      }
    }
  }
  catch (String err_message) {
    Serial.println(err_message);
  }
//----------- Read button state from firebase --------------------------
  if (Firebase.getString(fData, "/state/state1")) {
    btn_state = fData.stringData();
    if (btn_state == "1" && !isOn) {
      Serial.println("on");
      isOn = !isOn;  
    } else if (btn_state == "0" && isOn) {
      Serial.println("off");
      isOn = !isOn;
    }
  } 
}


void Read_Uart() {
  st = "";
  while (mySerial.available()) {
    inChar = (char)mySerial.read();
    if (inChar == '\n') {
      A = append_text.indexOf("A");
      B = append_text.indexOf("\r");
      st = append_text.substring(A + 1, B);
      append_text = "";
      test = st;
      
      if (test != "") {
        humid =test;
      }
      return;
    }
    
    if (append_text.length() > 6 ) {
      append_text = "";
    } else {
      append_text += inChar;
    }  
  }
}
