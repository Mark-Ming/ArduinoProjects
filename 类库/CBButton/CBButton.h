
class CBButton
{

  public:
    CBButton(int pin);
    bool keyDown();


  private:
    int pinNum;
    bool btn_pressed;

    unsigned long btnDownStartTime;


};





