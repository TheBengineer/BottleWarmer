# Bottle Warmer

## A Crockpot hacked to keep bottles warm

This project started one night when I was trying to warm up
some water, so I could make formula, while holding a crying baby.

"Things would be so much easier if the bottles were already up to temperature" I thought.

And then I was buying parts one-handed on Amazon while feeding with the other.

## Project Structure
### [Arduino Sketch](BottleWarmer/BottleWarmer.ino)
### [Python for monitoring and logo generation](BottleLogo)

## Features

* Accurate Temperature control ±1 degree Fahrenheit
* Programmable Heat cycle to periodically sterilize the water
* Wifi enabled to allow time sync over NTP
    * Also allows for remote monitoring and over the air updates
    * TODO - Add control over http
* OLED Screen to display current temperature
* Rotary encoder to set temperature and cycle time
* EEPROM to maintain settings across power cycles

## Screenshots

### Boot screen with Wi-Fi info
<img src="https://github.com/TheBengineer/BottleWarmer/blob/main/github/images/Splash.jpg?raw=true"  width="70%"></img>
### Main screen shows current temperature and settings
<img src="https://github.com/TheBengineer/BottleWarmer/blob/main/github/images/interface.jpg?raw=true"  width="70%"></img>
### Configuration screen
<img src="https://github.com/TheBengineer/BottleWarmer/blob/main/github/images/settings.jpg?raw=true"  width="70%"></img>
### Over the air update progress screen
<img src="https://github.com/TheBengineer/BottleWarmer/blob/main/github/images/OTA%20update.jpg?raw=true"  width="70%"></img>


## Parts List
1) [Crockpot](https://www.amazon.com/Crock-Pot-SCV700SS-Stainless-7-Quart-Manual/dp/B003OAJGJO)
2) [esp8266](https://www.amazon.com/HiLetgo-Internet-Development-Wireless-Micropython/dp/B081CSJV2V)
3) [Solid State Relay](https://www.amazon.com/gp/product/B079BGBCF4)
8) [I2C Oled Screen](https://www.amazon.com/dp/B09C5K91H7)
4) [Temperature Probes](https://www.amazon.com/gp/product/B09NVFJYPS)
5) [Encoder With Button](https://www.amazon.com/dp/B07T3672VK)
5) [USB Charger](https://www.amazon.com/Charger-iPhone-Travel-Adapter-Samsung/dp/B0995MV494)
6) [Breadboard](https://www.amazon.com/Gikfun-Solder-able-Breadboard-Plated-Arduino/dp/B071R3BFNL)
7) [Jumper Wires](https://www.amazon.com/EDGELEC-Breadboard-Optional-Assorted-Multicolored/dp/B07GD2BWPY)

All in all, this project cost me about $100, and once it's no longer needed
as a bottle warmer, I'll have an accurate
temperature controlled crockpot that functions as a sois vide.








