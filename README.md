# Who's Home

A NodeMCU project that tells me at a glance who is home.

## Tools and Components

- Arduino IDE
- NodeMCU v3
- [RicardoOliveira/FriendDetector](https://github.com/RicardoOliveira/FriendDetector)

## Challenges

### Interrogating devices

**Pinging**

Pinging was the first (naive) approach. It turns out that our phones would not always respond to pings so this was quickly scraped.

I wrote a python script to ping network devices and record the frequency of responses. It wasn't feasible.

**nmap**

Each of our devices would respond to `nmap -sN <IP> -p 80`. This would send a TCP packet with sequence 0, and the devices would more often than not respond. This mode of interrogation worked when pinging did not. Turns out it's really hard to make this work so we gave up.

### Broadcast timings vs LED timing

TODO: Write this

## TODO

- Installation
- Schematic
- Pictures
