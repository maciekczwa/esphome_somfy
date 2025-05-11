#include "esphome.h"

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/remote_transmitter/remote_transmitter.h"
#include "esphome/components/api/custom_api_device.h"

#define SYMBOL 640
#define DEBUG 1

namespace esphome
{
  namespace somfy
  {

    using namespace esphome::remote_transmitter;

    static const char *SOMFY_TAG = "somfy";

    enum Command : uint8_t
    {
      My = 0x1,
      Up = 0x2,
      MyUp = 0x3,
      Down = 0x4,
      MyDown = 0x5,
      UpDown = 0x6,
      Prog = 0x8,
      SunFlag = 0x9,
      Flag = 0xA
    };

    class RFsomfy : public Component, public api::CustomAPIDevice, public EntityBase, public remote_base::RemoteTransmittable
    {

    private:
      ESPPreferenceObject pref;
      uint32_t remoteId;
      std::string name;

    public:
      RFsomfy()
      {
      }

      void set_address(uint32_t _remoteId)
      {
        remoteId = _remoteId;
        name = str_snprintf("somfy_remote_%d", 32, remoteId);
        set_object_id(name.c_str());
        set_name(name.c_str());
      }

      void setup() override
      {
        pref = global_preferences->make_preference<int>(this->get_object_id_hash());
        register_service(&RFsomfy::on_up, "up");
        register_service(&RFsomfy::on_down, "down");
        register_service(&RFsomfy::on_stop, "stop");
        register_service(&RFsomfy::on_prog, "prog");
      }

      void on_up()
      {
        ESP_LOGD(SOMFY_TAG, "command up");
        sendCommand(Command::Up, 1);
      }

      void on_down()
      {
        ESP_LOGD(SOMFY_TAG, "command down");
        sendCommand(Command::Down, 1);
      }

      void on_stop()
      {
        ESP_LOGD(SOMFY_TAG, "command stop");
        sendCommand(Command::My, 1);
      }

      void on_prog()
      {
        ESP_LOGD(SOMFY_TAG, "command prog");
        sendCommand(Command::Prog, 1);
      }

      void sendCommand(Command command, int repeat)
      {

        uint16_t rollingCode = 0;
        pref.load(&rollingCode);
        rollingCode++;
        pref.save(&rollingCode);
        global_preferences->sync();

        ESP_LOGD(SOMFY_TAG, "name base: %s", this->get_name().c_str());
        ESP_LOGD(SOMFY_TAG, "objectid: %s", this->get_object_id().c_str());
        ESP_LOGD(SOMFY_TAG, "objectid: %d", this->get_object_id_hash());
        ESP_LOGD(SOMFY_TAG, "name: %s", name.c_str());
        ESP_LOGD(SOMFY_TAG, "code :%d", rollingCode);
        uint8_t frame[7];
        buildFrame(frame, command, rollingCode);
        sendFrame(frame, 2);
        for (int i = 0; i < repeat; i++)
        {
          sendFrame(frame, 7);
        }
      }

      void buildFrame(uint8_t *frame, Command command, uint16_t code)
      {
        const uint8_t button = static_cast<uint8_t>(command);
        frame[0] = 0xA7;           // Encryption key. Doesn't matter much
        frame[1] = button << 4;    // Which button did  you press? The 4 LSB will be the checksum
        frame[2] = code >> 8;      // Rolling code (big endian)
        frame[3] = code;           // Rolling code
        frame[4] = remoteId >> 16; // Remote address
        frame[5] = remoteId >> 8;  // Remote address
        frame[6] = remoteId;       // Remote address

#ifdef DEBUG
        ESP_LOGD(SOMFY_TAG, "Frame         : ");
        printFrame(frame);
#endif

        // Checksum calculation: a XOR of all the nibbles
        uint8_t checksum = 0;
        for (uint8_t i = 0; i < 7; i++)
        {
          checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
        }
        checksum &= 0b1111; // We keep the last 4 bits only

        // Checksum integration
        frame[1] |= checksum;

#ifdef DEBUG
        ESP_LOGD(SOMFY_TAG, "With checksum : ");
        printFrame(frame);
#endif

        // Obfuscation: a XOR of all the bytes
        for (uint8_t i = 1; i < 7; i++)
        {
          frame[i] ^= frame[i - 1];
        }

#ifdef DEBUG
        ESP_LOGD(SOMFY_TAG, "Obfuscated    : ");
        printFrame(frame);
#endif
      }

      void sendFrame(uint8_t *frame, uint8_t sync)
      {
        ESP_LOGD(SOMFY_TAG, "sendFrame sync: %d", sync);
        printFrame(frame);
        auto transmit = this->transmitter_->transmit();
        auto *data = transmit.get_data();
        if (sync == 2)
        { // Only with the first frame.
          // Wake-up pulse & Silence
          data->mark(9415);
          data->space(89565);
        }

        // Hardware sync: two sync for the first frame, seven for the following ones.
        for (int i = 0; i < sync; i++)
        {
          data->mark(4 * SYMBOL);
          data->space(4 * SYMBOL);
        }

        // Software sync
        data->mark(4550);
        data->space(SYMBOL);

        // Data: bits are sent one by one, starting with the MSB.
        for (uint8_t i = 0; i < 56; i++)
        {
          if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 1)
          {
            data->space(SYMBOL);
            data->mark(SYMBOL);
          }
          else
          {
            data->mark(SYMBOL);
            data->space(SYMBOL);
          }
        }

        // Inter-frame silence
        data->space(30415);
        transmit.perform();
      }

      void printFrame(uint8_t *f)
      {
        ESP_LOGD(SOMFY_TAG, "%X %X %X %X %X %X %X ", f[0], f[1], f[2], f[3], f[4], f[5], f[6]);
      }
    };

  }
}