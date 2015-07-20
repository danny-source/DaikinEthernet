# DaikinEthernet

Daikin ARC remote control for arduino with ethershield and include DHT11 sensor.

Daikin ARC 遠端控制使用Arduino配合ethershield完成網路控制Daikin

## Circuit
 ![](http://dylab.mediweb.35g.tw:30000/DY-Open-Source-Project/DaikinEthernet/raw/master/arduino_remote_daikin_ethernetshield_ciruit.png)

## Picture

 ![](http://dylab.mediweb.35g.tw:30000/DY-Open-Source-Project/DaikinEthernet/raw/master/daikinethernet-picture.jpg)

## Prepare

first,you need to know [how to install arduino library](https://www.arduino.cc/en/guide/libraries) and finish this librarys:

首先，你必需要學會如何[自行安裝函數庫](http://cms.35g.tw/coding/arduino-remote-daikin/)，完成下列的安裝：

* [DHT Sensor](https://github.com/adafruit/DHT-sensor-library)

* [Arduino_IRremote_Daikin](https://github.com/danny-source/Arduino_IRremote_Daikin)


## Download and Run

Download DaikinEthernet source code and unzip it.next, you can open the `DaikinEthernet.ino` .

下載 DaikinEthernet 源始碼並且解壓後開啟它

## Modify Network IP

modify the ip in your network enverment. make sure your network can be used.

根據你的網路環境來修改你的IP，並且確認你的網路環境是可運作的。

 ![](http://dylab.mediweb.35g.tw:30000/DY-Open-Source-Project/DaikinEthernet/raw/master/daikinethernet-1.png)

if your network need to connect internet,you must fix `Network Mask`、`Gateway` setting

如果你要連外網路的話必需要填入`Network Mask`、`Gateway`

find 尋找

```
IPAddress ip(192, 168, 9, 59);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
```

replace 覆蓋

```
IPAddress ip(192, 168, 9, 59);
IPAddress gateway(192, 168, 9, 1);
IPAddress networkmask(255, 255, 255, 0);
```

find 尋找

```
  //ethernet shield init
  Ethernet.begin(mac, ip);
  Serial.println(Ethernet.localIP());
  server.begin();
```

```
  //ethernet shield init
  Ethernet.begin(mac, ip, gateway, networkmask);
  Serial.println(Ethernet.localIP());
  server.begin();
```

## Upload to Arduino

connect arduino to pc by usb cable. click upload button to flash code to arduino.

利用USB將你的Arduino連接並且按下上傳鍵將程式上傳至Arduino


## Command/Format 命令與格式

### format 格式

http://your ip/action[get|set][number][value]
http://你的位址/動作[取|存][號碼][值]

#### action 動作

action is device like sensor,daikin ac...etc.

#### get/set 取//存

get is `?`,set is `=`

取值為問號`?`，存值為`=`

#### number 號碼

number can support same type of devices,but it is reserve and value is `00`.

號碼為了將來能支援同類型的裝置而準備，目前功能未完成，預設值以`00`

#### value 值

value is two digital ascii number a group.ex: 1 is `01`.

值為兩位數的字串型數值，像數值1則以`01`表示

### example 範例

http://192.168.9.59/ft?

check your arduino feature and response like:

檢查你的arduino支援功能的請求，得到的結果如下：

```
{"status":"ok","version":"1","ac":"1,"humidity":"1","temperature":"1"}
```

### command 命令

number is reserve and value is `00`

#### getter 取值命令
* ft?00

  get feature 取得支援功能

```
{"status":"ok","version":"1","ac":"1,"humidity":"1","temperature":"1"}
```

* hm?00

  get humidity 取得溼度

```
{"status":"ok","number":"0","humidity":"32.00"}
```

* tp?00

  get temperature 取得溫度

```
{"status":"ok","number":"0","temperature":"25.00"}
```

* ac?00

  get air condition state 取得空調的狀態


```
{"status":"ok","number":"0","power":"0","swing":"0","mode":"0","speed":"0","temperature":"25"}
```

#### setter 存值命令

* ac=[00][power][swing][mode][speed][temperature]

  set air condition state 設定空調的狀態

   power 電源

   0: off
   1: on

  swing 擺動

  0:off
  1:on

  mode 模式

  0:FAN
  1:COOL
  2:DRY

  speed 速度

  0:speed 1
  1:speed 2
  2:speed 3
  3:speed 4
  4:speed 5
  5:Auto
  6:moon

  temperature 溫度

  18 ~ 33 degree.

##### example
```
 curl http://192.168.9.59/ac=000000010625
```

response

```
{"status":"ok"}
```

if it have any error ,response:

```
{"status":"command error"}
```