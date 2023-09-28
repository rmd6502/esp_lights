# esp_lights
Lights driven by ESP8266 and MQTT

# Setup
When first started, the lights will act as an access point, e.g. 'esp8266-lights-livingroom' The password will be whatever you set up in config.h

Select the SSID you want to connect to, enter the password and the IP of the MQTT server, and you should be good to go. You may need to manually reset or power cycle.

# Message
The message sent to the lights is to topic _/lightname/color_ and is a JSON payload, with retention set
_{ color: '#RRGGBB', brightness: 0-1 }_ . You can also leave the brightness at 1 and just change it with the color.

I believe there's also a React webapp included in this repo that will let you change the color of lights from a simple web app.
