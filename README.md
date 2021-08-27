## Two sound modules that interact with each other wirelessly; like a duet   
![](https://github.com/xoskeletor/ESP8266_DFPlayer/blob/main/ESP_DFPlay_Breadboard.png?raw=true)  
* An ESP8266 WeMos D1 Mini controls the DFPlayer Mini via serial  
* The modules are connected via WiFi   
* Module A is access point and station  
* Module B is station  
* At initialization, i.e. at power/on or ESP reset, module plays MP3 Track 1  
* Push-button input triggers MP3 Track 2, then send signal to other module  
* When the other module receives signal, it plays MP3 Track 3  
* Optional LED at D7 as WiFi ready indicator
## Micro-SD card  
![](https://github.com/xoskeletor/ESP8266_DFPlayer/blob/3757b1a80db6d880d37c80d6c4838956a24cbe24/DFplay_microSD.png)  
* Tracks must be placed in top directory named "mp3"  
* Tracks must be named with numbers with leading zeros e.g. "001_myFavoriteSong.mp3"  
