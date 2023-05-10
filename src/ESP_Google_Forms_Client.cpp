/**
 * Google Forms Client, ESP_Google_Forms_Client.cpp v1.0.0
 *
 * This library supports Espressif ESP8266 and ESP32 MCUs
 *
 * Created May 9, 2023
 *
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
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

#ifndef GFormsClass_CPP
#define GFormsClass_CPP

#include "ESP_Google_Forms_Client.h"

GFormsClass::GFormsClass()
{
    authMan.begin(&config, &mbfs, &mb_ts, &mb_ts_offset);
    authMan.newClient(&authMan.tcpClient);
}

GFormsClass::~GFormsClass()
{
    authMan.end();
}

void GFormsClass::auth(const char *client_email, const char *project_id, const char *private_key, const char *sa_key_file, esp_google_forms_file_storage_type storage_type, ESP8266_SPI_ETH_MODULE *eth)
{
    config.service_account.data.client_email = client_email;
    config.service_account.data.project_id = project_id;
    config.service_account.data.private_key = private_key;
    config.signer.expiredSeconds = 3600;

    config.service_account.json.path = sa_key_file;
    config.service_account.json.storage_type = (mb_fs_mem_storage_type)storage_type;

    if (eth)
    {
#if defined(ESP8266) && defined(ESP8266_CORE_SDK_V3_X_X)
#ifdef INC_ENC28J60_LWIP
        config.spi_ethernet_module.enc28j60 = eth;
#endif
#ifdef INC_W5100_LWIP
        config.spi_ethernet_module.w5100 = eth;
#endif
#ifdef INC_W5500_LWIP
        config.spi_ethernet_module.w5500 = eth;
#endif
#endif
    }

#if defined(ESP32) || defined(ESP8266)
    config.internal.reconnect_wifi = WiFi.getAutoReconnect();
#endif
    config.signer.tokens.token_type = token_type_oauth2_access_token;

    authMan.begin(&config, &mbfs, &mb_ts, &mb_ts_offset);
}

void GFormsClass::setTokenCallback(TokenStatusCallback callback)
{
    config.token_status_callback = callback;
}

void GFormsClass::addAP(const char *ssid, const char *password)
{
    config.wifi.addAP(ssid, password);
}

void GFormsClass::clearAP()
{
    config.wifi.clearAP();
}

bool GFormsClass::checkToken()
{
    return authMan.tokenReady();
}

String GFormsClass::accessToken()
{
    return config.internal.auth_token.c_str();
}

void GFormsClass::setPrerefreshSeconds(uint16_t seconds)
{
    if (seconds >= 60 || seconds <= 3540)
        config.signer.preRefreshSeconds = seconds;
}

bool GFormsClass::setClock(float gmtOffset)
{
    return TimeHelper::syncClock(&authMan.ntpClient, &mb_ts, &mb_ts_offset, gmtOffset, &config);
}

#if defined(ESP_GOOGLE_FORMS_CLIENT_ENABLE_EXTERNAL_CLIENT)
void GFormsClass::setClient(Client *client, GFORMS_NetworkConnectionRequestCallback networkConnectionCB,
                            GFORMS_NetworkStatusRequestCallback networkStatusCB)
{
    authMan.tcpClient->setClient(client, networkConnectionCB, networkStatusCB);
    authMan.tcpClient->setCACert(nullptr);
}

void GFormsClass::setUDPClient(UDP *client, float gmtOffset)
{
    authMan.udp = client;
    authMan.gmtOffset = gmtOffset;
}
#endif

bool GFormsClass::waitClockReady()
{
    unsigned long ms = millis();
    while (!setClock(config.internal.gmt_offset) && millis() - ms < 3000)
    {
        Utils::idle();
    }
    return config.internal.clock_rdy;
}

void GFormsClass::setCert(const char *ca)
{
    int addr = reinterpret_cast<int>(ca);
    if (addr != cert_addr)
    {
        cert_updated = true;
        cert_addr = addr;
#if defined(ESP8266) || defined(MB_ARDUINO_PICO)
        waitClockReady();
#endif
    }
}

void GFormsClass::setCertFile(const char *filename, esp_google_forms_file_storage_type type)
{
    config.cert.file = filename;
    config.cert.file_storage = (mb_fs_mem_storage_type)type;
    cert_addr = 0;
    if (config.cert.file.length() > 0)
    {
        cert_updated = true;

#if defined(ESP8266) || defined(MB_ARDUINO_PICO)
        waitClockReady();
#endif
    }
}

void GFormsClass::reset()
{
    config.internal.client_id.clear();
    config.internal.client_secret.clear();
    config.internal.auth_token.clear();
    config.internal.last_jwt_generation_error_cb_millis = 0;
    config.signer.tokens.expires = 0;
    config.internal.rtoken_requested = false;

    config.internal.priv_key_crc = 0;
    config.internal.email_crc = 0;
    config.internal.password_crc = 0;

    config.signer.tokens.status = token_status_uninitialized;
}

bool GFormsClass::setSecure()
{
    GFORMS_TCP_Client *client = authMan.tcpClient;

    if (!client)
        return false;

    client->setConfig(&config, &mbfs);

    if (!authMan.reconnect(client))
        return false;

#if (defined(ESP8266) || defined(MB_ARDUINO_PICO))
    if (TimeHelper::getTime(&mb_ts, &mb_ts_offset) > GFORMS_DEFAULT_TS)
    {
        config.internal.clock_rdy = true;
        client->setClockStatus(true);
    }
#endif

    if (client->getCertType() == gforms_cert_type_undefined || cert_updated)
    {

        if (!config.internal.clock_rdy && (config.cert.file.length() > 0 || config.cert.data != NULL || cert_addr > 0))
            TimeHelper::syncClock(&authMan.ntpClient, &mb_ts, &mb_ts_offset, config.internal.gmt_offset, &config);

        if (config.cert.file.length() == 0)
        {
            if (cert_addr > 0)
                client->setCACert(reinterpret_cast<const char *>(cert_addr));
            else if (config.cert.data != NULL)
                client->setCACert(config.cert.data);
            else
                client->setCACert(NULL);
        }
        else
        {
            if (!client->setCertFile(config.cert.file.c_str(), config.cert.file_storage))
                client->setCACert(NULL);
        }
        cert_updated = false;
    }
    return true;
}

bool GFormsClass::beginRequest(MB_String &req, host_type_t host_type)
{
    GFORMS_TCP_Client *client = authMan.tcpClient;

    if (!setSecure())
        return false;

    if (client && !client->connected())
    {

#if defined(ESP8266) || defined(MB_ARDUINO_PICO)
        if (host_type == host_type_forms)
            client->ethDNSWorkAround(&config.spi_ethernet_module, (const char *)FPSTR("forms.googleapis.com"), 443);
        else if (host_type == host_type_drive)
            client->ethDNSWorkAround(&config.spi_ethernet_module, (const char *)FPSTR("www.googleapis.com"), 443);
#endif

        if (host_type == host_type_forms)
            client->begin((const char *)FPSTR("forms.googleapis.com"), 443, &response_code);
        else if (host_type == host_type_drive)
            client->begin((const char *)FPSTR("www.googleapis.com"), 443, &response_code);
    }

    return true;
}

void GFormsClass::addHeader(MB_String &req, host_type_t host_type, int len)
{
    req += FPSTR(" HTTP/1.1\r\n");
    if (host_type == host_type_forms)
        req += FPSTR("Host: forms.googleapis.com\r\n");
    else if (host_type == host_type_drive)
        req += FPSTR("Host: www.googleapis.com\r\n");
    req += FPSTR("Authorization: Bearer ");
    req += config.internal.auth_token;
    req += FPSTR("\r\n");

    if (len > -1)
    {
        req += FPSTR("Content-Length: ");
        req += len;
        req += FPSTR("\r\n");

        req += FPSTR("Content-Type: application/json\r\n");
    }

    req += FPSTR("Connection: keep-alive\r\n");
    req += FPSTR("Keep-Alive: timeout=30, max=100\r\n");
    req += FPSTR("Accept-Encoding: identity;q=1,chunked;q=0.1,*;q=0\r\n");
}

bool GFormsClass::processRequest(MB_String &req, MB_String &response, int &httpcode, const char *key)
{
    GFORMS_TCP_Client *client = authMan.tcpClient;

    if (!client)
        return false;

    authMan.response_code = 0;

    int ret = client->send(req.c_str());
    req.clear();
    config.signer.tokens.error.message.clear();

    if (ret > 0)
    {
        ret = authMan.handleResponse(client, httpcode, response, key, false);
        if (!ret)
        {
            authMan.response_code = httpcode;
            FirebaseJson json(response);
            FirebaseJsonData result;
            json.get(result, "error/message");
            if (result.success)
                config.signer.tokens.error.message = result.stringValue;
            else
                config.signer.tokens.error.message = response;
        }
    }

    if (ret < 0)
    {
        authMan.response_code = ret;
        httpcode = ret;
    }

    if (!ret)
        client->stop();

    return ret > 0;
}

bool GFormsClass::create(MB_String &response, const char *title, const char *docTitle)
{
    if (!checkToken())
        return false;

    MB_String req, payload;
    int httpcode = 0;

    if (!beginRequest(req, host_type_forms))
        return false;

    payload = FPSTR("{\"info\":{\"title\":\"");
    payload += title;
    payload += FPSTR("\",\"document_title\":\"");
    payload += strlen(docTitle) > 0 ? docTitle : title;
    payload += FPSTR("\"}}");

    req = FPSTR("POST /v1/forms");

    addHeader(req, host_type_forms, payload.length());

    req += FPSTR("\r\n");
    req += payload;

    return processRequest(req, response, httpcode);
}

bool GFormsClass::isError(MB_String &response)
{
    authMan.initJson();
    bool ret = false;
    if (JsonHelper::setData(authMan.jsonPtr, response, false))
        ret = JsonHelper::parse(authMan.jsonPtr, authMan.resultPtr, gauth_pgm_str_14) || JsonHelper::parse(authMan.jsonPtr, authMan.resultPtr, gauth_pgm_str_14);

    authMan.freeJson();
    return ret;
}

bool GFormsClass::createPermission(MB_String &response, const char *fileid, const char *role, const char *type, const char *email)
{
    GFORMS_TCP_Client *client = authMan.tcpClient;

    if (!client)
        return false;

    MB_String req;
    int httpcode = 0;

    if (!beginRequest(req, host_type_drive))
        return false;

    req = FPSTR("POST /drive/v3/files/");
    req += fileid;
    req += FPSTR("/permissions?supportsAllDrives=true");

    if (strcmp(role, (const char *)FPSTR("owner")) == 0)
        req += FPSTR("&transferOwnership=true");

    FirebaseJson js;
    js.add((const char *)FPSTR("role"), role);
    js.add((const char *)FPSTR("type"), type);
    js.add((const char *)FPSTR("emailAddress"), email);

    addHeader(req, host_type_drive, strlen(js.raw()));

    req += FPSTR("\r\n");
    req += js.raw();

    bool ret = processRequest(req, response, httpcode);

    return ret;
}

bool GFormsClass::batchUpdate(MB_String &response, const char *formId, FirebaseJson *request)
{
    if (!checkToken())
        return false;

    MB_String req;
    int httpcode = 0;

    if (!beginRequest(req, host_type_forms))
        return false;

    req = FPSTR("POST /v1/forms/");
    req += formId;

    req += FPSTR(":batchUpdate");

    if (request)
    {
        addHeader(req, host_type_forms, strlen(request->raw()));
        req += FPSTR("\r\n");
        req += request->raw();

        return processRequest(req, response, httpcode);
    }

    return false;
}

bool GFormsClass::getForm(MB_String &response, const char *formId)
{
    if (!checkToken())
        return false;

    MB_String req;
    int httpcode = 0;

    if (!beginRequest(req, host_type_forms))
        return false;

    req = FPSTR("GET /v1/forms/");
    req += formId;

    addHeader(req, host_type_forms);

    req += FPSTR("\r\n");

    return processRequest(req, response, httpcode);
}

bool GFormsClass::listResponses(MB_String &response, const char *formId, const char *key, const char *filter, int pageSize, const char *pageToken)
{
    if (!checkToken())
        return false;

    MB_String req, qr;
    int httpcode = 0;

    if (!beginRequest(req, host_type_forms))
        return false;

    req = FPSTR("GET /v1/forms/");
    req += formId;
    req += FPSTR("/responses");

    if (strlen(filter) > 0)
    {
        qr += qr.length() == 0 ? FPSTR("?") : FPSTR("&");
        qr += FPSTR("filter=");
        qr += filter;
    }

    if (pageSize > 0)
    {
        qr += qr.length() == 0 ? FPSTR("?") : FPSTR("&");
        qr += FPSTR("pageSize=");
        qr += pageSize;
    }

    if (strlen(pageToken) > 0)
    {
        qr += qr.length() == 0 ? FPSTR("?") : FPSTR("&");
        qr += FPSTR("pageToken=");
        qr += pageToken;
    }

    req += qr;

    addHeader(req, host_type_forms);

    req += FPSTR("\r\n");

    return processRequest(req, response, httpcode, key);
}

bool GFormsClass::getResponse(MB_String &response, const char *formId, const char *responseId)
{
    if (!checkToken())
        return false;

    MB_String req;
    int httpcode = 0;

    if (!beginRequest(req, host_type_forms))
        return false;

    req = FPSTR("GET /v1/forms/");
    req += formId;
    req += FPSTR("/responses/");
    req += responseId;

    addHeader(req, host_type_forms);

    req += FPSTR("\r\n");

    return processRequest(req, response, httpcode, "");
}

bool GFormsClass::createWatch(MB_String &response, const char *formId, FirebaseJson *request)
{
    if (!checkToken())
        return false;

    MB_String req;
    int httpcode = 0;

    if (!beginRequest(req, host_type_forms))
        return false;

    req = FPSTR("POST /v1/forms/");
    req += formId;
    req += FPSTR("/watches");

    if (request)
    {
        addHeader(req, host_type_forms, strlen(request->raw()));
        req += FPSTR("\r\n");
        req += request->raw();

        return processRequest(req, response, httpcode);
    }

    return false;
}

bool GFormsClass::listWatch(MB_String &response, const char *formId, const char *key)
{
    if (!checkToken())
        return false;

    MB_String req;
    int httpcode = 0;

    if (!beginRequest(req, host_type_forms))
        return false;

    req = FPSTR("GET /v1/forms/");
    req += formId;
    req += FPSTR("/watches");

    addHeader(req, host_type_forms);

    req += FPSTR("\r\n");

    return processRequest(req, response, httpcode, key);
}

bool GFormsClass::deleteWatch(MB_String &response, const char *formId, const char *watchId)
{
    if (!checkToken())
        return false;

    MB_String req;
    int httpcode = 0;

    if (!beginRequest(req, host_type_forms))
        return false;

    req = FPSTR("DELETE /v1/forms/");
    req += formId;
    req += FPSTR("/watches/");
    req += watchId;

    addHeader(req, host_type_forms);

    req += FPSTR("\r\n");

    return processRequest(req, response, httpcode);
}

bool GFormsClass::listWatches(MB_String &response, const char *formId)
{
    if (!checkToken())
        return false;

    MB_String req;
    int httpcode = 0;

    if (!beginRequest(req, host_type_forms))
        return false;

    req = FPSTR("GET /v1/forms/");
    req += formId;
    req += FPSTR("/watches");

    addHeader(req, host_type_forms);

    req += FPSTR("\r\n");

    return processRequest(req, response, httpcode);
}

bool GFormsClass::renewWatch(MB_String &response, const char *formId, const char *watchId)
{
    if (!checkToken())
        return false;

    MB_String req;
    int httpcode = 0;

    if (!beginRequest(req, host_type_forms))
        return false;

    req = FPSTR("POST /v1/forms/");
    req += formId;
    req += FPSTR("/watches/");
    req += watchId;
    req += FPSTR(":renew");

    addHeader(req, host_type_forms, 0);

    req += FPSTR("\r\n");

    return processRequest(req, response, httpcode);
}

ESP_Google_Forms_Client GForms = ESP_Google_Forms_Client();

#endif