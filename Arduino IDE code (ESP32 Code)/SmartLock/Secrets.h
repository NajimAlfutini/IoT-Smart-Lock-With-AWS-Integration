// To store large amount of data in the program space (flash memory) without using the RAM
#include <pgmspace.h>
 
#define SECRET
#define THINGNAME "---"  

// key to decode JWT token
char SECRET_KEY[] = "---";
 
const char WIFI_SSID[] = "---";              
const char WIFI_PASSWORD[] = "---";          
const char AWS_IOT_ENDPOINT[] = "---";       

// MQTT topics
#define AWS_IOT_PUBLISH_TOPIC   "---"
#define AWS_IOT_SUBSCRIBE_TOPIC "---"

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";
 
// Device Certificate                                             
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)KEY";
 
// Device Private Key                                              
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
)KEY";