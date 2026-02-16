#include "radsens.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/datatypes.h"
#include "esphome/core/helpers.h"

#ifdef USE_SWITCH
#include "switch/high_voltage_control.h"
#include "switch/low_power_control.h"
#include "switch/led_control.h"
#endif
#ifdef USE_NUMBER
#include "number/polling_interval_control.h"
#endif

namespace esphome {
namespace radsens {

static const char *const TAG = "radsens";
static const uint8_t RADSENS_DEVICE_ID = 0x7D;
static const uint8_t RADSENS_ADDRESS = 0x66;
static const uint8_t RADSENS_REGISTER_IDENTIFICATION = 0x00; // 8 bit
static const uint8_t RADSENS_REGISTER_IDENTIFICATION_FIRMWARE_VERSION = 0x01; // 8bit
static const uint8_t RADSENS_REGISTER_DATA_DYNAMIC_INTENSITY = 0x03; // 24bit
static const uint8_t RADSENS_REGISTER_DATA_STATIC_INTENSITY = 0x06; // 24bit
static const uint8_t RADSENS_REGISTER_DATA_PULSE_COUNTER= 0x09; // 16bit
static const uint8_t RADSENS_REGISTER_CONTROL_DEVICE_ADDRESS = 0x10; // 8bit
static const uint8_t RADSENS_REGISTER_CONTROL_HIGH_VOLTAGE_GENERATOR = 0x11; // 8bit
static const uint8_t RADSENS_REGISTER_CONTROL_SENSITIVITY = 0x12; // 16bit
static const uint8_t RADSENS_REGISTER_CONTROL_LED = 0x14;  // 8bit
static const uint8_t RADSENS_REGISTER_CONTROL_LOW_POWER_MODE = 0x0C; ///8bit

void RadSensComponent::set_control(uint8_t reg, uint8_t val){
  // I was expecting to need to read/save the count register here
  // it does not appear to be needed from experimentation
  if (!this->write_byte(reg, val)){
    ESP_LOGCONFIG(TAG, "RadSens Write: failed writing control register %u", reg);
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;    
  }
}

bool RadSensComponent::get_control(uint8_t reg){
  uint8_t val;
  if (!this->read_byte(reg, &val)){
    ESP_LOGCONFIG(TAG, "RadSens Write: failed reading control register %u", reg);
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return false;    
  }
  return val == 1 ? true : false;
}

void RadSensComponent::set_high_voltage(bool enable){
  set_control(RADSENS_REGISTER_CONTROL_HIGH_VOLTAGE_GENERATOR, enable);
}
bool RadSensComponent::get_high_voltage(){
  return get_control(RADSENS_REGISTER_CONTROL_HIGH_VOLTAGE_GENERATOR);
}

void RadSensComponent::set_led(bool enable){
  set_control(RADSENS_REGISTER_CONTROL_LED, enable);
}
bool RadSensComponent::get_led(){
  return get_control(RADSENS_REGISTER_CONTROL_LED);
}

void RadSensComponent::set_low_power(bool enable){
  set_control(RADSENS_REGISTER_CONTROL_LOW_POWER_MODE, enable);
}
bool RadSensComponent::get_low_power(){
  return get_control(RADSENS_REGISTER_CONTROL_LOW_POWER_MODE);
}

void RadSensComponent::setup() {
  ESP_LOGCONFIG(TAG, "RadSens Setup: starting...");
  uint8_t id;
  if (!this->read_byte(RADSENS_REGISTER_IDENTIFICATION, &id)) {
    ESP_LOGCONFIG(TAG, "RadSens Setup: failed reading id");
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }

  if (id != RADSENS_DEVICE_ID) {
    ESP_LOGCONFIG(TAG, "RadSens Setup: id wrong %u != %u", id, RADSENS_DEVICE_ID);
    this->error_code_ = ID_REGISTERS;
    this->mark_failed();
    return;
  }

  if (!this->read_byte(RADSENS_REGISTER_IDENTIFICATION_FIRMWARE_VERSION, &this->firmware_version)) {
    ESP_LOGCONFIG(TAG, "RadSens Setup: failed reading firmware version");
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }

  // The docs say this should be a 16 bit value defaulting to 105
  // however I read mine before writing to it and I got the wrong
  // byte order (it was 105, but backwards compared to the pulse
  // counter).  Referencing
  // https://github.com/climateguard/RadSens/blob/master/src/CG_RadSens.cpp
  // it does reverse count, but not sensitivity
  // Read in byte order from device (expecting little endian)
  Uint16 old_sensitivity;
  if (!this->read_bytes(RADSENS_REGISTER_CONTROL_SENSITIVITY, old_sensitivity.a8, 2)) {
    ESP_LOGCONFIG(TAG, "RadSens Setup: failed reading sensitivity");
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  // This number is little endian oddly, convert if needed
  old_sensitivity.u16 = convert_little_endian(old_sensitivity.u16); // No-Op on esp32
  ESP_LOGCONFIG(TAG, "RadSens setup: sensitivity is %u", old_sensitivity.u16);
  if (this->sensitivity_ != 0 && old_sensitivity.u16 != this->sensitivity_){
    ESP_LOGCONFIG(TAG, "RadSens setup: sensitivity setting to %u", this->sensitivity_);
    Uint16 sensitivity_to_send;
    sensitivity_to_send.u16 = convert_little_endian(this->sensitivity_);
    ESP_LOGCONFIG(TAG, "RadSens setup: writing %u %u", sensitivity_to_send.a8[0], sensitivity_to_send.a8[1]);
    if (!this->write_bytes(RADSENS_REGISTER_CONTROL_SENSITIVITY, sensitivity_to_send.a8, 2)) {
      ESP_LOGCONFIG(TAG, "RadSens Setup: failed writing sensitivity");
      this->error_code_ = COMMUNICATION_FAILED;
      this->mark_failed();
      return;
    }
  }

#ifdef USE_SWITCH
  if (this->control_low_power_switch_ != nullptr)
    static_cast<LowPowerControl*>(this->control_low_power_switch_)->setup();
  if (this->control_high_voltage_switch_ != nullptr)
    static_cast<HighVoltageControl*>(this->control_high_voltage_switch_)->setup();
  if (this->control_led_switch_ != nullptr)
    static_cast<LedControl*>(this->control_led_switch_)->setup();
#endif

#ifdef USE_NUMBER
  if (this->polling_interval_number_ != nullptr)
    static_cast<PollingIntervalControl*>(this->polling_interval_number_)->setup();
#endif

  this->start_millis_ = millis();
  this->total_cpp_ = 0;
  this->max_cpm_ = 0.0f;
  this->max_cpp_ = 0;
  this->max_cpm_timestamp_s_ = 0;
  this->max_cpp_timestamp_s_ = 0;
  this->accumulated_dose_ur_ = 0.0f;
  this->cpm_window_duration_ms_ = 0;
  this->cpm_window_total_counts_ = 0;
  this->last_update = 0;

  ESP_LOGCONFIG(TAG, "RadSens setup: Complete (firmware version %u)!", this->firmware_version);
}

void RadSensComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "RadSens:");
  LOG_I2C_DEVICE(this);
  if (this->error_code_ == COMMUNICATION_FAILED) {
    ESP_LOGE(TAG, "Communication with RadSens failed!");
  } else if (this->error_code_ == ID_REGISTERS) {
    ESP_LOGE(TAG, "The ID registers don't match - Is this really a RadSens?");
  }
  LOG_UPDATE_INTERVAL(this);

  LOG_SENSOR("  ", "Dynamic Intensity", this->dynamic_intensity_sensor_);
  LOG_SENSOR("  ", "Static Intensity", this->static_intensity_sensor_);
  LOG_SENSOR("  ", "Counts Per Polling", this->counts_per_polling_sensor_);
  LOG_SENSOR("  ", "Counts Per Minute", this->counts_per_minute_sensor_);
  LOG_SENSOR("  ", "RadSens Max CPM", this->radsens_max_cpm_sensor_);
  LOG_SENSOR("  ", "RadSens Max CPP", this->radsens_max_cpp_sensor_);
  LOG_SENSOR("  ", "Max CPM Timestamp", this->max_cpm_timestamp_sensor_);
  LOG_SENSOR("  ", "Max CPP Timestamp", this->max_cpp_timestamp_sensor_);
  LOG_SENSOR("  ", "Total CPP", this->total_cpp_sensor_);
  LOG_SENSOR("  ", "Uptime", this->uptime_sensor_);
  LOG_SENSOR("  ", "Accumulated Dose (uR)", this->accumulated_dose_ur_sensor_);
  LOG_SENSOR("  ", "Accumulated Dose (mSv)", this->accumulated_dose_msv_sensor_);
  LOG_SENSOR("  ", "Firmware Version", this->firmware_version_sensor_);

#ifdef USE_SWITCH
  LOG_SWITCH("  ", "High Voltage Switch", this->control_high_voltage_switch_);
  LOG_SWITCH("  ", "LED Switch", this->control_led_switch_);
  LOG_SWITCH("  ", "Low Power Switch", this->control_low_power_switch_);
#endif
}

float RadSensComponent::get_setup_priority() const { return setup_priority::DATA; }
void RadSensComponent::set_sensitivity(uint16_t sensitivity) { this->sensitivity_ = sensitivity; }

void RadSensComponent::set_polling_interval_seconds(uint32_t polling_interval_seconds) {
  if (polling_interval_seconds < 5) {
    polling_interval_seconds = 5;
  } else if (polling_interval_seconds > 300) {
    polling_interval_seconds = 300;
  }

  this->set_update_interval(polling_interval_seconds * 1000);
  this->cpm_window_duration_ms_ = 0;
  this->cpm_window_total_counts_ = 0;
  this->last_update = 0;
  ESP_LOGCONFIG(TAG, "RadSens polling interval set to %u s", polling_interval_seconds);
}

void RadSensComponent::update() {
  // must be zero'd as we write to the last 3 bytes
  // Union allows us to write as bytes then read / write as u32
  Uint32 raw_dynamic_intensity{}, raw_static_intensity{}; 
  uint16_t raw_counts;
  // NOTES: 
  // The manual says reading the rawcount resets it.
  // However it appears reading *any* I2C value resets the raw count
  // This reads the raw count first to make sure we don't lose that data 
  // I think we can still lose some between the count read and the intensity reads
  // but we do them as quickly as we can to minimize data loss 
  // TODO: see if we can keep the bus open?
  // The intensity values are 24bit uints that is why this writes 1 byte into a 32bit uint
  // we also have to fix endien
  uint32_t this_update = millis();
  if (!this->read_byte_16(RADSENS_REGISTER_DATA_PULSE_COUNTER, &raw_counts) ||
      !this->read_bytes(RADSENS_REGISTER_DATA_DYNAMIC_INTENSITY, &raw_dynamic_intensity.a8[1], 3) ||
      !this->read_bytes(RADSENS_REGISTER_DATA_STATIC_INTENSITY, &raw_static_intensity.a8[1], 3)){
    this->status_set_warning();
    return;
  }

  raw_dynamic_intensity.u32 = convert_big_endian(raw_dynamic_intensity.u32);
  raw_static_intensity.u32 = convert_big_endian(raw_static_intensity.u32);

  const float static_intensity_ur_h = raw_static_intensity.u32 * 0.1f;
  const uint32_t uptime_s = (this_update - this->start_millis_) / 1000;

  ESP_LOGD(TAG, "Got dynamic=%.1f static=%.1f counts=%d", 
                raw_dynamic_intensity.u32 * 0.1,
                raw_static_intensity.u32 * 0.1,
                raw_counts);

  if (this->counts_per_polling_sensor_ != nullptr) {
    this->counts_per_polling_sensor_->publish_state(raw_counts);
  }

  this->total_cpp_ += raw_counts;
  if (this->total_cpp_sensor_ != nullptr) {
    this->total_cpp_sensor_->publish_state(this->total_cpp_);
  }

  if (raw_counts > this->max_cpp_) {
    this->max_cpp_ = raw_counts;
    this->max_cpp_timestamp_s_ = uptime_s;
  }
  if (this->radsens_max_cpp_sensor_ != nullptr) {
    this->radsens_max_cpp_sensor_->publish_state(this->max_cpp_);
  }
  if (this->max_cpp_timestamp_sensor_ != nullptr) {
    this->max_cpp_timestamp_sensor_->publish_state(this->max_cpp_timestamp_s_);
  }

  if (this->uptime_sensor_ != nullptr) {
    this->uptime_sensor_->publish_state(uptime_s);
  }

  if (this->last_update != 0) {
    uint32_t elapsed_ms = this_update - this->last_update;
    if (elapsed_ms > 0) {
      this->accumulated_dose_ur_ += static_intensity_ur_h * (elapsed_ms / 3600000.0f);
      if (this->accumulated_dose_ur_sensor_ != nullptr) {
        this->accumulated_dose_ur_sensor_->publish_state(this->accumulated_dose_ur_);
      }
      if (this->accumulated_dose_msv_sensor_ != nullptr) {
        this->accumulated_dose_msv_sensor_->publish_state(this->accumulated_dose_ur_ * 0.00001f);
      }

      this->cpm_window_duration_ms_ += elapsed_ms;
      this->cpm_window_total_counts_ += raw_counts;

      if (this->cpm_window_duration_ms_ >= 60000) {
        float counts_per_minute = (this->cpm_window_total_counts_ * 60000.0f) / this->cpm_window_duration_ms_;

        if (counts_per_minute > this->max_cpm_) {
          this->max_cpm_ = counts_per_minute;
          this->max_cpm_timestamp_s_ = uptime_s;
        }

        if (this->counts_per_minute_sensor_ != nullptr) {
          this->counts_per_minute_sensor_->publish_state(counts_per_minute);
        }
        if (this->radsens_max_cpm_sensor_ != nullptr) {
          this->radsens_max_cpm_sensor_->publish_state(this->max_cpm_);
        }
        if (this->max_cpm_timestamp_sensor_ != nullptr) {
          this->max_cpm_timestamp_sensor_->publish_state(this->max_cpm_timestamp_s_);
        }

        this->cpm_window_duration_ms_ = 0;
        this->cpm_window_total_counts_ = 0;
      }
    }
  }
  this->last_update = this_update;

  if (this->dynamic_intensity_sensor_ != nullptr)
    this->dynamic_intensity_sensor_->publish_state(raw_dynamic_intensity.u32 * 0.1);
  if (this->static_intensity_sensor_ != nullptr)
    this->static_intensity_sensor_->publish_state(raw_static_intensity.u32 * 0.1);

  if (this->firmware_version_sensor_ != nullptr)
    this->firmware_version_sensor_->publish_state(this->firmware_version);

}

}  // namespace radsens
}  // namespace esphome
