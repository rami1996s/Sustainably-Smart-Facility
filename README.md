# Sustainably Smart Facility
implemented with Hussein Mohammad in 2024

An IoT-powered, cloud-integrated environmental monitoring and control system designed for sustainability, automation, and real-time insights. This project utilizes ESP32 devices and AWS cloud services for efficient sensor data management, actuation, and data-driven decision-making.

---

##  Overview

As part of the Distributed Systems and Cloud/Edge Computing curriculum, this project demonstrates a fully integrated smart facility using:

- Real-time environmental monitoring (temperature, humidity, gas)
- Automated actuation (fan and window control)
- Cloud data storage and processing
- Notifications and reporting

---

##  Hardware Components

| Component       | Purpose                                |
|----------------|----------------------------------------|
| ESP32           | Sensor node and actuator controller    |
| DHT11 Sensor    | Measures temperature and humidity      |
| MQ-2 Sensor     | Detects gas/smoke                      |
| Fan             | Ventilates when gas exceeds threshold  |
| Servo Motor     | Controls window ventilation            |

---

## ☁️ AWS Services Used

| Service         | Function                                                    |
|----------------|-------------------------------------------------------------|
| **IoT Core**    | MQTT broker between ESP32 and the cloud                    |
| **Lambda**      | Logic processing, command issuing, and data routing        |
| **DynamoDB**    | Real-time structured storage of sensor data                |
| **S3**          | Archives raw JSON sensor data                              |
| **SNS**         | Sends notifications (e.g., actuator triggered)             |
| **CloudWatch**  | Logs Lambda execution and system performance               |
| **Glue**        | Crawls S3 bucket and prepares schema metadata              |
| **Athena**      | Runs SQL-like queries on sensor data in S3                 |

---

##  System Workflow

1. **Data Collection**
   - ESP32 devices collect environmental data
   - Data is sent to **AWS IoT Core** via MQTT

2. **Data Processing**
   - **AWS Lambda**:
     - Stores data in **DynamoDB**
     - Archives data in **Amazon S3**
     - Triggers **SNS** email alerts if thresholds are exceeded

3. **Data Analytics**
   - **AWS Glue** creates a catalog of archived data in S3
   - **Athena** runs queries to extract insights like daily min/max/avg values

4. **Monitoring**
   - **CloudWatch** tracks all function activities for debugging and auditing

---

##  Folder Contents

```plaintext
Arduino_Code/         # ESP32 sketch to publish sensor data
AWS_Lambda/           # Python code for AWS Lambda function
S3_Sample_Data/       # Sample archived sensor data (JSON)
Glue_Athena/          # Athena SQL queries
presentation/         # Project slides (.pptx)
README.md             # Project documentation
.gitignore            # Untracked/ignored files
