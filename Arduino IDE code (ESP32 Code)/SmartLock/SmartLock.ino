// Include necessary libraries
#include "Secrets.h"                             
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <Keypad.h>
#include "LiquidCrystal_I2C.h"                   
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <MFRC522.h>
#include "time.h"
#include "sntp.h"
#include <CustomJWT.h> 
#include <ctime>
#include <MD5.h>


/*-------------------------------- Define ---------------------------------------*/
#define ACCESS_DELAY 5000 // Delay for access control in seconds
#define mySerial Serial1  // Fingerprint sensor setup
#define SS_PIN 21         // SDI pin for RFID
#define RST_PIN 34        // Rest pin for RFID
/*-------------------------------- Define ---------------------------------------*/


/*-------------------------------- Setup Components -----------------------------------*/
// Define NTP servers for time synchronization
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 10800; // GMT offset in seconds (Bahrain +3)

// Fingerprint setup
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// RFID reader setup
MFRC522 rfid(SS_PIN, RST_PIN);

// LCD display setup
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); 

// Keypad configuration
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Three columns
char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};
byte rowPins[ROWS] = {12, 27, 33, 15}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {32, 14, 25, 26}; // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// WiFi client setup
WiFiClientSecure net = WiFiClientSecure(); 
PubSubClient client(net);

// JWT setup 
CustomJWT jwt(SECRET_KEY, 256); 
/*-------------------------------- Setup Components -----------------------------------*/


/*-------------------------------- Global Varibles ---------------------------------------*/
// Global  varibles 
String Global_Found;
String Global_Token;
String Global_ID;
String Global_Username;
String Global_IsPasswordValid;
String Global_RegisterFingerID;

// Flags to synchronous communication between esp and aws
int getUerToken_Flag = 0;
int checkFingerPrint_Flag = 0;
int getUsername_Flag = 0;
int checkUid_Flag = 0;
int checkPassword_Flag = 0;

// Rest all global variables
void resetGlobalVariables(){
  Global_Found = "";
  Global_Token = "";
  Global_ID = "";
  Global_Username = "";
  Global_IsPasswordValid = "";
  Global_RegisterFingerID = "";
  
  getUerToken_Flag = 0;
  checkFingerPrint_Flag = 0;
  getUsername_Flag = 0;
  checkUid_Flag = 0;
  checkPassword_Flag = 0;
}

// Print Global variables (used for testing and debugging)
void printGlobalVariables(){
  Serial.print("Global_Found: ");
  Serial.println(Global_Found);   

  Serial.print("Global_Token: ");
  Serial.println(Global_Token);   
  
  Serial.print("Global_ID: ");
  Serial.println(Global_ID);   
  
  Serial.print("Global_Username: ");
  Serial.println(Global_Username);     
  
  Serial.print("Global_IsPasswordValid: ");
  Serial.println(Global_IsPasswordValid);   
  
  Serial.print("checkFingerPrint_Flag: ");
  Serial.println(checkFingerPrint_Flag); 

  Serial.print("getUerToken_Flag: ");
  Serial.println(getUerToken_Flag);   
  
  Serial.print("getUsername_Flag: ");
  Serial.println(getUsername_Flag); 

  Serial.print("checkUid_Flag: ");
  Serial.println(checkUid_Flag);  
  
  Serial.print("checkPassword_Flag: ");
  Serial.println(checkPassword_Flag);   
  
  Serial.print("Global_RegisterFingerID: ");
  Serial.println(Global_RegisterFingerID); 

}
/*-------------------------------- Global Varibles ---------------------------------------*/


/*---------------------------- Connecting To The Cloud -----------------------------------*/
// Connect to Wifi and AWS 
void connectAWS() 
{
  // Set WiFi mode and connect to the specified network
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);
  updateLCD("Con. to Wi-Fi: ",WIFI_SSID);

  // Wait until connected to Wi-Fi
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    lcd.setCursor(sizeof(WIFI_SSID)-1,1);  
    lcd.print(".");     
    delay(500);
    lcd.setCursor(sizeof(WIFI_SSID)-1,1);  
    lcd.print(" ");
    delay(500);
  }
  Serial.println("Connected");

  // Configure WiFiClientSecure with AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Set up a message handler
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IoT");
  updateLCD("Con. to AWS IoT:");

  // Attempt to connect to AWS IoT
  while (!client.connect(THINGNAME)){
    Serial.print(".");
    lcd.setCursor(0,1);  
    lcd.print(".");     
    delay(500);
    lcd.setCursor(0,1);  
    lcd.print(" ");
    delay(500);
  }

  if (!client.connected()){
      Serial.println("AWS IoT Timeout!");
      return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
  updateLCD("Connected");
  delay(1000);
  updateLCD("Use FingerPrint","    Or RFID");
}
/*---------------------------- Connecting To The Cloud -----------------------------------*/


/*----------------------------- Date and Time Process -------------------------------------*/
// Retrieve the local time and date from NTP server 
String getLocalTime() 
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("No time available (yet)"); // Print an error message if time retrieval fails
        return ""; // Return an empty string or handle the error condition
    }

    // Extract day, month, and year (formatted as DD/MM/YYYY)
    String formattedDate = "";

    // Ensure consistent formatting for day and month
    if (timeinfo.tm_mday < 10) formattedDate += "0"; // Add a leading zero if day is less than 10
      formattedDate += String(timeinfo.tm_mday) + "/";

    if ((timeinfo.tm_mon + 1) < 10) formattedDate += "0"; // Add a leading zero if month is less than 10
      formattedDate += String(timeinfo.tm_mon + 1) + "/" + String(timeinfo.tm_year + 1900);

    // Extract hours, minutes, and seconds (formatted as HH:MM:SS)
    String formattedTime = "";

    // Ensure consistent formatting for hours, minutes, and seconds
    if (timeinfo.tm_hour < 10) formattedTime += "0"; // Add a leading zero if hours are less than 10
      formattedTime += String(timeinfo.tm_hour) + ":";

    if (timeinfo.tm_min < 10) formattedTime += "0"; // Add a leading zero if minutes are less than 10
      formattedTime += String(timeinfo.tm_min) + ":";

    if (timeinfo.tm_sec < 10) formattedTime += "0"; // Add a leading zero if seconds are less than 10
      formattedTime += String(timeinfo.tm_sec);

    String localTime = formattedDate + " " + formattedTime;
    return localTime;
}

// Function to check if the date and time exceed the local time or not, If not exceed return true
bool isNotExpired(int dayL, int monthL, int yearL, int hourL, int minutesL, int secondsL,
               int dayE, int monthE, int yearE, int hourE, int minutesE, int secondsE) {
  if (yearL > yearE) {
      return false;
  } else if (yearL == yearE) {
      if (monthL > monthE) {
          return false;
      } else if (monthL == monthE) {
          if (dayL > dayE) {
              return false;
          } else if (dayL == dayE) {
              if (hourL > hourE) {
                  return false;
              } else if (hourL == hourE) {
                  if (minutesL > minutesE) {
                      return false;
                  } else if (minutesL == minutesE) {
                      return secondsL <= secondsE;
                  }
              }
          }
      }
  }
  return true; // not expired
}

// Function to extract date and time components from a date-time string
void extractDateTime(const String& dateTimeStr, int& day, int& month, int& year,
                     int& hour, int& minutes, int& seconds) {
  // Split the string by space to separate date and time
  String datePart = dateTimeStr.substring(0, 10); // Extract "25/04/2024"
  String timePart = dateTimeStr.substring(11);    // Extract "12:09:17"

  // Split the date part by '/' to extract day, month, and year
  int firstSlashIndex = datePart.indexOf('/');
  int secondSlashIndex = datePart.indexOf('/', firstSlashIndex + 1);

  day = datePart.substring(0, firstSlashIndex).toInt();
  month = datePart.substring(firstSlashIndex + 1, secondSlashIndex).toInt();
  year = datePart.substring(secondSlashIndex + 1).toInt();

  // Split the time part by ':' to extract hour, minutes, and seconds
  int firstColonIndex = timePart.indexOf(':');
  int secondColonIndex = timePart.indexOf(':', firstColonIndex + 1);

  hour = timePart.substring(0, firstColonIndex).toInt();
  minutes = timePart.substring(firstColonIndex + 1, secondColonIndex).toInt();
  seconds = timePart.substring(secondColonIndex + 1).toInt();
}

// Function to check if local time is within the specified range
bool isLocalTimeInRange(int dayF, int monthF, int yearF, int hourF, int minutesF, int secondsF,
                        int dayT, int monthT, int yearT, int hourT, int minutesT, int secondsT,
                        int dayL, int monthL, int yearL, int hourL, int minutesL, int secondsL) {
  struct tm fromTime = {secondsF, minutesF, hourF, dayF, monthF - 1, yearF - 1900};
  struct tm toTime = {secondsT, minutesT, hourT, dayT, monthT - 1, yearT - 1900};
  struct tm localTime = {secondsL, minutesL, hourL, dayL, monthL - 1, yearL - 1900};

  time_t fromTimestamp = mktime(&fromTime);
  time_t toTimestamp = mktime(&toTime);
  time_t localTimestamp = mktime(&localTime);

  // Check if local time is within the range
  return (localTimestamp >= fromTimestamp) && (localTimestamp <= toTimestamp);
}

// Function to check if the local time is within the specified expiration time range
bool isWithinRange(const String& expirationTime, const String& localTime) {
    // Extract the start time and end time from the expirationTime string
    String startTime = expirationTime.substring(0, 8);
    String endTime = expirationTime.substring(9);

    // Convert the start time, end time, and local time strings to tm struct
    struct tm startTm, endTm, currentTm;
    strptime(startTime.c_str(), "%H:%M:%S", &startTm);
    strptime(endTime.c_str(), "%H:%M:%S", &endTm);

    // Get the current time in seconds since epoch
    time_t now = time(nullptr);
    currentTm = *localtime(&now);

    // Set today's date for start and end times
    startTm.tm_year = currentTm.tm_year;
    startTm.tm_mon = currentTm.tm_mon;
    startTm.tm_mday = currentTm.tm_mday;
    endTm.tm_year = currentTm.tm_year;
    endTm.tm_mon = currentTm.tm_mon;
    endTm.tm_mday = currentTm.tm_mday;

    // Get the start and end timestamps for today
    time_t startTimestamp = mktime(&startTm);
    time_t endTimestamp = mktime(&endTm);
    Serial.print("startTimestamp: ");
    Serial.println(startTimestamp);    
    Serial.print("endTimestamp: ");
    Serial.println(endTimestamp);    
    Serial.print("currentTimestamp: ");
    Serial.println(now);

    // Compare the current time with the start time and end time
    if (now >= startTimestamp && now <= endTimestamp) {
        return true;
    }

    return false;
}
/*----------------------------- Date and Time Process -------------------------------------*/


/*--------------------------------- LCD Functions ----------------------------------------*/
// update the row1 and row2 in LCD
void updateLCD(String row1, String row2) {
  lcd.clear();
  lcd.setCursor(0, 0);  // Move cursor to the beginning of the first row
  lcd.print(row1);      // Display text on the first row
  lcd.setCursor(0, 1);  // Move cursor to the beginning of the second row
  lcd.print(row2);      // Display text on the second row
}

// update the row1 in LCD
void updateLCD(String row1) {
  lcd.clear();
  lcd.setCursor(0, 0);  // Move cursor to the beginning of the first row
  lcd.print(row1);      // Display text on the first row
}
/*--------------------------------- LCD Functions ----------------------------------------*/


/*---------------------------------- FingerPrint -----------------------------------------*/
// This function to return fingerPrint ID if it's found else return the error number to handel the error
int getFingerPrint() {
  int p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER ) return -1;

  int x = finger.image2Tz();
  if (x != FINGERPRINT_OK) return -2;

  int y  = finger.fingerSearch();
  if (y != FINGERPRINT_OK) return -3;
  
  // found a match then return the fingerID
  return finger.fingerID;
}

// check if the fingerPrint id exist in the database
bool checkFingerPrint(int FingerPrint){
  String FingerID = String(FingerPrint);
  StaticJsonDocument<200> doc;
  char timeBuffer[20];
  doc["fingerPrintID"] = FingerID;
  doc["message"] = "check fingerPrint";
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  Serial.println("Message 'check fingerPrint' sent to AWS");

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  while(checkFingerPrint_Flag == 0){client.loop();}

  if(Global_Found == "True"){return true;}
  return false;
}

// The following three function to enroll new finger print 
uint8_t id;
uint8_t readnumber(void)
{
  uint8_t num = 0;

  while (num == 0)
  {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}
bool enrollFingerprint(uint8_t newId)
{
  id = newId;
  if (id == 0)
  { // ID #0 not allowed, try again!
    return false;
  }

  Serial.print("Enrolling ID #");
  Serial.println(id);

  return getFingerprintEnroll();
}
uint8_t getFingerprintEnroll()
{
  int p = -1;
  Serial.print("Waiting for a valid finger to enroll as #");
  updateLCD("Put Your Finger");
  Serial.println(id);

  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  Serial.println("Remove finger");
  updateLCD("Remove Your Finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place the same finger again");
  updateLCD("Place the same"," Finger Again");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Stored!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}
/*---------------------------------- FingerPrint -----------------------------------------*/


/*---------------------------------- Token Process -----------------------------------------*/
// Generate Token
String generateToken(const String& id, const String& exp, const String& state) {
    // Create the JSON payload string
    String payload = "{\"id\":\"" + id + "\",\"exp\":\"" + exp + "\",\"state\":\"" + state + "\"}";
    
    // Convert the payload to a mutable char array
    char payloadArray[payload.length() + 1];
    payload.toCharArray(payloadArray, sizeof(payloadArray));
    
    // Allocate memory for JWT
    jwt.allocateJWTMemory();
    
    // Encode the JWT
    jwt.encodeJWT(payloadArray);
    
    // Get the generated token
    String generatedToken(jwt.out);
    
    // Clear the JWT memory
    jwt.clear();

    // Return the generated token
    return generatedToken;
}

// Send message to get the user Token from the DataBase
void getUserToken(){
  StaticJsonDocument<200> doc;
  char timeBuffer[20];
  doc["userId"] = Global_ID;
  doc["message"] = "get userToken";
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  // above line it will generate the below message 
  // {
  //   "userId": "123",
  //   "message": get userToken
  // }

  Serial.println("Message 'get userToken' sent to AWS");

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  while(getUerToken_Flag == 0){client.loop();}
}

/*
validate the Token by extracting the user token processing the payload message and returning true if it's valid 
if state ="1": the token will be valid until the expirationTime
  format of the token: 
  {
    "id": "1234",
    "exp": "20/04/2024 20:30:00",
    "state": "1"
  }

if state = "2" the token will be valid between the two dates in "exp"
  format of the token: 
  {
    "id": "1234",
    "exp": "20/04/2024 20:30:00 - 22/04/2024 20:30:00",
    "state": "2"
  }

if state = "3" the token will be valid every day between the two times that in "exp"
  format of the token: 
  {
  "id": "123",
  "exp": "09:30:00 15:30:00",
  "state": "3"
  }

if the "exp" = "-" in any case that means the token is always valid
NOTE: the date and the time must be in this format: DD/MM/YYYY HH:MM:SS
 - if it's range must be: DD/MM/YYYY HH:MM:SS - DD/MM/YYYY HH:MM:SS
 - if it's daily, it must be: HH:MM:SS HH:MM:SS
*/
bool ValidateToken(){
  jwt.allocateJWTMemory();
  int tokenLength = Global_Token.length() + 1;
  char* string = new char[tokenLength];
  Global_Token.toCharArray(string, tokenLength);

  int decodeResult = jwt.decodeJWT(string);
  Serial.print("\nDecoding JWT with result: ");
  Serial.println(decodeResult);
  /*
  decodeResult 0: The function ran without any problems
  decodeResult 1: Memory has not been allocated
  decodeResult 2: Input is not valid JWT
  decodeResult 3: Signature validation failed
  */
  if (decodeResult == 0){       
    Serial.println("Payload Info");
    String payload = jwt.payload;
    Serial.println(payload);

    // Parse the JSON payload
    StaticJsonDocument<200> doc; // Adjust the size as needed
    DeserializationError error = deserializeJson(doc, payload);

    String id = doc["id"].as<String>();
    String expirationTime = doc["exp"].as<String>();
    String state = doc["state"].as<String>();

    if(id != Global_ID){
      jwt.clear();
      return false;
    }

    if(expirationTime.equals("-")){
      jwt.clear();
      return true;
    }

    String localTime = getLocalTime();
    Serial.print("loclTimestamp: ");
    Serial.println(localTime);

    Serial.print("Expiration Time: ");
    Serial.println(expirationTime);

    if(state == "1"){
      int dayE, monthE, yearE, hourE, minutesE, secondsE;
      int dayL, monthL, yearL, hourL, minutesL, secondsL;

      extractDateTime(expirationTime, dayE, monthE, yearE, hourE, minutesE, secondsE);
      extractDateTime(localTime, dayL, monthL, yearL, hourL, minutesL, secondsL);

      if(isNotExpired(
        dayL, monthL, yearL, hourL, minutesL, secondsL,
        dayE, monthE, yearE, hourE, minutesE, secondsE
      )){
        jwt.clear();
        return true;
      }else{
        jwt.clear();
        return false;
      }
    }else if(state == "2"){
      int separatorIndex = expirationTime.indexOf(" - ");
      String fromDateStr = expirationTime.substring(0, separatorIndex);
      String toDateStr = expirationTime.substring(separatorIndex + 3);

      int dayF, monthF, yearF, hourF, minutesF, secondsF;
      int dayT, monthT, yearT, hourT, minutesT, secondsT;
      int dayL, monthL, yearL, hourL, minutesL, secondsL;

      extractDateTime(fromDateStr, dayF, monthF, yearF, hourF, minutesF, secondsT);
      extractDateTime(toDateStr, dayT, monthT, yearT, hourT, minutesT, secondsT);
      extractDateTime(localTime, dayL, monthL, yearL, hourL, minutesL, secondsL);

      bool isInRange = isLocalTimeInRange(dayF, monthF, yearF, hourF, minutesF,secondsF,
                                        dayT, monthT, yearT, hourT, minutesT,secondsT,
                                        dayL, monthL, yearL, hourL, minutesL,secondsL);
      if (isInRange) {
        jwt.clear();
        return true;
      }else {
        jwt.clear();
        return false;
      }

    }else if(state == "3"){
      String timePart = localTime.substring(11); 
      bool isInDailyRange = isWithinRange(expirationTime,timePart);
      
      if(isInDailyRange) {
        jwt.clear();
        return true;
      }else {
        jwt.clear();
        return false;
      }

    }else{
      jwt.clear();
      return false;
    }
  }
  else if(decodeResult == 3){
    jwt.clear();
    Serial.println("Invalid Signature");
    return false;
  }
  else{
    jwt.clear();
    Serial.println("Error Decoding");
    return false;
  }
  jwt.clear();
  return true;
}
/*---------------------------------- Token Process -----------------------------------------*/


/*---------------------------------- Password Process -----------------------------------------*/
// Function to generate MD5 hash
String generateMD5Hash(String input) {
  int inputLength = input.length() + 1;
  char* char_input = new char[inputLength];
  input.toCharArray(char_input, inputLength);
  unsigned char* hash = MD5::make_hash(char_input);
  char* md5str = MD5::make_digest(hash, 16);
  String md5string = String(md5str);
  free(hash);
  return md5string;
}

// Function to get the user password form keypad
String getUserPassword() {
    updateLCD("Enter Password: ");     
      
    static String userPassword = "";
    char key;
    while (true) {
        key = keypad.getKey();
        if (key) {
            if (key == '#' && !userPassword.isEmpty()) {
                String returnedPassword = userPassword;
                userPassword = "";
                Serial.println();
                String hashedPassword = generateMD5Hash(returnedPassword);
                return hashedPassword; 
            } 
            else if (key == '*' && !userPassword.isEmpty()) {    
                userPassword.remove(userPassword.length() - 1, 1);        
            }
            else if (key == '#' && userPassword.isEmpty()) {    
                userPassword = "";
            } 
            else if (key == '*' && userPassword.isEmpty()) {
                userPassword = "";
            }
            else {
                userPassword += key;
            }
            String pass = "";
            for(int i=0; i<userPassword.length(); i++){
              pass += '*';
            }
            updateLCD("Password: ", pass);       
        }
    }
    return "";
}

// to check if the password match with password stored in the database
bool verifyPassword(String UserPassword) {
  StaticJsonDocument<200> doc;
  char timeBuffer[20];
  doc["id"] = Global_ID;
  doc["password"] = UserPassword;
  doc["message"] = "check password";
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.println("Message 'check password' sent to AWS");

  while(checkPassword_Flag == 0){client.loop();}

 if(Global_IsPasswordValid == "True"){return true;}
  return false;

}
/*---------------------------------- Password Process -----------------------------------------*/


/*---------------------------------- RFID Process -----------------------------------------*/
// Get uid of the card to enroll new RFID card
bool getUid(String& uid){
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if (!rfid.PICC_IsNewCardPresent())
    return false;

	if (!rfid.PICC_ReadCardSerial())
    return false;


  uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      uid += String(rfid.uid.uidByte[i], HEX);
  }
  // Halt PICC
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return true;

}

int getRFID(){
  if ( ! rfid.PICC_IsNewCardPresent()) {
		return -1;
	}
	// Select one of the cards
	if ( ! rfid.PICC_ReadCardSerial()) {
		return -3;
	}

  Serial.println("----------------------------------------------------------");
  updateLCD("Read RFID....");

  // Get the UID as a hexadecimal string
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
      uidString += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      uidString += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.print("Card Uid: ");
  Serial.println(uidString);
 
  StaticJsonDocument<200> doc;
  char timeBuffer[20];
  doc["uid"] = uidString;
  doc["message"] = "check uid";
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

  Serial.println("Message 'check uid' sent to AWS");

  while(checkUid_Flag == 0){client.loop();}

  	// Halt PICC
	rfid.PICC_HaltA();
	rfid.PCD_StopCrypto1();
  
  if(Global_Found == "True"){return 1;}
  return -2;
}
/*---------------------------------- RFID Process -----------------------------------------*/


/*---------------------------------- Message Handler ---------------------------------------*/
// Function to handle incoming MQTT messages
void messageHandler(char* topic, byte* payload, unsigned int length) {
    Serial.print("Incoming message on topic [");
    Serial.print(topic);
    Serial.print("]: ");

    // Deserialize the incoming JSON payload
    StaticJsonDocument<1200> doc;
    deserializeJson(doc, payload);
    const char* message = doc["message"]; 

    Serial.println(message);

    // Check the message type and update global variables accordingly
    if (strcmp(message, "response check fingerPrint") == 0) {
      Global_ID = doc["id"].as<String>(); // convert the id from the Json message to String 
      Global_Found = doc["found"].as<String>();
      checkFingerPrint_Flag = 1;
    }
    else if(strcmp(message, "response get userToken") == 0){
      Global_Token = doc["token"].as<String>();
      getUerToken_Flag = 1;
    }
    else if(strcmp(message, "response get username") == 0){
      Global_Username = doc["username"].as<String>();
      getUsername_Flag = 1;
    }
    else if(strcmp(message, "response check uid") == 0){
      Global_ID = doc["id"].as<String>(); // convert the id from the Json message to String 
      Global_Found = doc["found"].as<String>();
      checkUid_Flag = 1;
    }
    else if(strcmp(message, "response check password") == 0){
      Global_IsPasswordValid = doc["valid"].as<String>(); // convert the id from the Json message to String 
      checkPassword_Flag = 1;
    }
    else if(strcmp(message, "openlock by admin") == 0){
      Global_ID = doc["id"].as<String>();
      Global_Username = doc["name"].as<String>();
      updateLCD("Welcome:",Global_Username);
      openTheLock(Global_ID, "By Admin","1");
    }    
    else if(strcmp(message, "register Finger ID") == 0){
      Global_ID = doc["id"].as<String>();
      Global_RegisterFingerID = doc["FID"].as<String>();
      
      // Call the enrollFingerprint() function to enroll a new fingerprint
      uint8_t newId = Global_RegisterFingerID.toInt(); // Set the desired ID number
      bool enrollmentResult = enrollFingerprint(newId);

      if (enrollmentResult){
        StaticJsonDocument<200> doc;
        char timeBuffer[20];
        doc["id"] = Global_ID;
        doc["fingerID"] = Global_RegisterFingerID;
        doc["message"] = "save fingerPrint";
        char jsonBuffer[512];
        serializeJson(doc, jsonBuffer);

        Serial.println("Message 'save fingerPrint' sent to AWS");

        client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
        Serial.println("Fingerprint enrollment successful");
      }
      else{
        Serial.println("Fingerprint enrollment failed");
      }
      updateLCD("Use FingerPrint","    Or RFID");
    }    
    else if(strcmp(message, "register RFID") == 0){
      updateLCD("Put RFID card");
      Global_ID = doc["id"].as<String>();
      String uid;
      while(true){
        if(getUid(uid)){
          Serial.print("reading uid: ");
          Serial.println(uid);
          StaticJsonDocument<200> doc;
          char timeBuffer[20];
          doc["id"] = Global_ID;
          doc["uid"] = uid;
          doc["message"] = "save uid";
          char jsonBuffer[512];
          serializeJson(doc, jsonBuffer);

          Serial.println("Message 'save uid' sent to AWS");

          client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
          break;
        }
      }

      updateLCD("Use FingerPrint","    Or RFID");
    }    
    else if(strcmp(message, "register password") == 0){
      Global_ID = doc["id"].as<String>();
      String UserPassword = getUserPassword();
      StaticJsonDocument<200> doc;
      char timeBuffer[20];
      doc["id"] = Global_ID;
      doc["password"] = UserPassword;
      doc["message"] = "save password";
      char jsonBuffer[512];
      serializeJson(doc, jsonBuffer);

      Serial.println("Message 'save password' sent to AWS");

      client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
      updateLCD("Use FingerPrint","    Or RFID");
    }    
    else if(strcmp(message, "register token") == 0){
      Global_ID = doc["id"].as<String>();
      String exp = doc["exp"].as<String>();
      String state = doc["state"].as<String>();
      String token = generateToken(Global_ID, exp, state);
      
      StaticJsonDocument<200> doc;
      char timeBuffer[20];
      doc["id"] = Global_ID;
      doc["token"] = token;
      doc["message"] = "save token";
      char jsonBuffer[512];
      serializeJson(doc, jsonBuffer);

      Serial.println("Message 'save token' sent to AWS");

      client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

      updateLCD("Use FingerPrint","    Or RFID");
    }
    else {
      Serial.println("Error message"); 
    }
}

// Open the lock and send message to to LockUpdates table
// state 0: user
// state 1: admin
void openTheLock(String userID, String OpenBy, String state) {
    digitalWrite(4, HIGH);               // Activate the lock 
    String localTime = getLocalTime();    // Get the local time
    
    // Create a JSON document to store lock opening details
    StaticJsonDocument<200> doc;
    char timeBuffer[20];
    if (state == "0"){
      doc["message"] = "Lock Opened"; // Indicate that the lock was opened
    }
    else{
      doc["message"] = "Lock Opened Admin"; // Indicate that the lock was opened
    }
    doc["timestamp"] = localTime;   // Store the timestamp
    doc["id"] = userID;             // Store the user ID
    doc["by"] = OpenBy;

    // Serialize the JSON document to a buffer
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);

    // Publish the JSON message to the AWS IoT topic
    client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
    Serial.println("Message 'Lock Opened' sent to AWS");

    delay(ACCESS_DELAY);       // Keep the lock opend for 5 Seccond
    digitalWrite(4, LOW);     // Close the lock
    updateLCD("Lock Closed");
    delay(2000);
    updateLCD("Use FingerPrint","    Or RFID");
}

// Function to get the User name from the database
void getUserName(){
  StaticJsonDocument<200> doc;
  char timeBuffer[20];
  doc["userId"] = Global_ID;
  doc["message"] = "get username";
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  Serial.println("Message 'get username' sent to AWS");

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  while(getUsername_Flag == 0){client.loop();}
}
/*---------------------------------- Message Handler ---------------------------------------*/


/*---------------------------------- MAIN FUNCTION ---------------------------------------*/
// Main Function of FingerPrint
void FingerPrint(){
  int fingerPrintID = getFingerPrint();
  while (fingerPrintID != -1) {
    if (fingerPrintID > 0){
      
      Serial.println("----------------------------------------------------------");
      updateLCD("Wait...");
      Serial.print("fingerPrintID: ");
      Serial.println(fingerPrintID);

      if(checkFingerPrint(fingerPrintID)){
        getUserToken();
        
        if(ValidateToken()){
          Serial.println("\nValid Token. Access allowed.");
          getUserName();
          updateLCD("Welcome:",Global_Username);
          openTheLock(Global_ID,"By finger","0");
        }
        else{
          Serial.println("\nInvalid token. Access denied.");
          updateLCD("Invalid token","Access denied.");
          delay(2000);
        }
      }
      else{
        Serial.println("Not Found in DB");
        updateLCD("Access denied");
        delay(2000);
      }
    }
    else{
      updateLCD("Access denied");
      delay(2000);
    }
    resetGlobalVariables();
    updateLCD("Use FingerPrint","    Or RFID");
    Serial.println("----------------------------------------------------------");
    break;
  }
}

// Main Function of RFID
void RFID(){
  // the user can access only if the card it's valid and the user enter the correct password and his token is valid 
  int state = getRFID();  
  if(state == 1){ // the card is found in the database
    String UserPassword = getUserPassword(); // get the user password 
    if(verifyPassword(UserPassword)){ // check the pass if it's match the user pass on the DB
      updateLCD("Wait...");
      getUserToken();
      if(ValidateToken()){
        Serial.println("\nValid Token. Access allowed.");
        getUserName();
        updateLCD("Welcome:",Global_Username);
        openTheLock(Global_ID,"By RFID","0");
      }
      else{
        Serial.println("\nInvalid token. Access denied.");
        updateLCD("Invalid token","Access denied.");
        delay(2000);
      }
    }
    else{
      updateLCD("Wrong Password");
      delay(2000);
    }
    resetGlobalVariables();
    updateLCD("Use FingerPrint","    Or RFID");
    Serial.println("----------------------------------------------------------");
  }
  else if(state == -2){
    Serial.println("\nInvalid card. Access denied.");
    updateLCD("Invalid Card","Access denied.");
    delay(2000);
    resetGlobalVariables();
    updateLCD("Use FingerPrint","    Or RFID");
    Serial.println("----------------------------------------------------------");
  }
}
/*---------------------------------- MAIN FUNCTION ---------------------------------------*/

// setup function
void setup()
{
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  
  connectAWS();
  pinMode(4, OUTPUT);

  sntp_servermode_dhcp(1); 
  configTime(gmtOffset_sec, 0 ,ntpServer1, ntpServer2);

  finger.begin(57600);
  delay(5);

  SPI.begin();
  rfid.PCD_Init();

  updateLCD("Use FingerPrint","    Or RFID");
  delay(100);


}

// loop function
void loop() {
  // If the wifi disconnected connect the the wifi again before continue processing
  if(WiFi.status() != WL_CONNECTED){
    updateLCD("    NO WIFI    ","   Connection   ");
    delay(1500);
    while(true){
      connectAWS();
      if(WiFi.status() == WL_CONNECTED) break;
    }
  }

  client.loop();          // check if there is an incoming MQTT message from AWS
  FingerPrint();          // call fingerPrint function
  RFID();                 // call RFID function
}