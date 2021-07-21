#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
//模块引脚定义
int CLK = D5;
int CS = D1;
int DIN = D7; //这里定义了那三个脚
int PIECENUM = 2;//数码管片数
int i = 0;

//---------------修改此处""内的信息-----------------------
const char *ssid = "wifi";          //WiFi名
const char *password = "12345678";  //WiFi密码
//-------------------------------------------------------

const unsigned long HTTP_TIMEOUT = 5000;
WiFiClient client;
HTTPClient http;
String response;
int follower = 0;
int favorited = 0;
void setup() {
  // put your setup code here, to run once:
  pinMode(CLK, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(DIN, OUTPUT); //让三个脚都是输出状态
  Init_MAX7219(PIECENUM);
  initdisplay();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
  }

}
bool getJson()
{
    bool r = false;
    http.setTimeout(HTTP_TIMEOUT);
    http.begin("http://修改api文件的URL/api.php");
    int httpCode = http.GET();
    if (httpCode > 0){
        if (httpCode == HTTP_CODE_OK){
            response = http.getString();
            //Serial.println(response);
            r = true;
        }
    }else{
        Serial.printf("[HTTP] GET JSON failed, error: %s\n", http.errorToString(httpCode).c_str());
        errorCode(0x2);
        r = false;
    }
    http.end();
    return r;
}

bool parseJson(String json)
{
    const size_t capacity = JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 70;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, json);
    int code = doc["code"];
    const char *message = doc["message"];
    if (code != 0){
        Serial.print("[API]Code:");
        Serial.print(code);
        Serial.print(" Message:");
        Serial.println(message);
        errorCode(0x3);
        return false;
    }
    JsonObject data = doc["data"];
    unsigned long data_mid = data["mid"];
    int data_follower = data["follower"];
  int data_favorited = data["favorited"];
    if (data_mid == 0){
        Serial.println("[JSON] FORMAT ERROR");
        errorCode(0x4);
        return false;
    }
    follower = data_follower;
  favorited = data_favorited;
    return true;
}

void loop() {
  // put your main code here, to run repeatedly:
  Write_Max7219(1, 0x0f, 0x00, 0);     //显示测试：1；测试结束，正常显示：0
  Write_Max7219(2, 0x0f, 0x00, 0);     //显示测试：1；测试结束，正常显示：0
    if (WiFi.status() == WL_CONNECTED){
        if (getJson()){
            if (parseJson(response)){
                displayNumber(follower);
                displayFavorite(favorited);
            }
        }
    }else{
        Serial.println("[WiFi] Waiting to reconnect...");
        errorCode(0x1);
    }
    delay(3000);//访问时间
}
void displayNumber(int number) //display number in the middle
{
    if (number < 0 || number > 99999999)
        return;
    int x = 1;
    int tmp = number;
    for (x = 1; tmp /= 10; x++);
    for (int i = 1; i < 9; i++)
    {
        if (i < (10 - x) / 2 || i >= (x / 2 + 5)){
            Write_Max7219(1, i, 0xf, 0);
        }else{
            int character = number % 10;
            Write_Max7219(1, i, character, 0);
            number /= 10;
        }
    }
}
void displayFavorite(int number) //display number in the middle
{
    if (number < 0 || number > 99999999)
        return;
    int x = 1;
    int tmp = number;
    for (x = 1; tmp /= 10; x++);
    for (int i = 1; i < 9; i++)
    {
        if (i < (10 - x) / 2 || i >= (x / 2 + 5)){
            Write_Max7219(2, i, 0xf, 0);
        }else{
            int character = number % 10;
            Write_Max7219(2, i, character, 0);
            number /= 10;
        }
    }
}

void Delay_xms(unsigned int x)
{
  unsigned int i, j;
  for (i = 0; i < x; i++)
    for (j = 0; j < 112; j++);
}

//切换地址，方便写
void Write_Mynum(int pnum,  unsigned char address, unsigned char dat , int dp) {
  Write_Max7219(pnum, 9 - address, dat, dp);
}

//--------------------------------------------
//功能：向MAX7219写入字节
//入口参数：DATA,dp显示小数点与否
void Write_Max7219_byte(unsigned char DATA, int dp)
{
  unsigned char i;
  digitalWrite(CS, LOW);
  for (i = 8; i >= 1; i--)
  {
    digitalWrite(CLK, LOW);
    if (i == 8 && dp == 1)
      digitalWrite(DIN, HIGH);
    else {
      if (DATA & 0X80)
        digitalWrite(DIN, HIGH);
      else
        digitalWrite(DIN, LOW);
    }
    DATA <<= 1;
    digitalWrite(CLK, HIGH);
  }
}

//-------------------------------------------
//功能：向MAX7219写入数据
//入口参数：pnum数码管片序号,address,dat,dp显示小数点与否
void Write_Max7219(int pnum, unsigned char address, unsigned char dat, int dp)
{
  digitalWrite(CS, LOW);
  Write_Max7219_byte(address, 0);          //写入地址，即数码管编号
  Write_Max7219_byte(dat, dp);              //写入数据，即数码管显示数字
  if (pnum > 1) {
    digitalWrite(CLK, HIGH);
    for (int i = 1; i < pnum; i++) {
      Write_Max7219_byte(0X00, 0);
      Write_Max7219_byte(0X00, 0);
    }
  }
  digitalWrite(CS, HIGH);
}


void initdisplay()
{
    //向两块数码管发送横线表示初始化
    Write_Max7219(1, 0x01, 0x0a, 0);
    Write_Max7219(1, 0x02, 0x0a, 0);
    Write_Max7219(1, 0x03, 0x0a, 0);
    Write_Max7219(1, 0x04, 0x0a, 0);
    Write_Max7219(1, 0x05, 0x0a, 0);
    Write_Max7219(1, 0x06, 0x0a, 0);
    Write_Max7219(1, 0x07, 0x0a, 0);
    Write_Max7219(1, 0x08, 0x0a, 0);

    Write_Max7219(2, 0x01, 0x0a, 0);
    Write_Max7219(2, 0x02, 0x0a, 0);
    Write_Max7219(2, 0x03, 0x0a, 0);
    Write_Max7219(2, 0x04, 0x0a, 0);
    Write_Max7219(2, 0x05, 0x0a, 0);
    Write_Max7219(2, 0x06, 0x0a, 0);
    Write_Max7219(2, 0x07, 0x0a, 0);
    Write_Max7219(2, 0x08, 0x0a, 0);
}
//增加错误代码显示
// E1--WiFi连接中断, 重连中.
// E2--HTTP请求错误, 当前网络不通畅或请求被拒.
// E3--API返回结果错误, 请检查输入的UID是否正确.
// E4--JSON格式错误, 请检查当前网络是否需要认证登陆.
void errorCode(byte errorcode)
{

    Write_Max7219(1, 0x01, 0x0a, 0);
    Write_Max7219(1, 0x02, 0x0a, 0);
    Write_Max7219(1, 0x03, 0x0a, 0);
    Write_Max7219(1, 0x04, errorcode, 0);
    Write_Max7219(1, 0x05, 0x0b, 0);
    Write_Max7219(1, 0x06, 0x0a, 0);
    Write_Max7219(1, 0x07, 0x0a, 0);
    Write_Max7219(1, 0x08, 0x0a, 0);

    Write_Max7219(2, 0x01, 0x0a, 0);
    Write_Max7219(2, 0x02, 0x0a, 0);
    Write_Max7219(2, 0x03, 0x0a, 0);
    Write_Max7219(2, 0x04, errorcode, 0);
    Write_Max7219(2, 0x05, 0x0b, 0);
    Write_Max7219(2, 0x06, 0x0a, 0);
    Write_Max7219(2, 0x07, 0x0a, 0);
    Write_Max7219(2, 0x08, 0x0a, 0);
}

//Max7219初始化
void Init_MAX7219(int pienum)
{
    Write_Max7219(1, 0x09, 0xff, 0);     //译码方式：BCD码
    Write_Max7219(1, 0x0a, 0x09, 0);     //参数3：亮度
    Write_Max7219(1, 0x0b, 0x07, 0);     //扫描界限；参数3：8个数码管显示
    Write_Max7219(1, 0x0c, 0x01, 0);     //掉电模式：参数3：0，普通模式：1
    Write_Max7219(1, 0x0f, 0x00, 0);     //显示测试：参数3：1；测试结束，正常显示：0

    Write_Max7219(2, 0x09, 0xff, 0);     //译码方式：BCD码
    Write_Max7219(2, 0x0a, 0x0f, 0);     //参数3：亮度
    Write_Max7219(2, 0x0b, 0x07, 0);     //扫描界限；参数3：8个数码管显示
    Write_Max7219(2, 0x0c, 0x01, 0);     //掉电模式：参数3：0，普通模式：1
    Write_Max7219(2, 0x0f, 0x00, 0);     //显示测试：参数3：1；测试结束，正常显示：0
} 
