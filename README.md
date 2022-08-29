# ESP32 DS18B20 Prometheus Exporter

This project is designed to use an ESP32 as DS18B20 temperature sensor aggregator, and expose a /metrics HTTP endpoint for simple metric collection.


- Use an ESP32
- Serial Speed: 115200

- Use Visual Studio Code with the PlatformIO Addon
- Connect the sensors to the data pin (PIN 14)
    - You may find it easier to use some Dupont cables -> Breadboard -> Terminal Block, and then have x sensors all connect into your terminal block
    - Alternatively, Wagos also work
- Supports up to MAX_SENSORS

- During writing, some ESP boards (ESP32 DEVKITV1) require holding down the 'BOOT' button before and during pressing "Upload"
- Join the ESP32 Wifi Network using your phone
- Join it to your WiFi

- Start the device and use WiFiManager to join the ESP to your WiFi
- Add the node to be scraped by your prometheus-compatible scraper
    - You may need to use your Router UI to discover the IP your device has been given and to make the DHCP allocation static
- http://SomeIPAddress:8080/metrics