import boto3
import sys
import json

dynamodb_client = boto3.client('dynamodb')
dynamodb = boto3.resource('dynamodb', region_name='me-south-1')
dynamodb_table = dynamodb.Table('UsersDatabase')
iot_client = boto3.client('iot-data', region_name='me-south-1')
    
def lambda_handler(event, context):
    received_message = event.get('message')
    
    if received_message == "check fingerPrint":

        dynamodb = boto3.resource('dynamodb')
        table = dynamodb.Table('UsersDatabase')

        response = table.query(
            IndexName='FingerPrintID-index',
            KeyConditionExpression="FingerPrintID = :val",
            ExpressionAttributeValues={":val": event['fingerPrintID']}
        )

        item = response.get('Items', [])

        found = "False"
        ID = ""
        if item != []:
            found = "True"
            ID = response.get('Items', [])[0].get('ID', 'No matching record found')

        response_message = {
            "message": "response check fingerPrint",
            "found": found,
            "id": ID
        }

        payload_size = sys.getsizeof(json.dumps(response_message))
        print("Payload size:", payload_size, "bytes")

        iot_client.publish(
            topic='smartLock/sub',
            qos=1,
            payload=json.dumps(response_message)
        )
        print(response_message)
    
    elif received_message == "get userToken":
        response = dynamodb_client.get_item(
        TableName='UsersDatabase',
        Key={
            'ID': {
                'S': event['userId']
                }
            }
        )
        
        item = response.get('Item', {})  
        token = item.get('Token', {}).get('S')
        
        response_message = {
            "message": "response get userToken",
            "token": token
        }
        
        payload_size = sys.getsizeof(json.dumps(response_message))
        print("Payload size:", payload_size, "bytes")
        
        iot_client.publish(
            topic='smartLock/sub',
            qos=1,
            payload=json.dumps(response_message)
        )
        print(response_message)
        
    elif received_message == "get username":
        response = dynamodb_client.get_item(
        TableName='UsersDatabase',
        Key={
            'ID': {
                'S': event['userId']
                }
            }
        )
        
        item = response.get('Item', {})  
        firstname = item.get('First Name', {}).get('S')
        lastname = item.get('Last Name', {}).get('S')
        username = firstname + " " + lastname
        
        response_message = {
            "message": "response get username",
            "username": username
        }
        
        payload_size = sys.getsizeof(json.dumps(response_message))
        print("Payload size:", payload_size, "bytes")
        
        iot_client.publish(
            topic='smartLock/sub',
            qos=1,
            payload=json.dumps(response_message)
        )
        print(response_message)    
    
    elif received_message == "Lock Opened":
        
        response = dynamodb_client.get_item(
        TableName='UsersDatabase',
        Key={
            'ID': {
                'S': event['id']
                }
            }
        )
        
        item = response.get('Item', {})  # Get the item dictionary
        firstname = item.get('First Name', {}).get('S')
        lastname = item.get('Last Name', {}).get('S')
        username = firstname + " " + lastname
        lockState = "The Lock Opened"+ " " + event['by']
        
        dynamodb_client.put_item(
            TableName = 'LockUpdates',
            Item={
                'timestamp': {
                    'S':event['timestamp']
                },
                'LockState': {
                    'S': lockState
                },
                'Name': {
                    'S': username
                },
                "ID": {
                    'S': event['id']
                },
                "sort": {
                    'N': '0'
                }
            }
        )

    elif received_message == "Lock Opened Admin":
        response = dynamodb_client.get_item(
        TableName='Admins',
        Key={
            'AID': {
                'S': event['id']
                }
            }
        )
        
        item = response.get('Item', {})  # Get the item dictionary
        print(item)
        name = item.get('Name', {}).get('S')
        lockState = "The Lock Opened"+ " " + event['by']
        dynamodb_client.put_item(
            TableName = 'LockUpdates',
            Item={
                'timestamp': {
                    'S':event['timestamp']
                },
                'LockState': {
                    'S': lockState
                },
                'Name': {
                    'S': name
                },
                "ID": {
                    'S': event['id']
                },                
                "sort": {
                    'N': '0'
                }
            }
        )
        
    elif received_message == "check uid":
        dynamodb = boto3.resource('dynamodb')
        table = dynamodb.Table('UsersDatabase')
        response = table.query(
            IndexName='RFID-index',  
            KeyConditionExpression="RFID = :val",
            ExpressionAttributeValues={":val": event['uid']}
        )
        item = response.get('Items', [])
        found = "False"
        ID = ""
        if item != []:
            found = "True"
            ID = response.get('Items', [])[0].get('ID', 'No matching record found')
        else:
            found = "False"
            
        response_message = {
            "message": "response check uid",
            "found": found,
            "id": ID
        }
        payload_size = sys.getsizeof(json.dumps(response_message))
        print("Payload size:", payload_size, "bytes")
        
        iot_client.publish(
            topic='smartLock/sub',
            qos=1,
            payload=json.dumps(response_message)
        )
        print(response_message)    
        
    elif received_message == "check password":
        response = dynamodb_client.get_item(
        TableName='UsersDatabase',
        Key={
            'ID': {
                'S': event['id']
                }
            }
        )
        
        item = response.get('Item', {})  # Get the item dictionary
        password = item.get('Password', {}).get('S')
        
        valid = "False"
        if password == event['password']:
            valid = "True"
            
        response_message = {
            "message": "response check password",
            "valid": valid
        }
        payload_size = sys.getsizeof(json.dumps(response_message))
        print("Payload size:", payload_size, "bytes")
        
        iot_client.publish(
            topic='smartLock/sub',
            qos=1,
            payload=json.dumps(response_message)
        )
        print(response_message)    
        
        
    elif received_message == "save uid":
        response = dynamodb_client.update_item(
            TableName='UsersDatabase',
            Key={
                'ID': {
                    'S': event['id']
                }
            },
            UpdateExpression='SET RFID = :new_rfid',
            ExpressionAttributeValues={
                ':new_rfid': {'S': event['uid']}
            }
        )
        print(response)    
        
    elif received_message == "save password":
        response = dynamodb_client.update_item(
            TableName='UsersDatabase',
            Key={
                'ID': {
                    'S': event['id']
                }
            },
            UpdateExpression='SET Password = :new_pass',
            ExpressionAttributeValues={
                ':new_pass': {'S': event['password']}
            }
        )
        print(response)    
        
    elif received_message == "save fingerPrint":
        response = dynamodb_client.update_item(
            TableName='UsersDatabase',
            Key={
                'ID': {
                    'S': event['id']
                }
            },
            UpdateExpression='SET FingerPrintID = :new_finger',
            ExpressionAttributeValues={
                ':new_finger': {'S': event['fingerID']}
            }
        )
        print(response) 
        
    elif received_message == "save token":
        response = dynamodb_table.update_item(
            Key={'ID': event.get('id')},
            UpdateExpression=f'SET #attr = :value',  # Use expression attribute name
            ExpressionAttributeNames={'#attr': "Token"},  # Define the attribute name
            ExpressionAttributeValues={':value': event.get('token')},
            ReturnValues='UPDATED_NEW'
        )
        
        print(response)

    return "No Error"

