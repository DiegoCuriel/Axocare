/*
Se incluyen las librearias que seran necesarias para realizar el proyecto
las cuales son arduino.h, Wifi.h, firebase.h, wire.h, y la de temperatura.
*/

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

/*
Se define la red que se estara utilizando para el desarrollo del proyecto, asi
como la contraseña. ademas se escriben la API KEY de la base de datos y la liga 
URL de la misma
*/

#define WIFI_SSID "iPhone de Juan Daniel"
#define WIFI_PASSWORD "cubo4190"
#define API_KEY "AIzaSyD9AGB7gseobpK5GBhH5czsYmN8mAdcqw8"
#define DATABASE_URL "https://axolotl-4c586-default-rtdb.firebaseio.com/" 

//Definir 
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

char *Object_Type[]={"Object","Ambient"};
// Accionadores de control
const int ventilador1 = 19;
const int ventilador2 = 18;
const int ventilador3 = 5;
const int calentador = 4;
const int filtro = 2;
// Valores necesarios para el uso del sensor de PH
float calibration_value = 20.34-0.7;
int phval = 0;
unsigned long int avgval;

// Objeto para el sensor de temperatura
Adafruit_MLX90614 MLX_Sensor = Adafruit_MLX90614();

void setup() {

  Serial.begin(115200);
   
  MLX_Sensor.begin(); 

  pinMode(ventilador1, OUTPUT);
  pinMode(ventilador2, OUTPUT);
  pinMode(ventilador3, OUTPUT);
  pinMode(filtro, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  Firebase.reconnectWiFi(true);
  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);

}
void loop() {
  

  // Programacion para sensor de PH
  float ph_act= 6.97 + ((7.03-6.97)/(710-690)*(analogRead(A0)-710)); 
  Serial.print("El PH es de: ");
  Serial.println(ph_act);

  // Programacion para sensor de temperatura infrarrojo
  float value = Get_Temperature_Sample('A');
  Serial.print("La temperatura en grados celsius es: ");
  Serial.println(value);
  if(value>15){
    digitalWrite(ventilador1, LOW);//Relevador activado
    digitalWrite(ventilador2, LOW);//Relevador activado
    digitalWrite(ventilador3, LOW);//Relevador activado
  }
  else{
    digitalWrite(ventilador1, HIGH);//Relevador desactivado
    digitalWrite(ventilador2, HIGH);//Relevador desactivado
    digitalWrite(ventilador3, HIGH);//Relevador desactivado
  }
  if(value<15){
    digitalWrite(calentador, LOW);//Relevador activado
  }
  else{
    digitalWrite(calentador, HIGH);//Relevador desactivado
  }
  
  if (Firebase.ready()){
    if(Firebase.RTDB.setInt(&fbdo, "/sensores/calentador:", value)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setInt(&fbdo, "/sensores/ph:", ph_act)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.getInt(&fbdo, "/sensores/filtro:")){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      if(fbdo.intData()==1){
        digitalWrite(filtro, LOW);
      }
      else{
        digitalWrite(filtro,HIGH);
      }      
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setInt(&fbdo, "/sensores/ventiladores:", digitalRead(ventilador1))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setInt(&fbdo, "/sensores/calentador:", digitalRead(calentador))){
     Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
  else{
    Serial.println("No se esta conectando con la base de datos. ");
  }

  Serial.println("########");
  delay(2000); // 2 segundos delay
}
float Get_Temperature_Sample(char type){
    float temp_value;
    float Object_Temperature = MLX_Sensor.readObjectTempC();
    float Ambient_Temperature = MLX_Sensor.readAmbientTempC();
   if(type =='E')
   {
    temp_value = MLX_Sensor.readObjectTempF(); //Fah. Object
   }else if(type =='F')
   {
    temp_value = MLX_Sensor.readAmbientTempF();//Fah Ambient
   }else if(type =='C')
   {
    temp_value = Object_Temperature + 273.15;// Object Kelvin
   }else if(type =='D')
   {
    temp_value = Ambient_Temperature + 273.15;//Ambient Kelvin
   }else if(type =='A'){
    temp_value = Object_Temperature;
   }
   else if(type =='B'){
    temp_value = Ambient_Temperature;
    }
   return temp_value;
}
