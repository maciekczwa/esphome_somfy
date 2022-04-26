#include "esphome.h"
using namespace esphome;

#include <NVSRollingCodeStorage.h>
#include <SomfyRemote.h>

static const char *SOMFY_TAG = "somfy";

class RFsomfy : public Component, public CustomAPIDevice {

private:
  NVSRollingCodeStorage rollingCodeStorage;
  SomfyRemote somfyRemote;

public:

  RFsomfy(int pin, int remoteId, const char* remoteName) : rollingCodeStorage("somfy", remoteName), somfyRemote(pin, remoteId, &rollingCodeStorage) {

  }

  void setup() override {
    somfyRemote.setup();
    register_service(&RFsomfy::on_up, "up");
    register_service(&RFsomfy::on_down, "down");
    register_service(&RFsomfy::on_stop, "stop");
    register_service(&RFsomfy::on_prog, "prog");
  }

  void on_up() {
    ESP_LOGD(SOMFY_TAG,"command up");
    somfyRemote.sendCommand(Command::Up, 1);
  }

  void on_down() {
    ESP_LOGD(SOMFY_TAG,"command down");
    somfyRemote.sendCommand(Command::Down, 1);
  }

  void on_stop() {
    ESP_LOGD(SOMFY_TAG,"command stop");
    somfyRemote.sendCommand(Command::My, 1);
  }

  void on_prog() {
    ESP_LOGD(SOMFY_TAG,"command prog");
    somfyRemote.sendCommand(Command::Prog, 1);
  }

};
