# DouYin_FansCounter
抖音粉丝数、点赞数计数器
原理跟B站的粉丝计数器一致，在单一显示粉丝数的基础上增加点赞数显示。
也就是用一块esp8266控制级联的MAX7219数码管

#### 运行环境
php+redis

#### 需要硬件：
ESP8266 Nodemcu  +  MAX7219八位数码管

#### 使用方法：
打开api.php修改里面的cookies，cookies目前是从抖音创作平台取出来的。
将api.ph传到web服务器，需要搭配redis。
用Arduino打开项目文件，修改wifi信息和API.PHP的URL链接编译即可。
