# Esphome Radsens Component
## About this repo
This is a full esphome native component for the ClimateGuard Radsens.
This was motivated by the coming ["custom component-pocolypse"](https://esphome.io/guides/contributing#a-note-about-custom-components) and I still suffer from occasional
hangs in the original module (the esp remains pingable but nothing else responds).
External components are quite a bit better integrated with esphome and easier for users to use and allow other i2c
devices on the same esp32.  Esphome actually calls out Wire as a library which doesn't play well with others.

## What is a Radsens?
Radsens is a geiger counter built by ClimateGuard.  Their pages can be found [here](https://climateguard.info/radsens/).
I ordered mine before the war with Ukraine and I don't entirely understand the relation between countries
 but its worth noting it appears to be a Kazakhstan company.

* [tindie carries the RadSens](https://www.tindie.com/products/climateguard/dosimeter-with-i2c-radsens-2-arduino/)

## I2C Errors
If you are using this and get i2c errors please let me know with a github issue.  It appears to have
a timing issue.  Setting the switches on startup began failing in esphome 2025.8.0.
I re-sorted the switch setup which seems like a bit of a hack, but it at least works for me at the moment.  

## Setup
### Simple Example

``` yaml
external_components:
  - source: github://d1m0k/esphome-radsens@1.0.3
    components: [ radsens ]
    refresh: 600s

i2c:
  - id: bus_a
    frequency: 100kHz

radsens:

sensor:
  - platform: radsens
    dynamic_intensity:
      name: "Dynamic Intensity"
    static_intensity:
      name: "Static Intensity"
    counts_per_polling:
      name: "Counts Per Polling"
    counts_per_minute:
      name: "Counts Per Minute"
    radsens_max_cpp:
      name: "RadSens Max CPP"
    max_cpp_timestamp:
      name: "Max CPP Timestamp (s uptime)"
    radsens_max_cpm:
      name: "RadSens Max CPM"
    max_cpm_timestamp:
      name: "Max CPM Timestamp (s uptime)"
    total_cpp:
      name: "Total CPP"
    uptime:
      name: "Uptime"
    accumulated_dose_ur:
      name: "Accumulated Dose (uR)"
    accumulated_dose_msv:
      name: "Accumulated Dose (mSv)"

number:
  - platform: radsens
    polling_interval:
      name: "Polling Interval"
```


### Full Configuration
A full configuration can be found [here](geiger.yaml) but I recommend reading the following to undestand it.

| field | notes |
|-------|-------|
| external_components | This is not yet in the core esphome so you need to pull it in this way |
| external_components:source | This should be pointed to this github repo github://krbaker/esphome-radsens likely you want to take the current release found [here](https://github.com/krbaker/esphome-radsens/releases) using @\<tag\>
| external_components:source:components | There is one and only component here [ radsens ] |
| external_components:source:refresh | Probably don't need any more than 600s here |
| radsens | The radens component must be enabled to install the component |
| radsens:sensativity | This field is not required.  If it us unset or set to 0 the component will not set anything.  The default according to the docs is 105 and that is what mine was shipped with.  You probably do not need this |
| sensor:radsens | Needed to enable the sensors |
| sensor:radsens:dynamic_intensity | The 'dynamic_intensity' sensor which is an average intensity over a varying time window |
| sensor:radsens:static_intensity | The 'static_intensity' sensor which is an average intensity over the last 5min |
| sensor:radsens:counts_per_polling | The number of detected impulses in the current polling interval |
| sensor:radsens:counts_per_minute | Counts-per-minute computed from average polling counts over 60 seconds (published every ~60s) |
| sensor:radsens:radsens_max_cpp | Max observed counts_per_polling since boot |
| sensor:radsens:max_cpp_timestamp | Uptime seconds when max CPP was detected |
| sensor:radsens:radsens_max_cpm | Max observed counts_per_minute since boot |
| sensor:radsens:max_cpm_timestamp | Uptime seconds when max CPM was detected |
| sensor:radsens:total_cpp | Sum of all counts_per_polling since boot |
| sensor:radsens:uptime | Device uptime in seconds since boot |
| sensor:radsens:accumulated_dose_ur | Integrated dose since boot in micro-roentgen |
| sensor:radsens:accumulated_dose_msv | Integrated dose since boot converted to mSv |
| number:radsens | Needed to enable interval control from Home Assistant |
| number:radsens:polling_interval | Poll interval in seconds (5..300). Value is applied immediately and restored after reboot |
| switch:radsens | Needed to enable the switches |
| switch:radsens:control_led | Controls if the blue led which flashes when an impulse is detected (default on) |
| switch:radsens:control_high_voltage | Controls if the high voltage generator is on.  When turning off it takes a while to change readings (~5min for me) but it will go to zero (default on) |
| switch:radsens:control_low_power | Sets a low power mode which turns off the tube power from time to time (default off) |

## Notes
  * I did see a failure to communicate with the device when setting the high voltage switch at startup but can't repro. 
  If you see this file a bug.
  * The endienness is different for some different i2c values, I think I got all of these right but dont be confused by that in code.
  This was a little surprising at first but I found it matched the arduino code.
  * Reading the dynamic and static intensity also reset the CPM counter.  Given I read them in the update loop I just read all three
  sequentialy with CPM first.  The library for arduino keeps a CPM counter and reads the CPM value before reading anything
  that seems to reset the CPM value.  There is a little bit of time that is probably lost but that is probably still true in
  the arduino code.
  * This is my first external component, sensors & switches in the same component took some time but I think are correct.
  if you see something wrong file an issue or a PR.
  * The switch restoration I have the least confidence in but it is working in my testing.
  * It might make sense to convert the exposure numbers from rostom to millisevert
  * The [english documents](https://github.com/climateguard/RadSens/blob/master/extras/datasheets/RadSens_datasheet_ENG.pdf) are not complete.  I used a translated Russian docs ([4v0](Radsens_datasheet_RU_1v3-EN.pdf) ,[1v3](Radsens_datasheet_RU_4v0-EN.pdf)) and the [arduino library](https://github.com/climateguard/RadSens/tree/master) as sources.


## Other RadSens software
* https://github.com/climateguard/RadSens/tree/master
* https://github.com/maaad/RadSens1v2

## Grafana dashboard (InfluxDB)

- Ready dashboard JSON: [`grafana/radsens-dashboard.json`](grafana/radsens-dashboard.json)
- Import in Grafana: **Dashboards → Import → Upload JSON file**.
- Select your InfluxDB datasource when prompted.

Notes:
- Queries are built for InfluxDB measurement `state` with tag `entity_id` (HA default style).
- Default entity IDs are based on this project example naming.
- If your entity IDs differ, edit constants in dashboard variables (e.g. `cpm_entity`, `cpp_entity`, `dose_msv_entity`) after import.
