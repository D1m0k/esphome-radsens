#pragma once

#include "esphome/components/number/number.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "../radsens.h"

namespace esphome {
namespace radsens {

class PollingIntervalControl : public number::Number, public Parented<RadSensComponent> {
 public:
  PollingIntervalControl() = default;
  void setup();
  void dump_config();

 protected:
  void control(float value) override;
  ESPPreferenceObject pref_;
};

}  // namespace radsens
}  // namespace esphome

