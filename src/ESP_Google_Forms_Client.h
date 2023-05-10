#ifndef ESP_GOOGLE_FORMS_CLIENT_VERSION
#define ESP_GOOGLE_FORMS_CLIENT_VERSION "1.0.0"
#endif

/**
 * Google Forms Client, ESP_Google_Forms_Client.h v1.0.0
 *
 * This library supports Espressif ESP8266 and ESP32 MCUs
 *
 * Created May 9, 2023
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
 *
 *
 * Permission is hereby granted, free of charge, to any person returning a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include "mbfs/MB_MCU.h"

#ifndef ESP_Google_FORMS_Client_H
#define ESP_Google_FORMS_Client_H

#include "auth/GFormsAuthManager.h"

class GFormsClass
{
    friend class ESP_Google_Forms_Client;

public:
    GFormsClass();
    ~GFormsClass();

private:
    enum operation_type_t
    {
        operation_type_undefined,
        operation_type_range,
        operation_type_batch,
        operation_type_filter,
    };

    enum host_type_t
    {
        host_type_forms,
        host_type_drive
    };

    gauth_cfg_t config;
    GAuthManager authMan;
    MB_FS mbfs;
    uint32_t mb_ts = 0;
    uint32_t mb_ts_offset = 0;
    int response_code = 0;

    int cert_addr = 0;
    bool cert_updated = false;

    void auth(const char *client_email, const char *project_id, const char *private_key, const char *sa_key_file, esp_google_forms_file_storage_type storage_type, ESP8266_SPI_ETH_MODULE *eth = nullptr);
    void setTokenCallback(TokenStatusCallback callback);
    void addAP(const char *ssid, const char *password);
    void clearAP();
    bool checkToken();
    String accessToken();
    void setPrerefreshSeconds(uint16_t seconds);
    bool isError(MB_String &response);

    bool beginRequest(MB_String &req, host_type_t host_type);
    void addHeader(MB_String &req, host_type_t host_type, int len = -1);
    bool processRequest(MB_String &req, MB_String &response, int &httpcode, const char *key = "");
    bool create(MB_String &response, const char *title, const char *docTitle = "");
    bool createPermission(MB_String &response, const char *fileId, const char *role, const char *type, const char *email);
    bool batchUpdate(MB_String &response, const char *formId, FirebaseJson *request);
    bool getForm(MB_String &response, const char *formId);
    bool listResponses(MB_String &response, const char *formId, const char *key = "", const char *filter = "", int pageSize = 0, const char *pageToken = "");
    bool getResponse(MB_String &response, const char *formId, const char *responseId);
    bool createWatch(MB_String &response, const char *formId, FirebaseJson *request);
    bool listWatch(MB_String &response, const char *formId, const char *key);
    bool deleteWatch(MB_String &response, const char *formId, const char *watchId);
    bool listWatches(MB_String &response, const char *formId);
    bool renewWatch(MB_String &response, const char *formId, const char *watchId);

    bool setClock(float gmtOffset);
#if defined(ESP_GOOGLE_FORMS_CLIENT_ENABLE_EXTERNAL_CLIENT)
    void setClient(Client *client, GFORMS_NetworkConnectionRequestCallback networkConnectionCB,
                   GFORMS_NetworkStatusRequestCallback networkStatusCB);
    void setUDPClient(UDP *client, float gmtOffset = 0);
#endif
    bool setSecure();
    void setCert(const char *ca);
    void setCertFile(const char *filename, esp_google_forms_file_storage_type type);
    void reset();
    bool waitClockReady();
};

class ESP_Google_Forms_Client
{

public:
    ESP_Google_Forms_Client()
    {
        gforms = new GFormsClass();
    };

    ~ESP_Google_Forms_Client()
    {
        if (gforms)
            delete gforms;
    };

    /** Begin the Google API authentication.
     *
     * @param client_email (string) The Service Account's client email.
     * @param project_id (string) The project ID.
     * @param private_key (string) The Service Account's private key.
     * @param eth (optional for ESP8266 only) The pointer to ESP8266 lwIP network class
     * e.g. ENC28J60lwIP, Wiznet5100lwIP and Wiznet5500lwIP.
     *
     */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
    void begin(T1 client_email, T2 project_id, T3 private_key, ESP8266_SPI_ETH_MODULE *eth = nullptr)
    {
        gforms->auth(toString(client_email), toString(project_id), toString(private_key), "", esp_google_forms_file_storage_type_undefined, eth);
    }

    /** Begin the Google API authentication.
     *
     * @param service_account_file (string) The Service Account's JSON key file.
     * @param storage_type (esp_google_forms_file_storage_type) The JSON key file storage type e.g. esp_google_forms_file_storage_type_flash and esp_google_forms_file_storage_type_sd.
     * @param eth (optional for ESP8266 only) The pointer to ESP8266 lwIP network class
     * e.g. ENC28J60lwIP, Wiznet5100lwIP and Wiznet5500lwIP.
     */
    template <typename T1 = const char *>
    void begin(T1 service_account_file, esp_google_forms_file_storage_type storage_type, ESP8266_SPI_ETH_MODULE *eth = nullptr)
    {
        gforms->auth("", "", "", toString(service_account_file), storage_type, eth);
    }

    /** Set the Root certificate data for server authorization.
     * @param ca PEM format certificate string.
     */
    void setCert(const char *ca) { gforms->setCert(ca); }

    /** Set the Root certificate file for server authorization.
     * @param filename PEM format certificate file name incuded path.
     * @param storageType The storage type of certificate file. esp_google_forms_file_storage_type_flash or esp_google_forms_file_storage_type_sd
     */
    template <typename T = const char *>
    void setCertFile(T filename, esp_google_forms_file_storage_type storageType) { gforms->setCertFile(toString(filename), storageType); }

    /** Set the OAuth2.0 token generation status callback.
     *
     * @param callback The callback function that accepts the TokenInfo as argument.
     *
     */
    void setTokenCallback(TokenStatusCallback callback)
    {
        gforms->setTokenCallback(callback);
    }

    /** Add the WiFi Access point credentials for connection resume (non-ESP device only).
     *
     * @param ssid The access point ssid.
     * @param password The access point password.
     *
     */
    template <typename T1 = const char *, typename T2 = const char *>
    void addAP(T1 ssid, T2 password)
    {
        gforms->addAP(toString(ssid), toString(password));
    }

    /** Clear all WiFi Access points credentials (non-ESP device only).
     *
     */
    void clearAP()
    {
        gforms->clearAP();
    }

    /** Assign external Arduino Client and required callback fumctions.
     *
     * @param client The pointer to Arduino Client derived class of SSL Client.
     * @param networkConnectionCB The function that handles the network connection.
     * @param networkStatusCB The function that handle the network connection status acknowledgement.
     */
    void setExternalClient(Client *client, GFORMS_NetworkConnectionRequestCallback networkConnectionCB,
                           GFORMS_NetworkStatusRequestCallback networkStatusCB)
    {
#if defined(ESP_GOOGLE_FORMS_CLIENT_ENABLE_EXTERNAL_CLIENT)
        gforms->setClient(client, networkConnectionCB, networkStatusCB);
#endif
    }

    /** Assign UDP client and gmt offset for NTP time synching when using external SSL client
     * @param client The pointer to UDP client based on the network type.
     * @param gmtOffset The GMT time offset.
     */
    void setUDPClient(UDP *client, float gmtOffset = 0)
    {
#if defined(ESP_GOOGLE_FORMS_CLIENT_ENABLE_EXTERNAL_CLIENT)
        gforms->setUDPClient(client, gmtOffset);
#endif
    }

    /** Set the network status acknowledgement.
     *
     * @param status The network status.
     */
    void setNetworkStatus(bool status)
    {
        gforms->authMan.tcpClient->setNetworkStatus(status);
    }

    /** Get the authentication ready status and process the authentication.
     *
     * @note This function should be called repeatedly in loop.
     *
     */
    bool ready()
    {
        return gforms->checkToken();
    }

    /**
     * Get the generated access token.
     *
     * @return String of OAuth2.0 access token.
     *
     */
    String accessToken() { return gforms->accessToken(); }

    /** Set the seconds to refesh auth token before it expires.
     *
     * @param seconds The seconds (60 sec to 3540 sec) that auth token will refresh before expired.
     * Default value is 300 seconds.
     *
     */
    void setPrerefreshSeconds(uint16_t seconds)
    {
        gforms->setPrerefreshSeconds(seconds);
    }

    /**
     * Get the token type string.
     *
     * @param info The TokenInfo structured data contains token info.
     * @return token type String.
     *
     */
    String getTokenType() { return gforms->authMan.getTokenType(); }
    String getTokenType(TokenInfo info) { return gforms->authMan.getTokenType(info); }

    /**
     * Get the token status string.
     *
     * @param info The TokenInfo structured data contains token info.
     * @return token status String.
     *
     */
    String getTokenStatus() { return gforms->authMan.getTokenStatus(); }
    String getTokenStatus(TokenInfo info) { return gforms->authMan.getTokenStatus(info); }

    /**
     * Get the token generation error string.
     *
     * @param info The TokenInfo structured data contains token info.
     * @return token generation error String.
     *
     */
    String getTokenError() { return gforms->authMan.getTokenError(); }
    String getTokenError(TokenInfo info) { return gforms->authMan.getTokenError(info); }

    /**
     * Get the token expiration timestamp (seconds from midnight Jan 1, 1970).
     *
     * @return timestamp.
     *
     */
    unsigned long getExpiredTimestamp() { return gforms->authMan.getExpiredTimestamp(); }

    /** Force the token to expire immediately and refresh.
     */
    void refreshToken() { gforms->authMan.refresh(); };

    /** Reset stored config and auth credentials.
     *
     */
    void reset() { gforms->authMan.reset(); };

    /**
     * Get error reason from last operation.
     *
     * @return error String.
     *
     */
    String errorReason()
    {
        MB_String buf;
        gforms->authMan.errorToString(gforms->authMan.response_code, buf);
        return buf.c_str();
    }

    /** Create a new form using the title given in the provided form message in the request.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param title (string) The title of the form which is visible to responders.
     * @param docTitle (string) The title of the document which is visible in Drive.
     * @param sharedUserEmail (string) Email of user to share the access.
     *
     * @note Drive API should be enabled at, https://console.cloud.google.com/apis/library/drive.googleapis.com
     *
     * @return Boolean type status indicates the success of the operation.
     *
     * For ref doc go to https://developers.google.com/forms/api/reference/rest/v1/forms/create
     *
     */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
    bool createForm(FirebaseJson *response, T1 title, T2 docTitle, T3 sharedUserEmail)
    {
        MB_String _response;

        bool ret = gforms->create(_response, toString(title), toString(docTitle));

        if (ret)
        {
            ret = !gforms->isError(_response);
            MB_String formId = gforms->authMan.getValue(_response, (const char *)FPSTR("formId"));
            if (formId.length() > 0)
            {
                MB_String res;
                ret = gforms->createPermission(res, formId.c_str(), (const char *)FPSTR("writer"), (const char *)FPSTR("user"), sharedUserEmail);
            }
        }

        response->setJsonData(_response);

        return ret;
    }

    /** Create a new form using the title given in the provided form message in the request.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param title (string) The title of the form which is visible to responders.
     * @param docTitle (string) The title of the document which is visible in Drive.
     * @param sharedUserEmail (string) Email of user to share the access.
     *
     * @note Drive API should be enabled at, https://console.cloud.google.com/apis/library/drive.googleapis.com
     *
     * @return Boolean type status indicates the success of the operation.
     *
     * For ref doc go to https://developers.google.com/forms/api/reference/rest/v1/forms/create
     *
     */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
    bool createForm(String *response, T1 title, T2 docTitle, T3 sharedUserEmail)
    {
        MB_String _response;

        bool ret = gforms->create(_response, toString(title), toString(docTitle));

        if (ret)
        {
            ret = !gforms->isError(_response);
            MB_String formId = gforms->authMan.getValue(_response, (const char *)FPSTR("formId"));
            if (formId.length() > 0)
            {
                MB_String res;
                ret = gforms->createPermission(res, formId.c_str(), (const char *)FPSTR("writer"), (const char *)FPSTR("user"), sharedUserEmail);
            }
        }

        *response = _response.c_str();

        return ret;
    }

    /** Change the form with a batch of updates.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param request (FirebaseJson of request object) The request body.
     *
     * @return Boolean type status indicates the success of the operation.
     *
     * For ref doc go to https://developers.google.com/forms/api/reference/rest/v1/forms/batchUpdate
     *
     */
    template <typename T = const char *>
    bool batchUpdate(FirebaseJson *response, T formId, FirebaseJson *request)
    {
        MB_String _response;

        bool ret = gforms->batchUpdate(_response, toString(formId), request);

        if (ret)
            ret = !gforms->isError(_response);

        response->setJsonData(_response);

        return ret;
    }

    /** Change the form with a batch of updates.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param request (FirebaseJson of request object) The request body.
     *
     * @return Boolean type status indicates the success of the operation.
     *
     * For ref doc go to https://developers.google.com/forms/api/reference/rest/v1/forms/batchUpdate
     *
     */
    template <typename T = const char *>
    bool batchUpdate(String *response, T formId, FirebaseJson *request)
    {
        MB_String _response;

        bool ret = gforms->batchUpdate(_response, toString(formId), request);

        if (ret)
            ret = !gforms->isError(_response);

        *response = _response.c_str();

        return ret;
    }

    /** Get a form.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     *
     * @return Boolean type status indicates the success of the operation.
     *
     * For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1/forms/get
     *
     */
    template <typename T = const char *>
    bool getForm(FirebaseJson *response, T formId)
    {
        MB_String _response;

        bool ret = gforms->getForm(_response, toString(formId));

        if (ret)
            ret = !gforms->isError(_response);

        response->setJsonData(_response);

        return ret;
    }

    /** Get a form.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     *
     * @return Boolean type status indicates the success of the operation.
     *
     * For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1/forms/get
     *
     */
    template <typename T = const char *>
    bool getForm(String *response, T formId)
    {
        MB_String _response;

        bool ret = gforms->getForm(_response, toString(formId));

        if (ret)
            ret = !gforms->isError(_response);

        *response = _response.c_str();

        return ret;
    }

    /** List a form's responses.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param filter (string) Which form responses to return. Currently, the only supported filters are timestamp.
     * @param pageSize (int) The maximum number of responses to return.
     * @param pageToken (string) A page token returned by a previous list response.
     *
     * @return Boolean type status indicates the success of the operation.
     *
     * For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1/forms.responses/list
     *
     */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
    bool listResponses(FirebaseJson *response, T1 formId, T2 filter = "", int pageSize = 0, T3 pageToken = "")
    {
        MB_String _response;

        bool ret = gforms->listResponses(_response, toString(formId), "", toString(filter), pageSize, toString(pageToken));

        if (ret)
            ret = !gforms->isError(_response);

        response->setJsonData(_response);

        return ret;
    }

    /** List a form's responses.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param filter (string) Which form responses to return. Currently, the only supported filters are timestamp.
     * @param pageSize (int) The maximum number of responses to return.
     * @param pageToken (string) A page token returned by a previous list response.
     * @return Boolean type status indicates the success of the operation.
     *
     * For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1/forms.responses/list
     *
     */
    template <typename T1 = const char *, typename T2 = const char *, typename T3 = const char *>
    bool listResponses(String *response, T1 formId, T2 filter = "", int pageSize = 0, T3 pageToken = "")
    {
        MB_String _response;

        bool ret = gforms->listResponses(_response, toString(formId), "", toString(filter), pageSize, toString(pageToken));

        if (ret)
            ret = !gforms->isError(_response);

        *response = _response.c_str();

        return ret;
    }

    /** Get the list(array) of response ID in a form.
     *
     * @param formId (string) The form ID.
     * @param responseId (string array) The array of String that contains the response ID in a form.
     * @return Boolean type status indicates the success of the operation.
     *
     */
    template <typename T = const char *>
    bool getResponseIDList(T formId, std::vector<String> &responseId)
    {
        MB_String _response;

        bool ret = gforms->listResponses(_response, toString(formId), toString(FPSTR("responseId")));

        if (ret)
            ret = !gforms->isError(_response);

        if (ret && _response.length() > 0)
            StringHelper::splitString(_response, responseId, ',');

        return ret;
    }

    /** Get one response from the form.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param responseId (string) The form's response ID.
     *
     * @return Boolean type status indicates the success of the operation.
     *
     * For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.responses/get
     *
     */
    template <typename T1 = const char *, typename T2 = const char *>
    bool getResponse(FirebaseJson *response, T1 formId, T2 responseId)
    {
        MB_String _response;

        bool ret = gforms->getResponse(_response, toString(formId), toString(responseId));

        if (ret)
            ret = !gforms->isError(_response);

        response->setJsonData(_response);

        return ret;
    }

    /** Get one response from the form.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param responseId (string) The form's response ID.
     *
     * @return Boolean type status indicates the success of the operation.
     *
     * For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.responses/get
     *
     */
    template <typename T1 = const char *, typename T2 = const char *>
    bool getResponse(String *response, T1 formId, T2 responseId)
    {
        MB_String _response;

        bool ret = gforms->getResponse(_response, toString(formId), toString(responseId));

        if (ret)
            ret = !gforms->isError(_response);

        *response = _response.c_str();

        return ret;
    }

    /** Create a new watch.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param request (FirebaseJson of request object) The request body.
     * @return Boolean type status indicates the success of the operation.
     * 
     * For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/create
     *
     * @note The watch using service account credentials is currently not working.
     * You may get the error "Project does not have necessary OAuth permissions from authorizing user.",
     * see https://issuetracker.google.com/issues/242295786?pli=1
     *
     */
    template <typename T = const char *>
    bool createWatch(FirebaseJson *response, T formId, FirebaseJson *request)
    {
        MB_String _response;

        bool ret = gforms->createWatch(_response, toString(formId), request);

        if (ret)
            ret = !gforms->isError(_response);

        response->setJsonData(_response);

        return ret;
    }

    /** Create a new watch.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param request (FirebaseJson of request object) The request body.
     * @return Boolean type status indicates the success of the operation.
     * 
     * For ref doc, go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/create
     *
     * @note The watch using service account credentials is currently not working.
     * You may get the error "Project does not have necessary OAuth permissions from authorizing user.",
     * see https://issuetracker.google.com/issues/242295786?pli=1
     *
     */
    template <typename T = const char *>
    bool createWatch(String *response, T formId, FirebaseJson *request)
    {
        MB_String _response;

        bool ret = gforms->createWatch(_response, toString(formId), request);

        if (ret)
            ret = !gforms->isError(_response);

        *response = _response.c_str();

        return ret;
    }

    /** Get the list(array) of watch ID in a form.
     *
     * @param formId (string) The form ID.
     * @param watchId (string array) The array of String that contains the watch IDs.
     * @return Boolean type status indicates the success of the operation.
     *
     */
    template <typename T = const char *>
    bool getWatchIDList(T formId, std::vector<String> &watchId)
    {
        MB_String _response;

        bool ret = gforms->listWatch(_response, toString(formId), toString(FPSTR("id")));

        if (ret)
            ret = !gforms->isError(_response);

        if (ret && _response.length() > 0)
            StringHelper::splitString(_response, watchId, ',');

        return ret;
    }

    /** Delete a watch.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param watchId (string) The watch ID.
     * @return Boolean type status indicates the success of the operation.
     * 
     * For ref doc go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/delete
     *
     */
    template <typename T1 = const char *, typename T2 = const char *>
    bool deleteWatch(FirebaseJson *response, T1 formId, T2 watchId)
    {
        MB_String _response;

        bool ret = gforms->deleteWatch(_response, toString(formId), toString(watchId));

        if (ret)
            ret = !gforms->isError(_response);

        response->setJsonData(_response);

        return ret;
    }

    /** Delete a watch.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param watchId (string) The watch ID.
     * @return Boolean type status indicates the success of the operation.
     * 
     * For ref doc go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/delete
     *
     */
    template <typename T1 = const char *, typename T2 = const char *>
    bool deleteWatch(String *response, T1 formId, T2 watchId)
    {
        MB_String _response;

        bool ret = gforms->deleteWatch(_response, toString(formId), toString(watchId));

        if (ret)
            ret = !gforms->isError(_response);

        *response = _response.c_str();

        return ret;
    }

    /** Return a list of the watches owned by the invoking project.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @return Boolean type status indicates the success of the operation.
     * 
     * For ref doc go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/list
     *
     */
    template <typename T = const char *>
    bool listWatches(FirebaseJson *response, T formId)
    {
        MB_String _response;

        bool ret = gforms->listWatches(_response, toString(formId));

        if (ret)
            ret = !gforms->isError(_response);

        response->setJsonData(_response);

        return ret;
    }

    /** Return a list of the watches owned by the invoking project.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @return Boolean type status indicates the success of the operation.
     * 
     * For ref doc go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/list
     *
     */
    template <typename T = const char *>
    bool listWatches(String *response, T formId)
    {
        MB_String _response;

        bool ret = gforms->listWatches(_response, toString(formId));

        if (ret)
            ret = !gforms->isError(_response);

        *response = _response.c_str();

        return ret;
    }

    
    /** Renew an existing watch for seven days.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param watchId (string) The watch ID.
     * @return Boolean type status indicates the success of the operation.
     * 
     * For ref doc go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/renew
     *
     */
    template <typename T1 = const char *, typename T2 = const char *>
    bool renewWatch(FirebaseJson *response, T1 formId, T2 watchId)
    {
        MB_String _response;

        bool ret = gforms->renewWatch(_response, toString(formId), toString(watchId));

        if (ret)
            ret = !gforms->isError(_response);

        response->setJsonData(_response);

        return ret;
    }

    /** Renew an existing watch for seven days.
     *
     * @param response (FirebaseJson or String) The returned response.
     * @param formId (string) The form ID.
     * @param watchId (string) The watch ID.
     * @return Boolean type status indicates the success of the operation.
     * 
     * For ref doc go to https://developers.google.com/forms/api/reference/rest/v1beta/forms.watches/renew
     *
     */
    template <typename T1 = const char *, typename T2 = const char *>
    bool renewWatch(String *response, T1 formId, T2 watchId)
    {
        MB_String _response;

        bool ret = gforms->renewWatch(_response, toString(formId), toString(watchId));

        if (ret)
            ret = !gforms->isError(_response);

        *response = _response.c_str();

        return ret;
    }


#if defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD)

    /** Initiate SD card with SPI port configuration.
     *
     * @param ss SPI Chip/Slave Select pin.
     * @param sck SPI Clock pin.
     * @param miso SPI MISO pin.
     * @param mosi SPI MOSI pin.
     * @param frequency The SPI frequency
     * @return Boolean type status indicates the success of the operation.
     */
    bool sdBegin(int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, uint32_t frequency = 4000000)
    {
        return gforms->mbfs.sdBegin(ss, sck, miso, mosi, frequency);
    }

#if defined(ESP8266)

    /** Initiate SD card with SD FS configurations (ESP8266 only).
     *
     * @param ss SPI Chip/Slave Select pin.
     * @param sdFSConfig The pointer to SDFSConfig object (ESP8266 only).
     * @return Boolean type status indicates the success of the operation.
     */
    bool sdBegin(SDFSConfig *sdFSConfig)
    {
        return gforms->mbfs.sdFatBegin(sdFSConfig);
    }

#endif

#if defined(ESP32)
    /** Initiate SD card with chip select and SPI configuration (ESP32 only).
     *
     * @param ss SPI Chip/Slave Select pin.
     * @param spiConfig The pointer to SPIClass object for SPI configuartion.
     * @param frequency The SPI frequency.
     * @return Boolean type status indicates the success of the operation.
     */
    bool sdBegin(int8_t ss, SPIClass *spiConfig = nullptr, uint32_t frequency = 4000000)
    {
        return gforms->mbfs.sdSPIBegin(ss, spiConfig, frequency);
    }
#endif

#if defined(MBFS_ESP32_SDFAT_ENABLED) || defined(MBFS_SDFAT_ENABLED)
    /** Initiate SD card with SdFat SPI and pins configurations (with SdFat included only).
     *
     * @param sdFatSPIConfig The pointer to SdSpiConfig object for SdFat SPI configuration.
     * @param ss SPI Chip/Slave Select pin.
     * @param sck SPI Clock pin.
     * @param miso SPI MISO pin.
     * @param mosi SPI MOSI pin.
     * @return Boolean type status indicates the success of the operation.
     */
    bool sdBegin(SdSpiConfig *sdFatSPIConfig, int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1)
    {
        return gforms->mbfs.sdFatBegin(sdFatSPIConfig, ss, sck, miso, mosi);
    }

    /** Initiate SD card with SdFat SDIO configuration (with SdFat included only).
     *
     * @param sdFatSDIOConfig The pointer to SdioConfig object for SdFat SDIO configuration.
     * @return Boolean type status indicates the success of the operation.
     */
    bool sdBegin(SdioConfig *sdFatSDIOConfig)
    {
        return gforms->mbfs.sdFatBegin(sdFatSDIOConfig);
    }

#endif

#endif

#if defined(ESP32) && defined(MBFS_SD_FS) && defined(MBFS_CARD_TYPE_SD_MMC)
    /** Initialize the SD_MMC card (ESP32 only).
     *
     * @param mountpoint The mounting point.
     * @param mode1bit Allow 1 bit data line (SPI mode).
     * @param format_if_mount_failed Format SD_MMC card if mount failed.
     * @return The boolean value indicates the success of operation.
     */
    bool sdMMCBegin(const char *mountpoint = "/sdcard", bool mode1bit = false, bool format_if_mount_failed = false)
    {
        return gforms->mbfs.sdMMCBegin(mountpoint, mode1bit, format_if_mount_failed);
    }
#endif

private:
    GFormsClass *gforms = NULL;

protected:
    template <typename T>
    auto toString(const T &val) -> typename mb_string::enable_if<mb_string::is_std_string<T>::value || mb_string::is_arduino_string<T>::value || mb_string::is_mb_string<T>::value || mb_string::is_same<T, StringSumHelper>::value, const char *>::type { return val.c_str(); }

    template <typename T>
    auto toString(T val) -> typename mb_string::enable_if<mb_string::is_const_chars<T>::value, const char *>::type { return val; }

    template <typename T>
    auto toString(T val) -> typename mb_string::enable_if<mb_string::fs_t<T>::value, const char *>::type { return (const char *)val; }

    template <typename T>
    auto toString(T val) -> typename mb_string::enable_if<mb_string::is_same<T, std::nullptr_t>::value, const char *>::type { return ""; }
};

extern ESP_Google_Forms_Client GForms;

#endif