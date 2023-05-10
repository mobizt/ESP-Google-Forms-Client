
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: suwatchai@outlook.com
 *
 * Github: https://github.com/mobizt
 *
 * Copyright (c) 2023 mobizt
 *
 */

// This example shows how to create new form, add new items and update some item.

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ESP_Google_Forms_Client.h>

// For SD/SD_MMC mounting helper
#include <GForms_SDHelper.h>

#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// For how to create Service Account and how to use the library, go to https://github.com/mobizt/ESP-Google-Forms-Client

#define PROJECT_ID "PROJECT_ID"

// Service Account's client email
#define CLIENT_EMAIL "CLIENT_EMAIL"

// Your email to share access to form
#define USER_EMAIL "USER_EMAIL"

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

/**
const char rootCACert[] PROGMEM = "-----BEGIN CERTIFICATE-----\n"
                                  "MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw\n"
                                  "CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n"
                                  "MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAw\n"
                                  "MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n"
                                  "Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA\n"
                                  "A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo\n"
                                  "27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w\n"
                                  "Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7zUjw\n"
                                  "TcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0Pfybl\n"
                                  "qAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtcvfaH\n"
                                  "szVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4Zor8\n"
                                  "Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUspzBmk\n"
                                  "MiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOORc92\n"
                                  "wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYWk70p\n"
                                  "aDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+DVrN\n"
                                  "VjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgFlQID\n"
                                  "AQABo0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E\n"
                                  "FgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBAJ+qQibb\n"
                                  "C5u+/x6Wki4+omVKapi6Ist9wTrYggoGxval3sBOh2Z5ofmmWJyq+bXmYOfg6LEe\n"
                                  "QkEzCzc9zolwFcq1JKjPa7XSQCGYzyI0zzvFIoTgxQ6KfF2I5DUkzps+GlQebtuy\n"
                                  "h6f88/qBVRRiClmpIgUxPoLW7ttXNLwzldMXG+gnoot7TiYaelpkttGsN/H9oPM4\n"
                                  "7HLwEXWdyzRSjeZ2axfG34arJ45JK3VmgRAhpuo+9K4l/3wV3s6MJT/KYnAK9y8J\n"
                                  "ZgfIPxz88NtFMN9iiMG1D53Dn0reWVlHxYciNuaCp+0KueIHoI17eko8cdLiA6Ef\n"
                                  "MgfdG+RCzgwARWGAtQsgWSl4vflVy2PFPEz0tv/bal8xa5meLMFrUKTX5hgUvYU/\n"
                                  "Z6tGn6D/Qqc6f1zLXbBwHSs09dR2CQzreExZBfMzQsNhFRAbd03OIozUhfJFfbdT\n"
                                  "6u9AWpQKXCBfTkBdYiJ23//OYb2MI3jSNwLgjt7RETeJ9r/tSQdirpLsQBqvFAnZ\n"
                                  "0E6yove+7u7Y/9waLd64NnHi/Hm3lCXRSHNboTXns5lndcEZOitHTtNCjv0xyBZm\n"
                                  "2tIMPNuzjsmhDYAPexZ3FL//2wmUspO8IFgV6dtxQ/PeEMMA3KgqlbbC1j+Qa3bb\n"
                                  "bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c\n"
                                  "-----END CERTIFICATE-----\n";
*/

bool taskComplete = false;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

void tokenStatusCallback(TokenInfo info);

void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    Serial.printf("ESP Google Forms Client v%s\n\n", ESP_GOOGLE_FORMS_CLIENT_VERSION);

#if defined(ESP32) || defined(ESP8266)
    WiFi.setAutoReconnect(true);
#endif

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

    // In case SD/SD_MMC storage file access, mount the SD/SD_MMC card.
    // SD_Card_Mounting(); // See src/GS_SDHelper.h

    // GForms.setCert(rootCACert); // or GForms.setCertFile("path/to/certificate/file.pem", esp_google_forms_file_storage_type_flash /* or esp_google_forms_file_storage_type_sd */);

    // Set the callback for Google API access token generation status (for debug only)
    GForms.setTokenCallback(tokenStatusCallback);

    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    GForms.clearAP();
    GForms.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

    // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
    GForms.setPrerefreshSeconds(10 * 60);

    // Begin the access token generation for Google API authentication
    GForms.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);

    // Or begin with the Service Account JSON file
    // GForms.begin("path/to/serviceaccount/json/file", esp_google_forms_file_storage_type_flash /* or esp_google_forms_file_storage_type_sd */);
}

void loop()
{
    // Call ready() repeatedly in loop for authentication checking and processing
    bool ready = GForms.ready();

    if (ready && !taskComplete)
    {

        // For basic FirebaseJson usage example, see examples/FirebaseJson/Create_Edit_Parse/Create_Edit_Parse.ino

        // If you assign the form ID from your own Google Forms,
        // you need to set share access to the Service Account's CLIENT_EMAIL

        FirebaseJson response;
        // Instead of using FirebaseJson for response, you can use String for response to the functions
        // especially in low memory device that deserializing large JSON response may be failed as in ESP8266

        Serial.println("\nCreate new form...");
        Serial.println("----------------------------");

        String formId, responderUri;

        bool success = GForms.createForm(&response, "Test Form", "", USER_EMAIL);
        if (success)
            response.toString(Serial, true);
        else
            Serial.println(GForms.errorReason());
        Serial.println();

        if (success)
        {
            // Get the form id from already created file.
            FirebaseJsonData result;
            response.get(result, FPSTR("formId")); // parse or deserialize the JSON response
            if (result.success)
                formId = result.to<const char *>();

            // Get the responder Uri.
            result.clear();
            response.get(result, FPSTR("responderUri")); // parse or deserialize the JSON response
            if (result.success)
            {
                responderUri = result.to<const char *>();
                Serial.println("\nThe responder Uri");
                Serial.println(responderUri);
            }

            Serial.println("\nCteate and update form items...");
            Serial.println("------------------------------");

            FirebaseJson request;

            request.add("includeFormInResponse", false);

            // create image item
            request.set("requests/[0]/createItem/item/title", "Simple image Question");
            request.set("requests/[0]/createItem/item/description", "This is image");
            request.set("requests/[0]/createItem/item/itemId", "001");
            request.set("requests/[0]/createItem/item/imageItem/image/altText", "simple image");
            request.set("requests/[0]/createItem/item/imageItem/image/properties/alignment", "LEFT");
            request.set("requests/[0]/createItem/item/imageItem/image/properties/width", 100);
            request.set("requests/[0]/createItem/item/imageItem/image/sourceUri", "https://lh3.googleusercontent.com/Fbyy_RusWO3Yd4HrMFiXOLnQTukW4LhHiCUACpJOMMaasonbLX1QE3PgFjFvD7mkhRdTe2HXNCGf-dENKmQ");
            request.set("requests/[0]/createItem/location/index", 0);

            // create paragraph text item
            request.set("requests/[1]/createItem/item/title", "Simple Text Question");
            request.set("requests/[1]/createItem/item/description", "This is question text");
            request.set("requests/[1]/createItem/item/itemId", "002");
            request.set("requests/[1]/createItem/item/questionItem/question/textQuestion/paragraph", true);
            request.set("requests/[1]/createItem/item/questionItem/question/required", true);
            request.set("requests/[1]/createItem/location/index", 1);

            // create options
            request.set("requests/[2]/createItem/item/title", "Simple Optoions Question");
            request.set("requests/[2]/createItem/item/description", "This is question options");
            request.set("requests/[2]/createItem/item/itemId", "003");
            request.set("requests/[2]/createItem/item/questionItem/question/choiceQuestion/type", "RADIO");
            request.set("requests/[2]/createItem/item/questionItem/question/choiceQuestion/options/[0]/goToAction", "RESTART_FORM");
            request.set("requests/[2]/createItem/item/questionItem/question/choiceQuestion/options/[0]/value", "Back to Top");
            request.set("requests/[2]/createItem/item/questionItem/question/choiceQuestion/options/[1]/goToAction", "NEXT_SECTION");
            request.set("requests/[2]/createItem/item/questionItem/question/choiceQuestion/options/[1]/value", "Go Next");
            request.set("requests/[2]/createItem/item/questionItem/question/required", true);
            request.set("requests/[2]/createItem/location/index", 2);

            // create date/time
            request.set("requests/[3]/createItem/item/title", "Simple Date/Time Question");
            request.set("requests/[3]/createItem/item/description", "This is date question");
            request.set("requests/[3]/createItem/item/itemId", "004");
            request.set("requests/[3]/createItem/item/questionItem/question/dateQuestion/includeTime", true);
            request.set("requests/[3]/createItem/item/questionItem/question/dateQuestion/includeYear", true);
            request.set("requests/[3]/createItem/item/questionItem/question/required", true);
            request.set("requests/[3]/createItem/location/index", 3);

            // update some fields of image item with updateMask
            request.set("requests/[4]/updateItem/item/title", "Only some image fields updated");
            request.set("requests/[4]/updateItem/item/description", "New description");
            request.set("requests/[4]/updateItem/item/itemId", "100");
            request.set("requests/[4]/updateItem/item/imageItem/image/altText", "new text");
            request.set("requests/[4]/updateItem/item/imageItem/image/sourceUri", "https://cdn-icons-png.flaticon.com/512/217/217820.png");
            request.set("requests/[4]/updateItem/updateMask", "title,description,itemId,imageItem.image.altText,imageItem.image.sourceUri");
            request.set("requests/[4]/updateItem/location/index", 0);

            // update form info
            request.set("requests/[5]/updateFormInfo/info/title", "New Form Title");
            request.set("requests/[5]/updateFormInfo/info/description", "New form description");
            request.set("requests/[5]/updateFormInfo/updateMask", "title,description");

            // move item at index 0 to index 1
            request.set("requests/[6]/moveItem/originalLocation/index", 0);
            request.set("requests/[6]/moveItem/newLocation/index", 1);

            // update settings
            request.set("requests/[7]/updateSettings/settings/quizSettings/isQuiz", true);
            request.set("requests/[7]/updateSettings/updateMask", "*");

            // delete item at index 1
            // request.set("requests/[8]/deleteItem/location/index", 1);

            // For Google Forms API ref doc, go to https://developers.google.com/forms/api/reference/rest/v1/forms/batchUpdate

            success = GForms.batchUpdate(&response /* returned response */, formId /* form Id to update */, &request /* request object */);
            response.toString(Serial, true);
            Serial.println();
        }

#if defined(ESP32) || defined(ESP8266)
        Serial.println(ESP.getFreeHeap());
#elif defined(PICO_RP2040)
        Serial.println(rp2040.getFreeHeap());
#endif

        taskComplete = true;
    }
}

void tokenStatusCallback(TokenInfo info)
{
    if (info.status == esp_signer_token_status_error)
    {
        Serial.printf("Token info: type = %s, status = %s\n", GForms.getTokenType(info).c_str(), GForms.getTokenStatus(info).c_str());
        Serial.printf("Token error: %s\n", GForms.getTokenError(info).c_str());
    }
    else
    {
        Serial.printf("Token info: type = %s, status = %s\n", GForms.getTokenType(info).c_str(), GForms.getTokenStatus(info).c_str());
    }
}