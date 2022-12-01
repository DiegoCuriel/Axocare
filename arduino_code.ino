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
const int calentador = 0;
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
  while(!Firebase.signUp(&config, &auth, "", "")){
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
    delay(100);
  }
  Serial.println("ok");
  signupOK = true;
 
  Firebase.begin(&config, &auth);

}
void loop() {
  //Variables necesarias para el funcionamiento, la condicion determina si el sistema esta encendido o apagado, y los valores son para pasar a la 
  // base de datos 
  bool condicion;
  int valorFiltro;
  int valorVentilador;
  int valorCalentador;
  // Se lee el valor de apagado o encendido en la aplicacion y se actualiza el valor de la condicion.
  if(Firebase.RTDB.getString(&fbdo, "/status/onoff")){
      Serial.println("Se obtuvo el estado de on/off");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.println(fbdo.stringData());
      if(fbdo.stringData()=="1"){
        condicion = true;
        Serial.println("ENCENDIDO");
      }
      else{
        condicion = false;
        Serial.println("APAGADO"); 
      }
    }
  if(condicion){
    
    digitalWrite(filtro, LOW);
  // Programacion para sensor de PH
    float ph_act= 6.97 + ((7.03-6.97)/(710-690)*(analogRead(A0)-710)); 
    Serial.print("El PH es de: ");
    Serial.println(ph_act);

    // Programacion para sensor de temperatura infrarrojo
    float value = Get_Temperature_Sample('A');
    Serial.print("La temperatura en grados celsius es: ");
    
    Serial.println(value);

    if(value>=23){
      digitalWrite(ventilador1, LOW);//Relevador activado
      digitalWrite(ventilador2, LOW);//Relevador activado
      digitalWrite(ventilador3, LOW);//Relevador activado
      digitalWrite(calentador, HIGH);
          }
    else{
      digitalWrite(ventilador1, HIGH);//Relevador desactivado
      digitalWrite(ventilador2, HIGH);//Relevador desactivado
      digitalWrite(ventilador3, HIGH);//Relevador desactivado
      digitalWrite(calentador, LOW);
    }
    
    if (Firebase.ready()){
      value = int(value);
      if(Firebase.RTDB.setInt(&fbdo, "/sensores/temperatura", value)){
        Serial.println("Se paso el valor de la temperatura actual");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      if(Firebase.RTDB.setInt(&fbdo, "/sensores/ph", ph_act)){
        Serial.println("Se paso el valor de los sensores de PH");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      if(digitalRead(filtro)==0){
        valorFiltro=1;
      }
      else{
        valorFiltro=0;
      }
      if(Firebase.RTDB.setInt(&fbdo, "/sensores/filtro", valorFiltro)){
        Serial.println("Se paso el estado del filtro de agua");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());    
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      
      if(digitalRead(ventilador1) == 0){
        valorVentilador = 1;
      }
      else{
        valorVentilador = 0;
      }
      if(Firebase.RTDB.setInt(&fbdo, "/sensores/ventiladores", valorVentilador)){
        Serial.println("Se paso el estado de los ventiladores");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if(digitalRead(calentador) == 0){
        if(Firebase.RTDB.setInt(&fbdo, "/sensores/calentador", 1)){
          Serial.println("Se paso el estado del calentador");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
        }
        else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }
      }
      else{
        if(Firebase.RTDB.setInt(&fbdo, "/sensores/calentador", 0)){
          Serial.println("Se paso el estado del calentador");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
        }
        else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }
      }
      
  }
  else{
    Serial.println("No se esta conectando con la base de datos. ");
  }
  Serial.println("###################################################");
  delay(3000); // 2 segundos delay
  }
  // Si el sistema se apaga se apagan los actuadores y se actualizan todos los datos en la base de datos
  else{
    Serial.println("La conexion se ha terminado desde la aplicación");
    //Se apagan todos los actuadores
    digitalWrite(ventilador1, HIGH);//Relevador desactivado
    digitalWrite(ventilador2, HIGH);//Relevador desactivado
    digitalWrite(ventilador3, HIGH);//Relevador desactivado
    digitalWrite(calentador, HIGH);//Relevador desactivado
    digitalWrite(filtro, HIGH);
    // Se actualizan los valores en la firebase
     if(Firebase.RTDB.setInt(&fbdo, "/sensores/temperatura", 0)){
      Serial.println("Se paso el valor de la temperatura actual");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setInt(&fbdo, "/sensores/ph", 0)){
      Serial.println("Se paso el valor de los sensores de PH");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    if(Firebase.RTDB.setInt(&fbdo, "/sensores/filtro", 0)){
      Serial.println("Se paso el estado del filtro de agua");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());    
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setInt(&fbdo, "/sensores/ventiladores", 0)){
      Serial.println("Se paso el estado de los ventiladores");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setInt(&fbdo, "/sensores/calentador", 0)){
     Serial.println("Se paso el estado del calentador");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    Serial.println("###################################################");
    delay(3000);
  }

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