#include "DFRobotDFPlayerMini.h"
#include <CBButton.h>

//MP3播放对象
DFRobotDFPlayerMini mp3;
//计算时间间隔变量
static unsigned long timer = millis();

//定义控制按钮
CBButton red(4);
CBButton green(5);
CBButton blue(6);
CBButton yellow(7);


void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);

  mp3.begin(Serial1);

  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  mp3.volume(15);  //设置音量  0 - 30
  mp3.play(1);  //播放第一首曲目
  mp3.enableLoopAll(); //列表循环
}

void loop() {
  if (millis() - timer > 50) {
    timer = millis();
    if (red.keyDown())
    {
//      Serial.println(red.keyDown());
      //下一曲
      mp3.next();
    }
    if (green.keyDown())
    {
      //上一曲
      mp3.previous();
    }
    if (blue.keyDown())
    {
      //暂停
      mp3.pause();
    }
    if (yellow.keyDown())
    {
      //暂停
      mp3.start();
    }
  }
}

