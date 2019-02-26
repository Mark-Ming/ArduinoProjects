#include <M5Stack.h>

int last_value_red = 0;
int cur_value_red = 0;
int last_value_blue = 0;
int cur_value_blue = 0;

int count = 100;
int firing = 0;
int last_time = 0;

void setup()
{
  M5.begin();
  delay(500);
  Serial.begin(115200);

  pinMode(21, INPUT);
  pinMode(22, INPUT);

  Serial2.begin(9600);              //音频模块使用
  volume(0x1E); //音量设置0x00-0x1E

  M5.Lcd.fillScreen(BLUE);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(20);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.printf("COUNT:");
  M5.Lcd.setCursor(110, 100);
  M5.Lcd.setTextSize(80);
  M5.Lcd.printf("%d", count);
}

void loop()
{ 
  cur_value_red = digitalRead(21);
  cur_value_blue = digitalRead(22);

  if(firing && (millis() - last_time >= 200))
  {
    count--;
    last_time = millis();
    M5.Lcd.setCursor(110, 100);
    M5.Lcd.fillRect(110, 100, 150, 100, BLUE);
    M5.Lcd.printf("%d", count);
  }

  //开火
  if(cur_value_blue != last_value_blue){
    if(cur_value_blue==0){ //蓝色按下
      firing = 1;
      Serial.println("Button Status: blue pressed");
      play(0x02);
    }
    else{ //蓝色松开
      Serial.println("Button Status: blue released");
      firing = 0;
      stopMusic();
    }
    last_value_blue = cur_value_blue;
  }
  //换子弹
  if(cur_value_red != last_value_red){
    if(cur_value_red==0){ //红色按下
      Serial.println("Button Status: red pressed");
      play(0x06);
      count = 100;
       M5.Lcd.setCursor(110, 100);
      M5.Lcd.fillRect(110, 100, 150, 100, BLUE);
    M5.Lcd.printf("%d", count);
    }
    else{ //红色松开
      Serial.println("Button Status: red released");
    }
    last_value_red = cur_value_red;
  }
  M5.update();
}

/*音频相关*/
void play(unsigned char Track)
{
  unsigned char play[6] = {0xAA, 0x07, 0x02, 0x00, Track, Track + 0xB3}; //0xB3=0xAA+0x07+0x02+0x00,即最后一位为校验和
  Serial2.write(play, 6);
}
void volume( unsigned char vol)
{
  unsigned char volume[5] = {0xAA, 0x13, 0x01, vol, vol + 0xBE}; //0xBE=0xAA+0x13+0x01,即最后一位为校验和
  Serial2.write(volume, 5);
}
void stopMusic()
{
  unsigned char play[4] = {0xAA, 0x04, 0x00, 0xAE};
  Serial2.write(play, 4);
}
