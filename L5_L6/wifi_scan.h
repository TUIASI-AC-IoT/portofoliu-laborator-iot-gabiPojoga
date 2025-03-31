#ifndef _WIFI_SCAN_H
#define _WIFI_SCAN_H

// Defines
#define DEFAULT_SCAN_LIST_SIZE 10

// Variables
extern wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];

// Functions
void wifi_scan();

#endif