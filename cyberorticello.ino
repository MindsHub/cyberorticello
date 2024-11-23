#include <Adafruit_NeoPixel.h>
#include "DHT.h"

#define TIPO_SENSORE_UMIDITA      DHT11 

// valori di configurazione

#define PIN_SENSORE_UMIDITA_ARIA  12
#define PIN_SENSORE_UMIDITA_TERRAD 8
#define PIN_SENSORE_UMIDITA_TERRAA A0
#define ACCENSIONE_SENSORE_TERRA    5
#define PIN_BOTTONE               10
#define POMPA_PIN                 4
#define PIN_ANELLO_LED            6
// Numero led dell'anello
#define NUMPIXELS                 16

// valori di configurazione del umidità del terreno
#define TERRENO_COMPLETAMENTE_UMIDO 580
#define TERRENO_COMPLETAMENTE_SECCO 1006

// stato iniziale del programma
int stato_programma=0;

//quanto aspettare tra una misurazione e l'altra del terreno
long unsigned intervallo_misura_terra= (long unsigned)60000*10; // 10 minuti

//tempo dell'inffiatura
long unsigned innaffiatura_time = 1200;

// numero in ms del tempo massimo da rimanere in una modalità display
long unsigned max_display_time = 20000;
// dichiarazione sensore umidità e temperatura
DHT aria(PIN_SENSORE_UMIDITA_ARIA, TIPO_SENSORE_UMIDITA);

// dichiarazione della striscia led
Adafruit_NeoPixel anello_led = Adafruit_NeoPixel(NUMPIXELS, PIN_ANELLO_LED, NEO_GRB + NEO_KHZ800);


// variabili globali
bool stato_bottone; // il bottone è premuto o meno?
int umidita_terreno; // ultima misurazione del terreno
long unsigned tempo_ultima_misura; // quando è stata effettuata l'ultima misura?






void setup() {
  
  //inizializzazione seriale
  Serial.begin(115200);

  // inizializzazione librerie
  aria.begin();
  anello_led.begin(); // Si inizializza la libreria NeoPixel.
  anello_led.setBrightness(10); //Si regola la luminosità

  // inizializzazione pin
  pinMode(PIN_BOTTONE, INPUT_PULLUP);
  pinMode(PIN_SENSORE_UMIDITA_TERRAD, INPUT);
  pinMode(POMPA_PIN, OUTPUT);
  pinMode(ACCENSIONE_SENSORE_TERRA, OUTPUT);
  digitalWrite(ACCENSIONE_SENSORE_TERRA, LOW);
  
  // la pompa funziona al contrario, HIGH significa spento
  digitalWrite(POMPA_PIN, HIGH);
  
  // misuriamo il terreno per avere un valore
  misura_terra();
  
  // qual'è lo stato iniziale del bottone?
  stato_bottone=digitalRead(PIN_BOTTONE);
  
}
 
void loop()
{
  // quando è passato intervallo_misura_terra dall'ultima misurazione del terreno, facciamo un altra misurazione
  if(millis()-tempo_ultima_misura>intervallo_misura_terra){
    misura_terra();
  }
  // se il bottone non ha lo valore che ci siamo salvati
  if(digitalRead(PIN_BOTTONE)!=stato_bottone){
    // ci salviamo il nuovo valore, e incrementiamo lo stato del programma
      stato_bottone=digitalRead(PIN_BOTTONE);
     stato_programma=stato_programma+1;
  }

  // a seconda dello stato eseguiamo varie operazioni
  if (stato_programma==0){
      // se lo stato è 0 non facciamo niente
  } else if (stato_programma==1){
    // se lo stato è 1, leggiamo la temperatura
    int valore = aria.readTemperature();
    
    // scriviamo su seriale il valore letto
    Serial.print("sensore di temperatura = ");
    Serial.println(valore);

    // leggiamo lo stato attuale del bottone. se cambia usciamo dal while
    unsigned long start=millis();
    bool cur_bottone= digitalRead(PIN_BOTTONE);
    while((cur_bottone==digitalRead(PIN_BOTTONE))){
        // se abbiamo visualizzato il valore per troppo tempo
        if (millis()-start>max_display_time){
          //impostiamo lo stato a 0, e finiamo il loop
          stato_programma=4;
          return;
        }
        // visualizziamo il valore della temperatura sull'anello led in rosso
        visualizza_valore_2(anello_led.Color(255, 0, 0), valore);
    }
  } else if (stato_programma==2){
    // se lo stato è 2 leggiamo l'umidità
    int valore =  aria.readHumidity();
    // scriviamo su seriale il valore letto
    Serial.print("sensore di umidita = ");
    Serial.println(valore);
    
    // leggiamo lo stato attuale del bottone. se cambia usciamo dal while
    unsigned long start=millis();
    bool cur_bottone= digitalRead(PIN_BOTTONE);
    while(cur_bottone==digitalRead(PIN_BOTTONE)){
        // se abbiamo visualizzato il valore per troppo tempo
        if (millis()-start>max_display_time){
          //impostiamo lo stato a 0, e finiamo il loop
          stato_programma=4;
          return;
        }
        // visualizziamo il valore dell'umidità sull'anello led in blu
        visualizza_valore_2(anello_led.Color(0, 0, 255), valore);
    }
  } else if (stato_programma==3){
    // calcoliamo il valore dell'umidità del terreno in base ai valori di calibrazione
    int valore =  map(umidita_terreno, TERRENO_COMPLETAMENTE_UMIDO, TERRENO_COMPLETAMENTE_SECCO, 16, 1);
    // se è più piccolo di 1 lo mettiamo a 1
    if (valore<1){
      valore=1;
    }
    
    // scriviamo su seriale il valore letto
    Serial.print("sensore di umidita del terreno = ");
    Serial.println(valore);
    
    // leggiamo lo stato attuale del bottone. se cambia usciamo dal while
    unsigned long start=millis();
    int cur_bottone= digitalRead(PIN_BOTTONE);
    while(cur_bottone==digitalRead(PIN_BOTTONE)){
      // se abbiamo visualizzato il valore per troppo tempo
        if (millis()-start>max_display_time){
          //impostiamo lo stato a 0, e finiamo il loop
          stato_programma=4;
          return;
        }
        // visualizza il valore dell'umidità del terreno in verde, 1 pixel= secco completo, 16 pixel = molto umido
      disegna_n_pixel(anello_led.Color(0, 255, 0), valore);
    }
  }  else {
    // pulisci i led, e imposta lo stato a 0
    disegna_n_pixel(anello_led.Color(0, 0, 0), 0);
    stato_programma=0;
  }
}

// funzione che controlla tutto ciò che serve per misurare il terreno
void misura_terra(){
  // accendiamo il sensore dell'umidità del terreno
  digitalWrite(ACCENSIONE_SENSORE_TERRA, HIGH);
  //aspettiamo un secondo
  delay(1000);
  // misuriamo l'umidità del terreno analogica, e ci salviamo il valore da parte
  umidita_terreno=analogRead(PIN_SENSORE_UMIDITA_TERRAA);

  // scriviamo su seriale il valore appena letto
  Serial.print("sensore di umidita del terreno = ");
  Serial.println(umidita_terreno);

  // imposto il tempo dell'ultima misurazione al tempo corrente
  tempo_ultima_misura=millis();

  // se il segnale digitale del sensore del terreno è alto
  if(digitalRead(PIN_SENSORE_UMIDITA_TERRAD)==HIGH){
    // accendiamo la pompa
    digitalWrite(POMPA_PIN, LOW);
    //aspettiamo il tempo necessario
    delay(innaffiatura_time);
    //spegnamo la pompa
    digitalWrite(POMPA_PIN, HIGH);
  }
  
  //spegnamo il sensore
  digitalWrite(ACCENSIONE_SENSORE_TERRA, LOW);
}

// funzione per visualizzare il valore del sensore sul display di pixel. può stampare massimo valori di 2 cifre, e non funziona con i numeri negativi
void visualizza_valore_2(uint32_t colore, int valore){
  
  
  // calcolo le cifre del valore
  int seconda_cifra = valore%10;
  int prima_cifra = valore/10; 

  // disegno della prima cifra
  disegna_n_pixel(colore, prima_cifra);
  
  // mi salvo il tempo di inizio e il valore del bottone
  unsigned long tempo_inizio= millis();
   int valore_bottone = digitalRead(PIN_BOTTONE);
  //finche non passano 3 secondi, o l'utente preme un bottone
  while(valore_bottone==digitalRead(PIN_BOTTONE) && millis()-tempo_inizio<3000){}

  // se il bottone è stato premuto non facciamo altro e usciamo
  if(valore_bottone!=digitalRead(PIN_BOTTONE))
    return;
  // disegniamo la seconda cifra
  disegna_n_pixel(colore, seconda_cifra);
  // ci salviamo il tempo di inizio
  tempo_inizio = millis();
  //finche non passano 3 secondi, o l'utente preme un bottone
   while(valore_bottone==digitalRead(PIN_BOTTONE)&& millis()-tempo_inizio<3000){}
   // pulisci l'anello led
  anello_led.clear();
  delay(200);
}

// disegna i pixel a schermo
void disegna_n_pixel(uint32_t colore, int n){
  // per ogni pixel
  for (int i=0; i<NUMPIXELS; i++){
    if (i<n){
      // se l'indice del pixel è minore del valore, allora lo coloro del colore
      anello_led.setPixelColor(i, colore);
    }else{
      // altrimenti lo lascio spento
      anello_led.setPixelColor(i, anello_led.Color(0, 0,0));
    }
  }
  // visualizza i dati sulla striscia led
  anello_led.show();
}
