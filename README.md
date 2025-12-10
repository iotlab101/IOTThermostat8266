io7 IOT Thermostat Device
---

This is an io7 IoT Thermostat and a part of io7 IoT Platform sample project.

<img width="923" height="405" alt="Screenshot 2025-12-10 at 5 17 21 PM" src="https://github.com/user-attachments/assets/c87441ca-55bf-43d5-9eb7-016c21a3eedb" />

* It reports its status every predefined interval to the io7 IoT Platform, ie. valve on/off status.
* It executes the command when recevied from the io7 IoT Platform, ie valve on/off command.

It can work with the io7 IoT Thermostat https://github.com/iotlab101/IOTValve8266.
1. Have an io7 IoT Platform running
2. Register a device ID, ex) `thermostat1`
3. Register an AppID, ex) `app1`
4. Assemble an ESP8266 as above diagram
5. Compile and Upload this repository to an ESP8266
6. Configure the ESP8266 with the io7 device ID and token.
7. Copy the content of the thermostat flow to the clipboard
8. Import the flow by pasting to the Import window of the NodeRED
9. Install NodeRED Dashboard @flowfuse/node-red-dashboard
10. Finish the NodeRED flow setting with AppID for io7 in/out node

---
### io7 IoT Platform can be installed by following these youtubes
* https://www.youtube.com/watch?v=ERqRcgKMdoE
* https://www.youtube.com/watch?v=18xfq__oo4E
