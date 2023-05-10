# Arduino Google Forms Client Library for ESP32, ESP8266 and Raspberry Pi Pico (RP2040)


Arduino Google Forms Client Library for ESP32, ESP8266 and Raspberry Pi Pico (RP2040).


This library works with Google Forms APIs directly using the Service Account which is safe and reliable as it does not require  dirty HTTP hacks to work.
 

Devices will be able to create a form, add, remove, change position and update the form's items, get the responses, and form watches are also supported.


The form that created using this library, owned by the Service Account and shared access to the user.


## Dependencies


This library required **ESP8266, ESP32 and Raspberry Pi Pico Arduino Core SDK** to be installed.

To install device SDK, in Arduino IDE, ESP8266, ESP32 and Pico Core SDK can be installed through **Boards Manager**. 

In PlatfoemIO IDE, ESP32 and ESP8266 devices's Core SDK can be installed through **PIO Home** > **Platforms** > **Espressif 8266 or Espressif 32**.


### RP2040 Arduino SDK installation

For Arduino IDE, the Arduino-Pico SDK can be installed from Boards Manager by searching pico and choose Raspberry Pi Pico/RP2040 to install.

For PlatformIO, the Arduino-Pico SDK can be installed via platformio.ini

```ini
[env:rpipicow]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = rpipicow
framework = arduino
board_build.core = earlephilhower
monitor_speed = 115200
board_build.filesystem_size = 1m
```

See this Arduino-Pico SDK [documentation](https://arduino-pico.readthedocs.io/en/latest/) for more information.




## Prerequisites

This library used the Service Account crendentials to create the short lived access token for Google API authentication, which will be expired in 1 hour and will be refresh automatically.

You need to create the Service Account private key and enable Google Forms and Google Drive APIs for your project.

To enable Google Forms API for your project, go to https://console.cloud.google.com/apis/library/forms.googleapis.com

![Create SA](/media/images/API_Enable1.png)

To enable Google Drive API for your project, go to https://console.cloud.google.com/apis/library/drive.googleapis.com

![Create SA](/media/images/API_Enable2.png)


In case the form that works with this library was created or owned by you, you need to share that form to the Service Account client email.


## How to Create Service Account Private Key


Go to [Google Cloud Console](https://console.cloud.google.com/projectselector2/iam-admin/settings).

1. Choose or create project to create Service Account.

2. Choose Service Accounts

![Select Project](/media/images/GC_Select_Project.png)

3. Click at + CREAT SERVICE ACCOUNT.

![Create SA](/media/images/GC_Create_SA.png)

4. Enter the Service account name, 

5. Service account ID and

6. Click at CREATE AND CONTINUE

![Create SA2](/media/images/GC_Create_SA2.png)

7. Select Role.

8. Click at CONTINUE.

![Create SA3](/media/images/GC_Create_SA3.png)

9. Click at DONE.

![Create SA4](/media/images/GC_Create_SA4.png)

10. Choose service account that recently created from the list.

![Create SA5](/media/images/GC_Create_SA5.png)

11. Choose KEYS.

![Create SA6](/media/images/GC_Create_SA6.png)

12. Click ADD KEY and choose Create new key.

![Create SA7](/media/images/GC_Create_SA7.png)

13. Choose JSON for Key type and click CREATE. 


![Create SA8](/media/images/GC_Create_SA8.png)

14. Private key will be created for this service account and downloaded to your computer, click CLOSE. 


![Create SA9](/media/images/GC_Create_SA9.png)

In the following stepts (15-16) for saving the Service Account Credential in flash memory at compile time.

If you want to allow library to read the Service Account JSON key file directly at run time, skip these steps.

15. Open the .json file that is already downloaded with text editor.

```json
{
  "type": "service_account",
  "project_id": "...",
  "private_key_id": "...",
  "private_key": "-----BEGIN PRIVATE KEY-----\n...\n-----END PRIVATE KEY-----\n",
  "client_email": "...",
  "client_id": "...",
  "auth_uri": "https://accounts.google.com/o/oauth2/auth",
  "token_uri": "https://oauth2.googleapis.com/token",
  "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
  "client_x509_cert_url": "..."
}

```
16. Copy project_id, client_email, private_key_id and private_key from .json file and paste to these defines in the example.

```cpp
#define PROJECT_ID "..." //Taken from "project_id" key in JSON file.
#define CLIENT_EMAIL "..." //Taken from "client_email" key in JSON file.
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\n...\n-----END PRIVATE KEY-----\n"; //Taken from "private_key" key in JSON file.
```






## Installation


### Using Library Manager

At Arduino IDE, go to menu **Sketch** -> **Include Library** -> **Manage Libraries...**

In Library Manager Window, search **"forms"** in the search form then select **"ESP Google Forms Client"**. 

Click **"Install"** button.



For PlatformIO IDE, using the following command.

**pio lib install "ESP Google Forms Client""**

Or at **PIO Home** -> **Library** -> **Registry** then search **ESP Google Forms Client**.



### Manual installation

For Arduino IDE, download zip file from the repository (Github page) by select **Clone or download** dropdown at the top of repository, select **Download ZIP** 

From Arduino IDE, select menu **Sketch** -> **Include Library** -> **Add .ZIP Library...**.

Choose **ESP-Google-Forms-Client-master.zip** that previously downloaded.

Go to menu **Files** -> **Examples** -> **ESP-Google-Forms-Client-master** and choose one from examples.



## Usages


See [all examples](/examples) for complete usages.


```cpp

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <ESP_Google_Forms_Client.h>

#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define PROJECT_ID "PROJECT_ID"

// Service Account's client email
#define CLIENT_EMAIL "CLIENT_EMAIL"

// Your email to share access to form
#define USER_EMAIL "USER_EMAIL"

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

bool taskComplete = false;

// For Pico, WiFiMulti is recommended.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    // Set auto reconnect WiFi or network connection
#if defined(ESP32) || defined(ESP8266)
    WiFi.setAutoReconnect(true);
#endif

    // Connect to WiFi or network
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
        if (millis() - ms > 10000)
            break;
#endif
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();


    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    GForms.clearAP();
    GForms.addAP(WIFI_SSID, WIFI_PASSWORD);
    // You can add many WiFi credentials here
#endif

    //Begin the access token generation for Google API authentication
    GForms.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);

    // In case SD/SD_MMC storage file access, mount the SD/SD_MMC card.
    // SD_Card_Mounting(); // See src/GS_SDHelper.h

    // Or begin with the Service Account JSON file that uploaded to the Filesystem image or stored in SD memory card.
    // GForms.begin("path/to/serviceaccount/json/file", esp_google_forms_file_storage_type_flash /* or esp_google_forms_file_storage_type_sd */);
}


void loop()
{
    //Call ready() repeatedly in loop for authentication checking and processing
    bool ready = GForms.ready();

    if (ready && !taskComplete)
    {
        //For basic FirebaseJson usage example, see examples/FirebaseJson/Create_Edit_Parse/Create_Edit_Parse.ino

        //If you assign the form id from your own form,
        //you need to set share access to the Service Account's CLIENT_EMAIL

        bool success = GForms.createForm(&response, "My Form", "My Form", USER_EMAIL);
        if (success)
            response.toString(Serial, true);
        else
            Serial.println(GForms.errorReason());
        Serial.println();

        taskComplete = true;
    }
}

```




## IDE Configuaration for ESP8266 MMU - Adjust the Ratio of ICACHE to IRAM

### Arduino IDE

When you update the ESP8266 Arduino Core SDK to v3.0.0, the memory can be configurable from Arduino IDE board settings.

By default MMU **option 1** was selected, the free Heap can be low and may not suitable for the SSL client usage in this library.

To increase the Heap, choose the MMU **option 3**, 16KB cache + 48KB IRAM and 2nd Heap (shared).

![Arduino IDE config](/media/images/ArduinoIDE.png)

To use external Heap from 1 Mbit SRAM 23LC1024, choose the MMU **option 5**, 128K External 23LC1024.

![MMU VM 128K](/media/images/ESP8266_VM.png)

To use external Heap from PSRAM, choose the MMU **option 6**, 1M External 64 MBit PSRAM.

The connection between SRAM/PSRAM and ESP8266

```
23LC1024/ESP-PSRAM64                ESP8266

CS (Pin 1)                          GPIO15
SCK (Pin 6)                         GPIO14
MOSI (Pin 5)                        GPIO13
MISO (Pin 2)                        GPIO12
/HOLD (Pin 7 on 23LC1024 only)      3V3
Vcc (Pin 8)                         3V3
Vcc (Pin 4)                         GND
```


### PlatformIO IDE

By default the balanced ratio (32KB cache + 32KB IRAM) configuration is used.

To increase the heap, **PIO_FRAMEWORK_ARDUINO_MMU_CACHE16_IRAM48_SECHEAP_SHARED** build flag should be assigned in platformio.ini.

```ini
[env:d1_mini]
platform = espressif8266
build_flags = -D PIO_FRAMEWORK_ARDUINO_MMU_CACHE16_IRAM48_SECHEAP_SHARED
board = d1_mini
framework = arduino
monitor_speed = 115200
```

And to use external Heap from 1 Mbit SRAM 23LC1024 and 64 Mbit PSRAM, **PIO_FRAMEWORK_ARDUINO_MMU_EXTERNAL_128K** and **PIO_FRAMEWORK_ARDUINO_MMU_EXTERNAL_1024K** build flags should be assigned respectively.

The supportedd MMU build flags in PlatformIO.

- **PIO_FRAMEWORK_ARDUINO_MMU_CACHE16_IRAM48**

   16KB cache + 48KB IRAM (IRAM)

- **PIO_FRAMEWORK_ARDUINO_MMU_CACHE16_IRAM48_SECHEAP_SHARED**

   16KB cache + 48KB IRAM and 2nd Heap (shared)

- **PIO_FRAMEWORK_ARDUINO_MMU_CACHE16_IRAM32_SECHEAP_NOTSHARED**

   16KB cache + 32KB IRAM + 16KB 2nd Heap (not shared)

- **PIO_FRAMEWORK_ARDUINO_MMU_EXTERNAL_128K**

   128K External 23LC1024

- **PIO_FRAMEWORK_ARDUINO_MMU_EXTERNAL_1024K**

   1M External 64 MBit PSRAM

- **PIO_FRAMEWORK_ARDUINO_MMU_CUSTOM**

   Disables default configuration and expects user-specified flags


To use PSRAM/SRAM for internal memory allocation which you can config to use it via [**ESP_Google_Forms_Client_FS_Config.h**](src/ESP_Google_Forms_Client_FS_Config.h) with this macro.

```cpp
#define ESP_GOOGLE_FORMS_CLIENT_USE_PSRAM
```

   
### Test code for MMU

```cpp

#include <Arduino.h>
#include <umm_malloc/umm_heap_select.h>

void setup() 
{
  Serial.begin(115200);
  HeapSelectIram ephemeral;
  Serial.printf("IRAM free: %6d bytes\r\n", ESP.getFreeHeap());
  {
    HeapSelectDram ephemeral;
    Serial.printf("DRAM free: %6d bytes\r\n", ESP.getFreeHeap());
  }

  ESP.setExternalHeap();
  Serial.printf("External free: %d\n", ESP.getFreeHeap());
  ESP.resetHeap();
}

void loop() {
  // put your main code here, to run repeatedly:
}

```


### Use PSRAM on ESP32


To enable PSRAM in ESP32 module with on-board PSRAM chip, in Arduino IDE

![Enable PSRAM in ESP32](/media/images/ESP32-PSRAM.png)


In PlatformIO in VSCode IDE, add the following build_flags in your project's platformio.ini file

```ini
build_flags = -DBOARD_HAS_PSRAM -mfix-esp32-psram-cache-issue
```

*When config the IDE or add the build flags to use PSRAM in the ESP32 dev boards that do not have on-board PSRAM chip, your device will be crashed (reset).


To use PSRAM for internal memory allocation which you can config to use it via [**ESP_Google_Forms_Client_FS_Config.h**](src/ESP_Google_Forms_Client_FS_Config.h) with this macro.

```cpp
#define ESP_GOOGLE_FORMS_CLIENT_USE_PSRAM
```

## Functions Description

### Global Functions

#### Begin the Google API authentication. 

param **`client_email`** (string) The Service Account's client email.

param **`project_id`** (string) The project ID. 

param **`private_key`** (string) The Service Account's private key.

param **`eth`** (optional for ESP8266 only) The pointer to ESP8266 lwIP network class e.g. ENC28J60lwIP, Wiznet5100lwIP and Wiznet5500lwIP.

```cpp
void begin(<string> client_email, <string> project_id, <string> private_key, <ESP8266_spi_eth_module> *eth = nullptr);
```



#### Begin the Google API authentication. 

param **`service_account_file`** (string) The Service Account's JSON key file.

param **`storage_type`** (esp_google_forms_file_storage_type) The JSON key file storage type e.g. esp_google_forms_file_storage_type_flash and esp_google_forms_file_storage_type_sd.

param **`eth`** (optional for ESP8266 only) The pointer to ESP8266 lwIP network class e.g. ENC28J60lwIP, Wiznet5100lwIP and Wiznet5500lwIP.

```cpp
void begin(<string> service_account_file, esp_google_forms_file_storage_type storage_type, <ESP8266_spi_eth_module> *eth = nullptr);
```



#### Set the OAuth2.0 token generation status callback. 

param **`callback`** The callback function that accepts the TokenInfo as argument.
 
```cpp
void setTokenCallback(TokenStatusCallback callback);
```

#### Add the WiFi Access point credentials for connection resume (non-ESP device only).

param **`param`** ssid The access point ssid.

param **`param`** password The access point password.
```cpp
void addAP(T1 ssid, T2 password);
```

#### Clear all WiFi Access points credentials (non-ESP device only).

```cpp
void clearAP();
```

#### Assign external Arduino Client and required callback fumctions.

param **`client`** The pointer to Arduino Client derived class of SSL Client.

param **`networkConnectionCB`** The function that handles the network connection.

param **`networkStatusCB`** The function that handle the network connection status acknowledgement.

```cpp
void setExternalClient(Client *client, GS_NetworkConnectionRequestCallback networkConnectionCB,
                           GS_NetworkStatusRequestCallback networkStatusCB);
```

####  Assign UDP client and gmt offset for NTP time synching when using external SSL client

param **`client`** The pointer to UDP client based on the network type.

param **`gmtOffset`** The GMT time offset.

```cpp
void setUDPClient(UDP *client, float gmtOffset = 0);
```

####  Set the network status acknowledgement.

param **`status`** The network status.

```cpp
void setNetworkStatus(bool status);
```


####  Set the seconds to refesh auth token before it expires.

param **`seconds`** The seconds (60 sec to 3540 sec) that auth token will refresh before expired.

Default value is 300 seconds.

```cpp
void setPrerefreshSeconds(uint16_t seconds);
```



#### Set the Root certificate data for server authorization 

param **`ca`** PEM format certificate string.
 
```cpp
void setCert(const char *ca);
```


#### Set the Root certificate file for server authorization. 

param **`filename`** PEM format certificate file name included path.

param **`storageType`** The storage type of certificate file. esp_google_forms_file_storage_type_flash or esp_google_forms_file_storage_type_sd

```cpp
void setCertFile(<string> filename, esp_google_forms_file_storage_type storageType);
```


#### Get the authentication ready status and process the authentication. 

Note: This function should be called repeatedly in loop.

```cpp
bool ready();
```


#### Get the generated access token.

retuen **`String`** of OAuth2.0 access token.

```cpp
String accessToken();
```


#### Get the token type string.

param  **`info`** The TokenInfo structured data contains token info.

retuen **`String`** of token type.

```cpp
String getTokenType();

String getTokenType(TokenInfo info);
```


#### Get the token status string.

param  **`info`** The TokenInfo structured data contains token info.

retuen **`String`** of token status.

```cpp
String getTokenStatus();

String getTokenStatus(TokenInfo info);
```


#### Get the token generation error string.

param  **`info`** The TokenInfo structured data contains token info.

retuen **`String`** of token error.

```cpp
String getTokenError();

String getTokenError(TokenInfo info);
```


#### Get the token expiration timestamp (seconds from midnight Jan 1, 1970).

retuen **`unsigned long`** of timestamp.

```cpp
unsigned long getExpiredTimestamp();
```

#### Force the token to expire immediately and refresh.

```cpp
void refreshToken();
```


#### Reset stored config and auth credentials.

```cpp
void reset();
```

#### Initiate SD card with SPI port configuration.

param **`ss`** The SPI Chip/Slave Select pin.

param **`sck`** The SPI Clock pin.

param **`miso`** The SPI MISO pin.

param **`mosi`** The SPI MOSI pin.

aram **`frequency`** The SPI frequency.

return **`boolean`** The boolean value indicates the success of operation.

```cpp
bool sdBegin(int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, uint32_t frequency = 4000000);
```


#### Initiate SD card with SD FS configurations (ESP8266 only).

param **`ss`** SPI Chip/Slave Select pin.

param **`sdFSConfig`** The pointer to SDFSConfig object (ESP8266 only).

return **`boolean`** type status indicates the success of the operation.

```cpp
  bool sdBegin(SDFSConfig *sdFSConfig);
```


#### Initiate SD card with chip select and SPI configuration (ESP32 only).

param **`ss`** The SPI Chip/Slave Select pin.

param **`spiConfig`** The pointer to SPIClass object for SPI configuartion.

param **`frequency`** The SPI frequency.

return **`boolean`** The boolean value indicates the success of operation.

```cpp
bool sdBegin(int8_t ss, SPIClass *spiConfig = nullptr, uint32_t frequency = 4000000);
```


#### Initiate SD card with SdFat SPI and pins configurations (with SdFat included only).

param **`sdFatSPIConfig`** The pointer to SdSpiConfig object for SdFat SPI configuration.

param **`ss`** The SPI Chip/Slave Select pin.

param **`sck`** The SPI Clock pin.

param **`miso`** The SPI MISO pin.

param **`mosi`** The SPI MOSI pin.

return **`boolean`** The boolean value indicates the success of operation.

```cpp
 bool sdBegin(SdSpiConfig *sdFatSPIConfig, int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1);
```


#### Initiate SD card with SdFat SDIO configuration (with SdFat included only).

param **`sdFatSDIOConfig`** The pointer to SdioConfig object for SdFat SDIO configuration.

return **`boolean`** The boolean value indicates the success of operation.

```cpp
 bool sdBegin(SdioConfig *sdFatSDIOConfig);
```


#### Initialize the SD_MMC card (ESP32 only).

param **`mountpoint`** The mounting point.

param **`mode1bit`** Allow 1 bit data line (SPI mode).

param **`format_if_mount_failed`** Format SD_MMC card if mount failed.

return **`Boolean`** type status indicates the success of the operation.

```cpp
bool sdMMCBegin(const char *mountpoint = "/sdcard", bool mode1bit = false, bool format_if_mount_failed = false);
```


#### Create a new form using the title given in the provided form message in the request.

param **`response`** (FirebaseJson or String) The returned response.

param **`title`** (string) The title of the form which is visible to responders.

param **`docTitle`** (string) The title of the document which is visible in Drive.

param **`sharedUserEmail`** (string) Email of user to share the access. 

Note Google Drive API should be enabled at, https://console.cloud.google.com/apis/library/drive.googleapis.com

return **`Boolean`** type status indicates the success of the operation.

For ref doc go to https://developers.google.com/forms/api/reference/rest/v1/forms/create

```cpp
bool createForm(FirebaseJson *response, <string> title, <string> docTitle, <string> sharedUserEmail);
```

```cpp
bool createForm(String *response, <string> title, <string> docTitle, <string> sharedUserEmail);
```

#### Change the form with a batch of updates.

param **`response`** (FirebaseJson or String) The returned response.

param **`formId`** (string) The form ID.

param **`request`** (FirebaseJson of request object) The request body.

return **`Boolean`** type status indicates the success of the operation.

For ref doc go to https://developers.google.com/forms/api/reference/rest/v1/forms/batchUpdate

```cpp
bool batchUpdate(FirebaseJson *response, <string> formId, FirebaseJson *request);
```

```cpp
bool batchUpdate(String *response, <string> formId, FirebaseJson *request);
```



#### Get a form.

param **`response`** (FirebaseJson or String) The returned response.

param **`formId`** (string) The form ID.

return **`Boolean`** type status indicates the success of the operation.

For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1/forms/get
  
```cpp
bool getForm(FirebaseJson *response, <string> formId);
```

```cpp
bool getForm(String *response, <string> formId);
```


### Forms.responses functions


#### List a form's responses.

param **`response`** (FirebaseJson or String) The returned response.

param **`formId`** (string) The form ID.

param **`filter`** (string) Which form responses to return. Currently, the only supported filters are timestamp.

param **`pageSize`** (int) The maximum number of responses to return. 

param **`pageToken`** (string) A page token returned by a previous list response.

return **`Boolean`** type status indicates the success of the operation.

For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1/forms.responses/list

```cpp
bool listResponses(FirebaseJson *response, <string> formId, <string> filter = "", int pageSize = 0, <string> pageToken = "");
```

```cpp
bool listResponses(String *response, <string> formId, <string> filter = "", int pageSize = 0, <string> pageToken = "");
```



#### Get the list(array) of response ID in a form.

param **`formId`** (string) The form ID.

param **`responseId`** (string array) The array of String that contains the response ID in a form.

return **`Boolean`** type status indicates the success of the operation.

Note The response may be empty.

```cpp
bool getResponseIDList(<string> formId, std::vector<String> &responseId)
```



#### Get one response from the form.

param **`response`** (FirebaseJson or String) The returned response.

param **`formId`** (string) The form ID.

param **`responseId`** (string) The form's response ID.

return **`Boolean`** type status indicates the success of the operation.

For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.responses/get

```cpp
bool getResponse(FirebaseJson *response, <string> formId, <string> responseId);
```

```cpp
bool getResponse(String *response, <string> formId, <string> responseId);
```

### Forms.watches functions


#### Create a new watch.

param **`response`** (FirebaseJson or String) The returned response.

param **`formId`** (string) The form ID.

param **`request`** (FirebaseJson of request object) The request body.

return **`Boolean`** type status indicates the success of the operation.

For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/create

To receive notifications, the topic must grant publish privileges to the Forms service account, forms-notifications@system.gserviceaccount.com. 

Only the project that owns a topic may create a watch with it.

Note: The watch using service account credentials is currently not working. 
You may get the error "Project does not have necessary OAuth permissions from authorizing user.", 
see https://issuetracker.google.com/issues/242295786?pli=1

```cpp
bool createWatch(FirebaseJson *response, <string> formId, FirebaseJson *request);
```

```cpp
bool createWatch(String *response, <string> formId, FirebaseJson *request);
```


#### Get the list(array) of watch ID in a form.

param **`formId`** (string) The form ID.

param **`responseId`** (string array) The array of String that contains the response ID in a form.

return **`Boolean`** type status indicates the success of the operation.


```cpp
bool getWatchIDList(<string> formId, std::vector<String> &watchId);
```



#### Delete a watch.

param **`response`** (FirebaseJson or String) The returned response.

param **`formId`** (string) The form ID.

param **`watchId`** (string) The watch ID.

return **`Boolean`** type status indicates the success of the operation.


For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/delete


```cpp
bool deleteWatch(FirebaseJson *response, <string> formId, <string> watchId);
```

```cpp
bool deleteWatch(String *response, <string> formId, <string> watchId);
```



#### Return a list of the watches owned by the invoking project.

param **`response`** (FirebaseJson or String) The returned response.

param **`formId`** (string) The form ID.

return **`Boolean`** type status indicates the success of the operation.


For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/list


```cpp
bool listWatches(FirebaseJson *response, <string> formId);
```

```cpp
bool listWatches(String *response, <string> formId);
```


#### Renew an existing watch for seven days.

param **`response`** (FirebaseJson or String) The returned response.

param **`formId`** (string) The form ID.

param **`watchId`** (string) The watch ID.

return **`Boolean`** type status indicates the success of the operation.


For ref doc go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/renew


```cpp
bool renewWatch(FirebaseJson *response, <string> formId, <string> watchId);
```

```cpp
bool renewWatch(String *response, <string> formId, <string> watchId);
```



## License

The MIT License (MIT)

Copyright (C) 2023 K. Suwatchai (Mobizt)


Permission is hereby granted, free of charge, to any person returning a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.



