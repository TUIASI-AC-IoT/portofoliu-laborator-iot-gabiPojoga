#ifndef _MDNS_CONFIG_H
#define _MDNS_CONFIG_H

#define MDNS_LOCAL_HOSTNAME "pojoga-esp32"
#define MDNS_LOCAL_INSTANCE_NAME "pojoga's ESP32 Thing"

void start_mdns_service();
void resolve_mdns_host();

#endif