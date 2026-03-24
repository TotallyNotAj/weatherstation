# Internet of Things (IoT) Template - Home Automation and Analysis

### Description
The Hardware component has two tasks: (1) Implement a remote authentication sub-system as part of a Smart Home Automation System, for the main entrance of a Smart Home. This enables homeowners to grant temporary access to their home by providing their guest with a 4-digit passcode, which can subsequently be changed remotely after the guest leaves. (2) Implement a real-time water management sub-system for the Home Automation System, to monitor the water level of a 1000 US gallon storage tank located on the compound. Additionally, the hardware component incorporates a weather station that reads and processes data from multiple sensors — including a DHT22 for air temperature and humidity, a BMP280 for air pressure and altitude, and a capacitive sensor for soil moisture — displaying readings locally on a TFT screen and publishing sensor data every second as a structured JSON payload to an MQTT broker over WiFi.

The Backend component also has two tasks: (1) Store data sent via HTTP POST request from the Arduino Nano in a database that adheres to a specified schema, including fields for temperature, humidity, heat index, pressure, altitude, soil moisture, and a Unix timestamp. (2) Facilitate access to stored data by both the hardware and frontend components, accomplished through the 
implementation of API routes accessible by the frontend and HTTP GET requests from the ESP32 hardware component.

The Frontend component is tasked with: (1) Providing a User Interface (UI), on a webpage, for remotely setting and updating the 4-digit passcode used by the remote authentication sub-system to control access at the main entrance. (2) Providing a User Interface (UI), on a webpage, featuring charts and graphs to display the real-time status of the Water Management System. (3) Providing an additional webpage that integrates visual representations, such as charts and graphs, extracting trends and illustrating correlations among meteorological variables from data stored in the database.


# Hardware Setup
Download and install [Arduino](https://www.arduino.cc/en/software) IDE. Subsequently, install the following Arduino IDE libraries following the tutorial [here](https://support.arduino.cc/hc/en-us/articles/5145457742236-Add-libraries-to-Arduino-IDE):
1. Adafruit GFX Library by Adafruit
2. Adafruit ILI9341 by Adafruit
3. ArduinoJson by Benoît Blanchon
4. PubSubClient by Nick O’Leary
5. NewPing by Tim Eckel
6. esp32 by Espressif Systems, from the Board Manager tab in the Arduino IDE



# Backend Setup
Always ensure to establish a virtual environment and install the necessary packages from your requirements file if you haven't already done so. Following that, activate your virtual environment and proceed to run your Flask API.

**The commands below must be executed from a command line terminal in the homeautomation/backend/ folder**
### Create a virtual environment

Windows 
```sh
python -m venv env  
```
Linux
```sh
python3 -m venv env  
```
### Activate virtual environment
Windows
```sh
.\env\Scripts\activate 
```
Linux
```sh
source env/bin/activate
```
### Install API requirements in the virtual environment
```sh
pip install -r requirements.txt 
```
### Create **.env** file
Create a **.env** file in the backend/ folder to store the application's environment variables. 
Refer to the lab manual for the specific information that must be added to this file. Subsequently, modify the **FLASK_RUN_HOST** variable found in this file from localhost to the IP address of the computer your backend is running on.

### Start Flask API
Windows
```sh
py run.py 
```
Linux
```sh
python3 run.py
```



# Frontend Setup ( [Vue js](https://vuejs.org/), [Vuetify](https://vuetifyjs.com/en/components/all/), [Vite](https://vitejs.dev/))
### Recommended IDE Setup
[VSCode](https://code.visualstudio.com/) + [Volar](https://marketplace.visualstudio.com/items?itemName=johnsoncodehk.volar) (and disable Vetur) + [TypeScript Vue Plugin (Volar)](https://marketplace.visualstudio.com/items?itemName=johnsoncodehk.vscode-typescript-vue-plugin).

Customize configuration
See [Vite Configuration Reference](https://vitejs.dev/config/).


### In a command line terminal, execute the first commands in the homeautomation/frontend/ folder to initiate the dev server for the initial setup. 
### For all subsequent instances, only run the second command to start the dev server.
### Once development is complete, run the final command to generate production files. Please be aware that the generation of production files is not part of this course.

Project Setup
```sh
npm install
```

Run dev server (Compile and Hot-Reload for Development)
```sh
npm run dev
```

Create a production bundle (Compile and Minify for Production)
```sh
npm run build
```


### Modify **vite.config.js** file
Modify the target in the proxy object found in the **frontend/vite.config.js** file shown in the code block below. Change localhost in the string to the IP address of the computer your backend is running on.
```js
proxy: {
      '^/api*': { 
        target: 'http://localhost:8080/' ,
       changeOrigin: false,
    },   
  }

```