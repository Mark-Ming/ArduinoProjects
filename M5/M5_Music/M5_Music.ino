#include <M5Stack.h>
#include <WiFi.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;

int last_value_red = 0;
int cur_value_red = 0;
int last_value_blue = 0;
int cur_value_blue = 0;

void setup()
{
  M5.begin();
  WiFi.mode(WIFI_OFF); 
  delay(500);
  M5.Speaker.tone(80);

  pinMode(21, INPUT);
  pinMode(22, INPUT);
  
  M5.Lcd.setTextFont(2);
  M5.Lcd.printf("Sample WAV playback begins...\n");
  Serial.printf("Sample WAV playback begins...\n");
  
  // pno_cs from https://ccrma.stanford.edu/~jos/pasp/Sound_Examples.html
  file = new AudioFileSourceSD("/gun.wav");
  out = new AudioOutputI2S(0, 1); // Output to builtInDAC
  out->SetOutputModeMono(true);
  wav = new AudioGeneratorWAV(); 
}

void loop()
{
  cur_value_red = digitalRead(21);
  cur_value_blue = digitalRead(22);

  M5.Lcd.setCursor(0,25); M5.Lcd.print("Blue Status: ");
  M5.Lcd.setCursor(0,45); M5.Lcd.print("Blue Value: ");
  M5.Lcd.setCursor(0,65); M5.Lcd.print(" Red Status: ");
  M5.Lcd.setCursor(0,85); M5.Lcd.print(" Red Value: ");

  if(cur_value_blue != last_value_blue){
    if(cur_value_blue==0){ //蓝色按下
      Serial.println("Button Status: blue pressed");
    }
    else{ //蓝色松开
      Serial.println("Button Status: blue released");
    }
    last_value_blue = cur_value_blue;
  }
  if(cur_value_red != last_value_red){
    if(cur_value_red==0){ //红色按下
      Serial.println("Button Status: red pressed");
      playMusic();
    }
    else{ //红色松开
      Serial.println("Button Status: red released");
      wav->stop();
    }
    last_value_red = cur_value_red;
  }
}
