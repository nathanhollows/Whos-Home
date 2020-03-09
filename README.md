# Who's Home

A NodeMCU project that tells me at a glance who is at home. The microprocessor pings devices on the network (set to have static IPs) and toggles a corresponding LED

## Tools and Components

So far:

- Arduino IDE
- NodeMCU v3
- Python3

## Challenges

Android phones don't always keep wifi connected, and in this case, a phone might appear to be disconnected from the network. Possible solutions to this are:

- Have the phones ping the controller (requires an app running as a background service)
- Ping more often, set a failure threshold before determining the device is offline
- Give up

`/tests/log.py` is currently being used to test the network status every 5 minutes. This may need to be made more or less frequent depending on the results.

## TODO

- Finish the project
- Installation Segment
- Schematics Segment
- Pictures
