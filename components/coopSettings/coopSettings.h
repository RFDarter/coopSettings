#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"
#include <cstring>

namespace esphome {
namespace coopSettings {

typedef struct RoomSettings
{
    unsigned long timeLightsOn;             //The time when the lights should be turned on         
    unsigned long timeLightsStartDimming;   //The time when the lights should be started to fade out           
    unsigned long timeLightsOff;            //The time when the lights should be turned off 
    bool isLightsOnByDimm;             //Are the lights turned on by the software?
    bool isLightsOnByUser;             //Are the lights turned on by the user?
    bool isDimmActive;                 //Is the Dimming active for this room?
    unsigned long dimmLevel;                //The bightness of the light represented by a number from 0 off to 65,535 full brighness
} RoomSettings;

typedef struct CoopSettings
{  
    unsigned long moonLightDimmLevel;
    unsigned long fullBrightnessDimmLevel;
    /*Room Settings*/
    RoomSettings room[2];
} CoopSettings;

class CoopSettingsComponent : public Component {
 public:
  using value_type = CoopSettings;
  explicit GlobalsComponent() = default;
  explicit GlobalsComponent(T initial_value) : value_(initial_value) {}
  explicit GlobalsComponent(std::array<typename std::remove_extent<CoopSettings>::type, std::extent<CoopSettings>::value> initial_value) {
    memcpy(this->value_, initial_value.data(), sizeof(CoopSettings));
  }

  CoopSettings &value() { return this->value_; }
  void setup() override {}

 protected:
  CoopSettings value_{};
};

class RestoringCoopSettingsComponent : public Component {
 public:
  using value_type = CoopSettings;
  explicit RestoringGlobalsComponent() = default;
  explicit RestoringGlobalsComponent(CoopSettings initial_value) : value_(initial_value) {}
  explicit RestoringGlobalsComponent(
      std::array<typename std::remove_extent<CoopSettings>::type, std::extent<CoopSettings>::value> initial_value) {
    memcpy(this->value_, initial_value.data(), sizeof(CoopSettings));
  }

  CoopSettings &value() { return this->value_; }

  void setup() override {
    this->rtc_ = global_preferences->make_preference<T>(1944399030U ^ this->name_hash_);
    this->rtc_.load(&this->value_);
    memcpy(&this->prev_value_, &this->value_, sizeof(CoopSettings));
  }

  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void loop() override { store_value_(); }

  void on_shutdown() override { store_value_(); }

  void set_name_hash(uint32_t name_hash) { this->name_hash_ = name_hash; }

 protected:
  void store_value_() {
    int diff = memcmp(&this->value_, &this->prev_value_, sizeof(CoopSettings));
    if (diff != 0) {
      this->rtc_.save(&this->value_);
      memcpy(&this->prev_value_, &this->value_, sizeof(CoopSettings));
    }
  }

  CoopSettings value_{};
  CoopSettings prev_value_{};
  uint32_t name_hash_{};
  ESPPreferenceObject rtc_;
};
CoopSettings &id(GlobalsComponent<CoopSettings> *value) { return value->value(); }
CoopSettings &id(RestoringGlobalsComponent<CoopSettings> *value) { return value->value(); }
//CoopSettings &id(RestoringGlobalStringComponent<T, SZ> *value) { return value->value(); }

}  // namespace globals
}  // namespace esphome