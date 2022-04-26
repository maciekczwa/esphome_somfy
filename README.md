# esphome_somfy
Somfy RTS remote integration with esphome


# Hardware

To use this component you will need esp32 and generic 433 MHz transmitter with changed quartz to 433.42 MHz.

# Setup

```yaml
esphome:
  name: somfy
  includes:
  - Somfy.h
  libraries:
  - EEPROM
  - legion2/Somfy_Remote_Lib@^0.4.1

esp32:
  board: esp32dev
  framework:
    type: arduino

wifi:
  ssid: !secret ssid_dol
  password: !secret wifi_password

logger:
  level: DEBUG

api:
  password: !secret api_password

ota:
  password: !secret ota_password
 
custom_component:
- lambda: |-
    auto my_custom = new RFsomfy(4, 0x1A7A11, "remote_1");
    return {my_custom}; 
```

```
RFsomfy parameters:
pin - for example GPIO4
remote_id - 0x1A7A11
name - "remote_1" - name of remote to store rolling code, it must be different for every remote you create
```

The component will create services in homeasssitant.
```
esphome.somfy_up - UP button
esphome.somfy_down - DOWN button
esphome.somfy_stop - STOP/MY button
esphome.somfy_prog - PROG button
```

To program the button into receiver for example Somfy Dexxo Optima hold the PROG button on the gate opener for more than two seconds. After the light on the opener turned on call esphome.somfy_prog service to send PROG command from emulated remote to gate opener.
Now you can use services to esphome.somfy_up, esphome.somfy_down, esphome.somfy_stop open/close/stop gate.

You can create template cover in Homeassistant for controlling the gate.
For example:
```yaml
cover:
  - platform: template
    covers:
      garaz:
        device_class: garage
        friendly_name: "Garage"
        value_template: "{{ states('binary_sensor.garage_sensor')=='on' }}"
        availability_template: "{{ states('binary_sensor.alarm_armed')=='off' }}"
        open_cover:
          service: esphome.bt_up
        close_cover:
          service: esphome.bt_down
        stop_cover:
          service: esphome.bt_stop
```

I use external sensor for determining the state of door. You can use value_template for that.
I also block the possibility of opening the gate when security alarm is armed. You can use availabilty_template for that.

