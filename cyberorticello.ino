#include <Adafruit_NeoPixel.h>
#include "DHT.h"

#define TIPO_SENSORE_UMIDITA      DHT11 

// valori di configurazione

#define PIN_SENSORE_UMIDITA_ARIA  12
#define PIN_SENSORE_UMIDITA_TERRAD 8
#define PIN_SENSORE_UMIDITA_TERRAA A0
#define ACCENSIONE_SENSORE_TERRA    5
#define PIN_BOTTONE               10
#define PIN_POMPA                 4
#define PIN_ANELLO_LED            6
// Numero led dell'anello
#define NUMPIXELS                 16

// dichiarazione sensore umidità e temperatura
DHT aria(PIN_SENSORE_UMIDITA_ARIA, TIPO_SENSORE_UMIDITA);

// dichiarazione della striscia led
Adafruit_NeoPixel anello_led = Adafruit_NeoPixel(NUMPIXELS, PIN_ANELLO_LED, NEO_GRB + NEO_KHZ800);

bool stato_bottone=false;
int stato_programma=0;
int umidita_terreno=0;
long unsigned tempo_ultima_misura=0;
long unsigned delay_time= (long unsigned)60000*10; // 10 minuti
long unsigned innaffiatura_time = 1200;

void setup() {
  
  //inizializzazione seriale
  Serial.begin(115200);

  // inizializzazione librerie
  aria.begin();
  anello_led.begin(); // Si inizializza la libreria NeoPixel.
  anello_led.setBrightness(10); //Si regola la luminosità

  // inizializzazione pin
  pinMode(PIN_BOTTONE, INPUT);
  pinMode(PIN_SENSORE_UMIDITA_TERRAD, INPUT);
  pinMode(PIN_POMPA, OUTPUT);
  pinMode(ACCENSIONE_SENSORE_TERRA, OUTPUT);
  digitalWrite(ACCENSIONE_SENSORE_TERRA, LOW);
  digitalWrite(PIN_POMPA, HIGH);
  misura_terra();
  stato_bottone=digitalRead(PIN_BOTTONE);
  
}
 
void loop()
{
  if(millis()-tempo_ultima_misura>delay_time){
    misura_terra();
  }
  if(digitalRead(PIN_BOTTONE)==HIGH && stato_bottone==false){
     stato_bottone=true;
     stato_programma=stato_programma+1;
  }
  if(digitalRead(PIN_BOTTONE)==LOW && stato_bottone==true){
     stato_bottone=false;
     stato_programma=stato_programma+1;
  }
  
  if (stato_programma==1){
    int s= digitalRead(PIN_BOTTONE);
    int valore = aria.readTemperature();
    Serial.print("sensore di temperatura = ");
    Serial.println(valore);
      while(stato_bottone==digitalRead(PIN_BOTTONE)){
          visualizza_valore_2(anello_led.Color(255, 0, 0), valore);
        }
  } else if (stato_programma==2){
    int s= digitalRead(PIN_BOTTONE);
    int valore =  aria.readHumidity();
    Serial.print("sensore di umidita = ");
    Serial.println(valore);
    
      while(stato_bottone==digitalRead(PIN_BOTTONE)){
          visualizza_valore_2(anello_led.Color(0, 0, 255), valore);
        }
  } else if (stato_programma==3){
    int s= digitalRead(PIN_BOTTONE);
    Serial.print("sensore di umidita del terreno = ");
    int valore =  map(umidita_terreno, 580, 1006, 16, 1);
    if (valore==0){
      valore=1;
    }
    Serial.println(valore);
 
      while(stato_bottone==digitalRead(PIN_BOTTONE)){
        disegna_n_pixel(anello_led.Color(0, 255, 0), valore);
        }
  }  else {
    disegna_n_pixel(anello_led.Color(0, 0, 0), 0);
    stato_programma=0;580
  }
}

void misura_terra(){
  Serial.println("misura terra");
  digitalWrite(ACCENSIONE_SENSORE_TERRA, HIGH);
  delay(1000);
  Serial.print("sensore di umidita del terreno = ");
  umidita_terreno=analogRead(PIN_SENSORE_UMIDITA_TERRAA);
  Serial.println(umidita_terreno);
  tempo_ultima_misura=millis();
  

  if(digitalRead(PIN_SENSORE_UMIDITA_TERRAD)==HIGH){
    digitalWrite(PIN_POMPA, LOW);
    delay(innaffiatura_time);
    digitalWrite(PIN_POMPA, HIGH);
  }
  
  digitalWrite(ACCENSIONE_SENSORE_TERRA, LOW);
}
void visualizza_valore_2(uint32_t colore, int valore){
  int valore_bottone = digitalRead(PIN_BOTTONE);
  int seconda_cifra = valore%10;
  int prima_cifra = valore/10; 

  // disegno della prima cifra
  disegna_n_pixel(colore, prima_cifra);
   unsigned long tempo_inizio= millis();
   while(valore_bottone==digitalRead(PIN_BOTTONE) && millis()-tempo_inizio<3000){}//
  if(valore_bottone!=digitalRead(PIN_BOTTONE))
    return;
  disegna_n_pixel(colore, seconda_cifra);
  tempo_inizio = millis();
   while(valore_bottone==digitalRead(PIN_BOTTONE)&& millis()-tempo_inizio<3000){}
  anello_led.clear();
  delay(200);
}


void disegna_n_pixel(uint32_t colore, int n){
  for (int i=0; i<NUMPIXELS; i++){
    if (i<n){
      anello_led.setPixelColor(i, colore);
    }else{
      anello_led.setPixelColor(i, anello_led.Color(0, 0,0));
    }
  }
  anello_led.show();
}
