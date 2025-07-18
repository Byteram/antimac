# antimac
(Millennium Hawk)

![star-wars-hyperspace](https://github.com/user-attachments/assets/bcf0da5f-234c-49ba-9981-852345f1f9fb)

> “She may not look like much, but she’s got it where it counts, kid.”  
> — Han Solo, Star Wars: Episode IV – A New Hope (1977)

---

**antimac** is a fast, gallant MAC address spoofing tool for macOS, written in C.  
Inspired by the Millennium Falcon: scrappy, reliable, and always ready to jump to lightspeed.

## Features

-  Instantly generate and set a random MAC address for any interface
-  Show the current MAC address of your device
-  Set a specific MAC address of your choosing
-  macOS native (arm64 & Intel)
-  Requires root for MAC changes (sudo)

## Usage

```sh
# Set a random MAC address
sudo ./antimac <device>

# Show the current MAC address
./antimac -s <device>
./antimac --show <device>

# Set a specific MAC address
sudo ./antimac -c <device> <new-mac-address>
sudo ./antimac --config <device> <new-mac-address>

# Show version
./antimac -v
./antimac --version
```

## Disambiguation

- **anti**: instead, in the place of
- **mac**: MAC address (media access control address) — a unique identifier assigned to a network interface controller (NIC) for use as a network address in communications within a network segment.

## Example

```sh
sudo ./antimac en0
./antimac -s en0
sudo ./antimac -c en0 00:11:22:33:44:55
```

## Credits

- Authors: [@vvrmatos](https://github.com/vvrmatos) and [@spacemany2k38](https://github.com/spacemany2k38)
- Millennium Hawk: “Special modifications” included.

---

> “Never tell me the odds.”  
> — Han Solo
