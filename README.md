#LED Candles with accuflicker
Uses a NodeMCU with control via MQTT to drive 4 LED circuits chock full of realistic flame flickering action
  
### Resources:
    - .ledhacker's blog         - https://ledhacker.blogspot.com/2011/12/led-candle-light-flicker-with-arduino.html
    - Christian Haschek's blog  - https://blog.haschek.at/2019/diy-garden-irrigation-for-less-than-20-bucks.html
 
### Notes:
    - Set your wifi and mqqt details in Credentials.h (Credentials.template.h provided)
    - Change the pin assignments in lin 28-31 as needed

    - To turn on, send a `candlecontrol/candle` topic with a payload of ON
    - To turn off, use a payload of OFF. ;)
    - The board will send a `status/candlecontrol` topic with a payload of ONLINE or OFFLINE
