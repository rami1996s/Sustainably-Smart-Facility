import json
import boto3
import time
from decimal import Decimal
from datetime import datetime

# Initialize AWS clients
sns = boto3.client('sns')
dynamodb = boto3.resource('dynamodb')

# Define SNS Topic ARN for notifications
SNS_TOPIC_ARN = 'arn:aws:sns:us-east-1:442426867071:RaspberryPiNotification'

# Define threshold values
TEMPERATURE_THRESHOLD = 24.0
GAS_VALUE_THRESHOLD = 1900
HUMIDITY_THRESHOLD = 60.0

# Notification interval (in seconds)
NOTIFICATION_INTERVAL = 60 * 60  # 1 hour

# DynamoDB table name
TABLE_NAME = 'Sensor_data'

# Track last notification time
last_notification_time = 0

def lambda_handler(event, context):
    global last_notification_time
    
    # Initialize DynamoDB table resource
    table = dynamodb.Table(TABLE_NAME)

    # Check if the message is wrapped in the "message" field
    message_data = event.get('message', event)  # Use event['message'] if it exists, otherwise event itself

    # Extract necessary fields from IoT event
    device_id = message_data.get('device_id')
    timestamp = str(message_data.get('timestamp', int(datetime.now().timestamp())))
    temperature = message_data.get('temperature', 0.0)
    gas_value = message_data.get('gas_value', 0)
    humidity = message_data.get('humidity', 0.0)
    
    print(f"Received data - DeviceId: {device_id}, Timestamp: {timestamp}, Temperature: {temperature}, GasValue: {gas_value}, Humidity: {humidity}")

    # Save data to DynamoDB regardless of thresholds
    try:
        item = {
            'device_id': str(device_id),
            'timestamp': timestamp,
            'Temperature': Decimal(str(temperature)),
            'GasValue': Decimal(str(gas_value)),
            'Humidity': Decimal(str(humidity))
        }
        response = table.put_item(Item=item)
        print("Data saved to DynamoDB:", response)
    except Exception as e:
        print(f"Error storing data to DynamoDB: {e}")

    # Check if thresholds are exceeded and create alert messages
    messages = []
    if temperature > TEMPERATURE_THRESHOLD:
        messages.append("The window is open because the temperature is high.")
    if gas_value > GAS_VALUE_THRESHOLD:
        messages.append("The fan is working because the Gas value is high.")
    if humidity > HUMIDITY_THRESHOLD:
        messages.append("The window is open because the humidity is high.")

    # Check if any messages are present and if the notification interval has passed
    current_time = time.time()
    if messages and (current_time - last_notification_time) >= NOTIFICATION_INTERVAL:
        # Send notifications for each exceeded threshold message
        for message in messages:
            print("Sending alert:", message)
            try:
                sns.publish(
                    TopicArn=SNS_TOPIC_ARN,
                    Message=message,
                    Subject="Sensor Threshold Alert"
                )
                print("Notification sent via SNS.")
            except Exception as sns_error:
                print(f"Error sending SNS notification: {sns_error}")
        
        # Update last notification time after sending alerts
        last_notification_time = current_time
    else:
        print("No alerts needed, or interval not reached.")

    return {
        'statusCode': 200,
        'body': json.dumps('Data processed. Notifications sent if conditions met.')
    }
