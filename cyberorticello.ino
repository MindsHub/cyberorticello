#include <Adafruit_NeoPixel.h>

 
//Pin a cui è collegato l'anello di led
#define PIN_RING            10
#define PIN_TEMPSENSOR      12
#define PIN_BUTTON          8

// Numero led dell'anello
#define NUMPIXELS      16
 
//Impostazione dei parametri
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_RING, NEO_GRB + NEO_KHZ800);
 
void setup() {
  pinMode(PIN_BUTTON, INPUT);

  pixels.begin(); // Si inizializza la libreria NeoPixel.
  pixels.setBrightness(10); //Si regola la luminosità
}
 
void loop()
{
  colorWipe(pixels.Color(255, 255, 255), 0.8); //white
}




// Riempie uno dopo l'altro i led con un colore
void colorWipe(uint32_t c, float value) {
  if (value > 1.0) {
    value = 1.0;
  }

  if (value < 0.0) {
    value = 0.0;
  }

  int iter = (int)(value*(float)pixels.numPixels());

  for (int i = 0; i <= iter; i++) {
    pixels.setPixelColor(i, c);
  }

  for (int i =iter+1; i<pixels.numPixels(); i++){
    pixels.setPixelColor(i, pixels.Color(0, 0, 0) );
  }

  pixels.show();
}