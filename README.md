# Who's Home

A NodeMCU project that tells me at a glance who is at home. The microprocessor pings devices on the network (set to have static IPs) and toggles a corresponding LED

## Tools and Components

So far:

- Arduino IDE
- NodeMCU v3
- Python3 (for testing)

## Challenges

Our phones don't always respond to pings. They do, however, response to `nmap -sN <IP> -p 80`, which, as I understand, sends a TCP request with a sequence 0 that the phone then responds to. This mode of interrogation works when ping does not.

`/tests/log.py` was used to ping the devices every 5 minutes and log the results

## TODO

- Installation Segment
- Schematics Segment
- Pictures
- Test nmap, looking for consistent results
- Switch from pinging to TCP requests on controller
