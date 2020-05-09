# MINIONS
## MINIONS: MarINe Internet Of Nodes Specification
### Luke Metcalfe UCT Mechatronics Final Year Project 2019

Defined a common electrical, mechanical, operational and communication specification for IoT marine monitoring devices and built two prototypes according to the specification. The system comprised of a marine buoy located at sea (Transmitter Node), a base station on land (Receiver Node), and a nearby server. The buoy was equipped with temperature and turbidity sensors, an ESP32 microcontroller, a power supply and management module, and a wireless communications module. Data was transmitted from the buoy to the base station via LoRaWAN. The base station displayed data on an OLED screen and interacted with the server using the MQTT protocol to upload data to an online user interface. Through the interface, users could view all data graphically, receive status updates, and control the buoy remotely.
