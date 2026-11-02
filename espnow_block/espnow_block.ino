#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_arduino_version.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

const uint8_t CHANNEL = 1;
const int MAX_BOARDS = 128;
const char MESSAGE[] = "BCA188-LAB10";
const uint8_t BROADCAST[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct MacAddress {
  uint8_t bytes[6];
};

QueueHandle_t received;
MacAddress heard[MAX_BOARDS];
uint8_t boardMac[6];
int boardCount = 0;
bool listFull = false;
uint32_t startedAt = 0;
uint32_t nextSend = 0;
uint32_t lastReport = 0;

void stopWithError(const char* message) {
  Serial.println(message);
  while (true) delay(1000);
}

void checkResult(esp_err_t result) {
  if (result != ESP_OK) stopWithError(esp_err_to_name(result));
}

void printMac(const uint8_t* mac) {
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2],
                mac[3], mac[4], mac[5]);
}

void queueSender(const uint8_t* mac, const uint8_t* data, int length) {
  if (!mac || !data || length != sizeof(MESSAGE)) return;
  if (memcmp(data, MESSAGE, sizeof(MESSAGE)) != 0) return;
  MacAddress sender;
  memcpy(sender.bytes, mac, 6);
  xQueueSend(received, &sender, 0);
}

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onReceive(const esp_now_recv_info_t* info, const uint8_t* data, int length) {
  if (info) queueSender(info->src_addr, data, length);
}
#else
void onReceive(const uint8_t* mac, const uint8_t* data, int length) {
  queueSender(mac, data, length);
}
#endif

void rememberBoard(const MacAddress& sender) {
  if (memcmp(sender.bytes, boardMac, 6) == 0) return;
  for (int i = 0; i < boardCount; i++) {
    if (memcmp(sender.bytes, heard[i].bytes, 6) == 0) return;
  }
  if (boardCount == MAX_BOARDS) {
    listFull = true;
    return;
  }
  heard[boardCount++] = sender;
}

void printReport() {
  Serial.println("\n--- ESP-NOW Block Network ---");
  Serial.print("Board MAC: ");
  printMac(boardMac);
  Serial.printf("\nChannel: %u\nElapsed: %lu seconds\n", CHANNEL,
                (unsigned long)((millis() - startedAt) / 1000));
  Serial.printf("Boards heard: %d\n", boardCount);
  for (int i = 0; i < boardCount; i++) {
    Serial.printf("%d. ", i + 1);
    printMac(heard[i].bytes);
    Serial.println();
  }
  if (boardCount == 0) Serial.println("No matching boards heard yet.");
  if (millis() - startedAt < 30000) Serial.println("Wait at least 30 seconds before recording results.");
  if (listFull) Serial.println("List limit reached; count is capped at 128.");
}

void setup() {
  Serial.begin(115200);
  uint32_t waitStarted = millis();
  while (!Serial && millis() - waitStarted < 2500) delay(10);

  WiFi.persistent(false);
  if (!WiFi.mode(WIFI_STA)) stopWithError("Radio setup failed.");
  WiFi.setAutoReconnect(false);
  WiFi.disconnect(false, false);
  WiFi.setSleep(false);
  checkResult(esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE));
  checkResult(esp_wifi_get_mac(WIFI_IF_STA, boardMac));

  received = xQueueCreate(MAX_BOARDS, sizeof(MacAddress));
  if (!received) stopWithError("Receive queue setup failed.");
  checkResult(esp_now_init());
  checkResult(esp_now_register_recv_cb(onReceive));

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST, 6);
  peer.channel = CHANNEL;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  checkResult(esp_now_add_peer(&peer));

  startedAt = lastReport = millis();
  nextSend = startedAt + esp_random() % 1000;
  Serial.println("Sample sketch: same sketch and channel required on the other boards.");
  printReport();
}

void loop() {
  MacAddress sender;
  while (xQueueReceive(received, &sender, 0) == pdTRUE) rememberBoard(sender);

  uint32_t now = millis();
  if ((int32_t)(now - nextSend) >= 0) {
    esp_err_t result = esp_now_send(BROADCAST, (const uint8_t*)MESSAGE, sizeof(MESSAGE));
    if (result != ESP_OK) Serial.printf("Send failed: %s\n", esp_err_to_name(result));
    nextSend = now + 1000 + esp_random() % 500;
  }
  if (now - lastReport >= 5000) {
    lastReport = now;
    printReport();
  }
  delay(10);
}
