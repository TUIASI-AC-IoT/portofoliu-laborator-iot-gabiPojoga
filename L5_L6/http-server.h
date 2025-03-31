#ifndef _HTTP_S_H_
#define _HTTP_S_H_

// #include "wifi_scan.h"

// extern wifi_ap_record_t scanned_wifis[DEFAULT_SCAN_LIST_SIZE];

httpd_handle_t start_webserver(void);
void stop_webserver();
#endif