# DaikinEthernet

Daikin remote control for arduino with ethershield and include DHT11 sensor.

Daikin 遠端控制使用Arduino配合ethershield完成網路控制Daikin

## Circuit
 ![](http://dylab.mediweb.35g.tw:30000/DY-Open-Source-Project/DaikinEthernet/raw/master/arduino_remote_daikin_ethernetshield_ciruit.png)

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

find

```
IPAddress ip(192, 168, 9, 59);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
```

replace

```
IPAddress ip(192, 168, 9, 59);
IPAddress gateway(192, 168, 9, 1);
IPAddress networkmask(255, 255, 255, 0);
```

find

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


## Command

### format

http://your ip/action[get|set][number][value]

get is `?`,set is `=` and number is machine number ,`00` is fixed, value is group of two digital ascii number

### example

http://192.168.9.59/ft?

check your arduino feature and response like:

```
{"status":"ok","version":"1","ac":"1,"humidity":"1","temperature":"1"}
```

### command

#### getter
* ft?00 get feature

```
{"status":"ok","version":"1","ac":"1,"humidity":"1","temperature":"1"}
```

* hm?00 get humidity

```
{"status":"ok","number":"0","humidity":"32.00"}
```

* tp?00 get temperature

```
{"status":"ok","number":"0","temperature":"25.00"}
```

* ac?00 get air condition state

```
{"status":"ok","number":"0","power":"0","swing":"0","mode":"0","speed":"0","temperature":"25"}
```

#### setter

* ac=00[power][swing][mode][speed][temperature]

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