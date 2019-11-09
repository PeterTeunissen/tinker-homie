# The most powerful Timing node for Node-Red

#Purpose
BigTimer is a powerful timing node offering a range of timing facilities for Node-Red and can be used singly or in groups. Full support for dusk/sunset dawn/sunrise and variations also day/week/month control and the node offers outputs to MQTT, speech and databases. You can also manually over-ride the UTC time setting on the host computer if required.

#Usage
Suitable for general use, BigTimer has 3 outputs, one of which triggers when there is a change and presents one of two messages (for, for example MQTT or other control mechanism), the second is a simple 1 or 0 every minute in the payload but also has additional outputs reflecting the status message in msg.state and message time. There is also a msg.name available  - and the third outputs a message which could be used for speech or debugging.

#Inputs
It also has an input. This can be used to override the schedule - valid commands in the payload are "on" (or 1), "off" (or 0), "manual" -  For a temporary hold (until next powerup), "default" (or "auto") which scraps manual settings, "stop" stops the timer working completely, without the affecting current outputs and "sync" outputs the current state immediately without changing anything.

#Override
The manual override will time-out after 24 hours (you can alter this in minutes) and go back to auto (just in case you forget to turn something off). Manual override will also be scrapped by any scheduled change. There is a pause schedule button for convenient longer-term pausing of the schedule.

#Special Days
Recent additions include special days (i.e. 25/12) and special weekdays (i.e. first Tuesday of the month).


#General
Note - if upgrading to a later version of BigTimer - check your settings. More information on BigTimer and a range of home-control-related projects can be found at http://tech.scargill.net



