#include "polling_interval_control.h"
#include "esphome/core/log.h"

namespace esphome {
namespace radsens {

static const char *const TAG = "radsens.polling_interval";

void PollingIntervalControl::setup() {
  uint32_t value;
  this->pref_ = global_preferences->make_preference<uint32_t>(this->get_preference_hash());
  if (!this->pref_.load(&value)) {
    value = this->parent_->get_polling_interval_seconds();
  }

  this->parent_->set_polling_interval_seconds(value);
  this->publish_state(this->parent_->get_polling_interval_seconds());
}

void PollingIntervalControl::control(float value) {
  uint32_t interval = static_cast<uint32_t>(value);
  this->parent_->set_polling_interval_seconds(interval);

  uint32_t effective_interval = this->parent_->get_polling_interval_seconds();
  this->publish_state(effective_interval);
  this->pref_.save(&effective_interval);
}

void PollingIntervalControl::dump_config() { LOG_NUMBER("  ", "Polling Interval", this); }

}  // namespace radsens
}  // namespace esphome

