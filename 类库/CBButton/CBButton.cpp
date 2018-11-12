#include <Arduino.h>
#include "CBButton.h"

CBButton::CBButton(int pin)
{
  pinMode(pin, INPUT);
  pinNum = pin;
  btn_pressed = false;

  btnDownStartTime = 0;
}



bool CBButton::keyDown()
{

  if (millis() - this->btnDownStartTime > 100) {

    if (digitalRead(pinNum) == HIGH) {
      this->btn_pressed = false;
    }
    this->btnDownStartTime = millis();
  }
    
  if (digitalRead(pinNum) == LOW && btn_pressed == false) {

    this->btn_pressed = true;
    return true;
  }


  else {
    return false;
  }


}




