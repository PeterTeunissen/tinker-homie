Project starts an ESP8266 which starts a Wifi Access point called LoraSwitch-<mac-2> 
You can connect to it with a phone and enter password: password-123
Then open a browser to 192.168.4.1 and it will show the internal state of the switch.

Port D7 of the ESP8266 controls a relay module which is wired to turn on/off an extension cord.

Port D5,D6 are connected to a RYLR-980 module that received messages from a host. If the received 
message contains ON it turns the relay on. If the message contains OFF ot turns the relay off.
Any other received message is ignored.

Since LORA is not a guaranteed delivery message protocol, the sender should just periodically send the
current state.
