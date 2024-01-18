Gas Monitoring System - IoT Project with Node-RED , InfluxDB and Telegram-Bot

Overview
This repository contains the necessary files for setting up an IoT project using Node-RED and InfluxDB. The project is designed for real-time data processing and visualization, and includes integration with a Telegram bot for notifications.

Contents
Dockerfile: Sets up the Node-RED environment with additional nodes for InfluxDB and Telegram.
docker-compose.yml: Configuration for Docker Compose to run Node-RED and InfluxDB services.
flows.json: Node-RED flow configurations.
Wokwi folder: Contains the Arduino sketch and related files for the IoT device simulation.
Link to the Wokwi simulation: https://wokwi.com/projects/387138533445295105
Diagrams and libraries used in the project.

Integrations
InfluxDB Integration
Purpose: InfluxDB, a time-series database, is used for storing and managing data related to gas monitoring, including temperature, humidity, gas level, and tank level. This data is crucial for trend analysis and historical tracking.
Data Flow: The system captures real-time data from various sensors and sends it to InfluxDB. This allows for efficient data querying and visualization, supporting both real-time monitoring and historical analysis.
Telegram Bot Integration
Functionality: The Telegram bot, implemented through node-red-contrib-telegrambot, provides a direct communication channel to users.
Alerts and Notifications: It is configured to send alerts and notifications related to the gas monitoring system. For instance, if the gas level exceeds a certain threshold or if there are significant changes in temperature or humidity, the bot sends a timely alert to the configured Telegram channel.
Remote Monitoring: This integration allows users to stay informed about the status of their home environment remotely, enhancing the overall safety and responsiveness of the system.
Gas Monitoring System Data Handling
Sensors: The system includes sensors for temperature, humidity, gas level, and tank level. These sensors continuously monitor the environment and provide vital data for safety and security.
MQTT Protocol: The data from these sensors is transmitted using the MQTT protocol, ensuring reliable and efficient communication.
Node-RED Flows: The flows in Node-RED are designed to process this data, trigger alerts, and store it in InfluxDB. They also facilitate the sending of relevant information to the Telegram bot for user notifications.


Setup Instructions:

Clone the Repository:
git clone https://github.com/adam-bouafia/Gas-Monitoring-System.git
cd Gas-Monitoring-System


Building and Running with Docker:

Ensure Docker and Docker Compose are installed on your system.
Run the following command to build and start the services:

docker-compose up -d


Accessing Node-RED:

Once the containers are up, access Node-RED at http://localhost:1880.
InfluxDB Configuration:

InfluxDB is accessible at http://localhost:8086.
Use the credentials specified in docker-compose.yml for initial login.

Loading Node-RED Flows:
Import flows.json into Node-RED for the pre-configured flows.
Wokwi Simulation:

Visit the provided Wokwi simulation link to view and interact with the IoT device simulation.



Contributing
Contributions to this project are welcome. Please ensure to follow the existing coding standards and submit pull requests for any enhancements.
