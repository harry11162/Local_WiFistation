#define INTERVAL_SENSOR   17000             //定义传感器采样时间间隔  597000
#define INTERVAL_NET      17000             //定义发送时间
//传感器部分================================   
#include <Wire.h>                                  //调用库  
#include "ESP8266.h"
#include "I2Cdev.h"                                //调用库  
//温湿度   
#include <SHT2x.h>
//光照
#define  sensorPin_1  A0

#define SSID           "wzy_bupt"                   // cannot be longer than 32 characters!
#define PASSWORD       "12345678"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

//WEBSITE     
#define HOST_NAME    "192.168.43.60"             //可改成自己的服务器地址和端口
#define HOST_PORT   (3005)

//3,传感器值的设置 
float sensor_tem, sensor_hum, sensor_lux;                    //传感器温度、湿度、光照   
char  sensor_tem_c[7], sensor_hum_c[7], sensor_lux_c[7] ;    //换成char数组传输
ESP8266 wifi(Serial1);                                      //定义一个ESP8266（wifi）的对象
unsigned long net_time1 = millis();                          //数据上传服务器时间
unsigned long sensor_time = millis();                        //传感器采样时间计时器

//int SensorData;                                   //用于存储传感器数据
String postString;                                //用于存储发送数据的字符串
//String jsonToSend;                                //用于存储发送的json格式参数

void setup(void)     //初始化函数  
{       
  //初始化串口波特率  
    Wire.begin();
    Serial.begin(115200);   
    while(!Serial);
    pinMode(sensorPin_1, INPUT);

   //ESP8266初始化
    Serial.print("setup begin\r\n");   

  Serial.print("FW Version:");
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStationSoftAP()) {
    Serial.print("to station + softap ok\r\n");
  } else {
    Serial.print("to station + softap err\r\n");
  }

  if (wifi.joinAP(SSID, PASSWORD)) {      //加入无线网
    Serial.print("Join AP success\r\n");  
    Serial.print("IP: ");
    Serial.println(wifi.getLocalIP().c_str());
  } else {
    Serial.print("Join AP failure\r\n");
  }

  if (wifi.disableMUX()) {
    Serial.print("single ok\r\n");
  } else {
    Serial.print("single err\r\n");
  }

  Serial.print("setup end\r\n");
    
  
}
void loop(void)     //循环函数  
{   
  if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR)              //传感器采样时间间隔  
  {  
    getSensorData();                                        //读串口中的传感器数据
    sensor_time = millis();
  }  
    delay(1000); 
    
  if (net_time1 > millis())  net_time1 = millis();
  
  if (millis() - net_time1 > INTERVAL_NET)                  //发送数据时间间隔
  {                
    updateSensorData();                                     //将数据上传到服务器的函数
    net_time1 = millis();
  }
  delay(1000);
}

void getSensorData(){  
    sensor_tem = SHT2x.GetTemperature() ;   
    sensor_hum = SHT2x.GetHumidity();   
    //获取光照
    sensor_lux = analogRead(A0);    
    delay(1000);
    dtostrf(sensor_tem, 2, 1, sensor_tem_c);
    dtostrf(sensor_hum, 2, 1, sensor_hum_c);
    dtostrf(sensor_lux, 3, 1, sensor_lux_c);
}
void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");
 

//postString将存储传输请求，格式很重要
postString = "POST ";               //post发送方式，后要有空格
  postString += "/process?tem=";    //接口process
  postString += sensor_tem_c;
  postString += "&hum=";             //要发送的数据
  postString += sensor_hum_c;
  postString += "&light=";            
  postString += sensor_lux_c;
  postString += " HTTP/1.1";          //空格+传输协议
  postString += "\r\n";
  postString += "Host: ";            //Host：+空格
  postString += HOST_NAME;
  postString += "\r\n";
  postString += "Content-Type: application/x-www-form-urlencoded\r\n";  //编码类型
  postString += "Connection: close\r\n";
  postString += "\r\n";               //不可删除

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println("send success");   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print("create tcp err\r\n");
  }
  
}
