#define ROCKER_LEFT  16  
#define ROCKER_RIGHT 32
#define ROCKER_UP    48
#define ROCKER_DOWN  64

#define ROCKER_ADVANCE 80
#define ROCKER_RETREAT 96

#define BUTTON 112  //除了遥杆的按钮情况

#define HIGH_FOUR 240  //取出高四位key部分使用  按位与
#define LOW_FOUR  15   //取出低四位data部分使用  按位与

int key_x = 0;
int key_y = 0;
int key_r_y = 0;
int key_button = 0;

int value_x = 0;
int value_y = 0;
int value_r_y = 0;
int value_button = 0;

int LX = 127; //初始值
int LY = 127; //初始值
int RY = 127; //初始值

unsigned char arr[4] = {};
void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600);
  //  设置模式开启
  //  pinMode(20, OUTPUT);
  //  digitalWrite(20, LOW);

  //  Serial1.print("AT+C200\r\n");
  //  Serial1.print("AT+RX\r\n");
  //  String str = Serial1.readString();
  //  Serial.println(str);
}

void loop()
{
  Serial1.readBytes(arr, 4);
  handleData(arr);

  //  Serial.print("x信息");
  //  Serial.print(key_x);
  //  Serial.print(":");
  //  Serial.print(value_x);
  //  Serial.print("  y信息");
  //  Serial.print(key_y);
  //  Serial.print(":");
  //  Serial.print(value_y);
  //  Serial.print("  key_r_y信息");
  //  Serial.print(key_r_y);
  //  Serial.print(":");
  //  Serial.println(value_r_y);
 
  Serial.print("x:");
  Serial.print(LX);
  Serial.print("  y:");
  Serial.print(LY);
  Serial.print("  ry:");
  Serial.println(RY);
  
  delay(50);
}

//处理接受到的数据包
void handleData(unsigned char *arr)
{
  key_x = arr[0] & HIGH_FOUR;
  value_x = arr[0] & LOW_FOUR;
  switch (key_x) {
    case ROCKER_LEFT:  LX = map(value_x, 15, 1, 126, 0); break;
    case ROCKER_RIGHT: LX = map(value_x, 1, 15, 128, 255); break;
    case 0: LX = 127; break;
  }
  key_y = arr[1] & HIGH_FOUR;
  value_y = arr[1] & LOW_FOUR;
  switch (key_y) {
    case ROCKER_UP:  LY = map(value_y, 15, 1, 126, 0); break;
    case ROCKER_DOWN: LY = map(value_y, 1, 15, 128, 255); break;
    case 0: LY = 127; break;
  }
  key_r_y = arr[2] & HIGH_FOUR;
  value_r_y = arr[2] & LOW_FOUR;
  switch (key_r_y) {
    case ROCKER_ADVANCE:  RY = map(value_r_y, 15, 1, 126, 0); break;
    case ROCKER_RETREAT: RY = map(value_r_y, 1, 15, 128, 255); break;
    case 0: RY = 127; break;
  }
  
//  key_button = arr[3] & HIGH_FOUR;
//  value_button = arr[3] & LOW_FOUR;
}
