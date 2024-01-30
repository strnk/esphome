#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/display/display_buffer.h"

namespace esphome {
namespace waveshare_epaper {

class WaveshareEPaper : public display::DisplayBuffer,
                        public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                                              spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_2MHZ> {
 public:
  void set_dc_pin(GPIOPin *dc_pin) { dc_pin_ = dc_pin; }
  float get_setup_priority() const override;
  void set_reset_pin(GPIOPin *reset) { this->reset_pin_ = reset; }
  void set_busy_pin(GPIOPin *busy) { this->busy_pin_ = busy; }
  void set_reset_duration(uint32_t reset_duration) { this->reset_duration_ = reset_duration; }

  void command(uint8_t value);
  void data(uint8_t value);

  virtual void display() = 0;
  virtual void initialize() = 0;
  virtual void deep_sleep() = 0;

  void update() override;

  void fill(Color color) override;

  void setup() {
    this->setup_pins_();
    this->initialize();
  }

  void on_safe_shutdown() override;

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_BINARY; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  virtual bool wait_until_idle_();

  void setup_pins_();

  virtual void reset_() {
    if (this->reset_pin_ != nullptr) {
      this->reset_pin_->digital_write(false);
      delay(reset_duration_);  // NOLINT
      this->reset_pin_->digital_write(true);
      delay(200);  // NOLINT
    }
  }

  virtual int get_width_controller() { return this->get_width_internal(); };

  virtual uint32_t get_buffer_length_();
  uint32_t reset_duration_{200};

  void start_command_();
  void end_command_();
  void start_data_();
  void end_data_();

  GPIOPin *reset_pin_{nullptr};
  GPIOPin *dc_pin_;
  GPIOPin *busy_pin_{nullptr};
  virtual uint32_t idle_timeout_() { return 1000u; }  // NOLINT(readability-identifier-naming)
};

enum WaveshareEPaperTypeAModel {
  WAVESHARE_EPAPER_1_54_IN = 0,
  WAVESHARE_EPAPER_1_54_IN_V2,
  WAVESHARE_EPAPER_2_13_IN,
  WAVESHARE_EPAPER_2_9_IN,
  WAVESHARE_EPAPER_2_9_IN_V2,
  TTGO_EPAPER_2_13_IN,
  TTGO_EPAPER_2_13_IN_B73,
  TTGO_EPAPER_2_13_IN_B1,
  TTGO_EPAPER_2_13_IN_B74,
};

class WaveshareEPaperTypeA : public WaveshareEPaper {
 public:
  WaveshareEPaperTypeA(WaveshareEPaperTypeAModel model);

  void initialize() override;

  void dump_config() override;

  void display() override;

  void deep_sleep() override {
    switch (this->model_) {
      // Models with specific deep sleep command and data
      case WAVESHARE_EPAPER_1_54_IN:
      case WAVESHARE_EPAPER_1_54_IN_V2:
      case WAVESHARE_EPAPER_2_9_IN_V2:
        // COMMAND DEEP SLEEP MODE
        this->command(0x10);
        this->data(0x01);
        break;
      // Other models default to simple deep sleep command
      default:
        // COMMAND DEEP SLEEP
        this->command(0x10);
        break;
    }
    this->wait_until_idle_();
  }

  void set_full_update_every(uint32_t full_update_every);

 protected:
  void write_lut_(const uint8_t *lut, uint8_t size);

  void init_display_();

  int get_width_internal() override;

  int get_height_internal() override;

  int get_width_controller() override;

  uint32_t full_update_every_{30};
  uint32_t at_update_{0};
  WaveshareEPaperTypeAModel model_;
  uint32_t idle_timeout_() override;

  bool deep_sleep_between_updates_{false};
};

enum WaveshareEPaperTypeBModel {
  WAVESHARE_EPAPER_2_7_IN = 0,
  WAVESHARE_EPAPER_4_2_IN,
  WAVESHARE_EPAPER_4_2_IN_B_V2,
  WAVESHARE_EPAPER_7_5_IN,
  WAVESHARE_EPAPER_7_5_INV2,
  WAVESHARE_EPAPER_7_5_IN_B_V2,
};

class WaveshareEPaper2P7In : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper2P7InV2 : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override { ; }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class GDEY029T94 : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class GDEW0154M09 : public WaveshareEPaper {
 public:
  void initialize() override;
  void display() override;
  void dump_config() override;
  void deep_sleep() override;

 protected:
  int get_width_internal() override;
  int get_height_internal() override;

 private:
  static const uint8_t CMD_DTM1_DATA_START_TRANS = 0x10;
  static const uint8_t CMD_DTM2_DATA_START_TRANS2 = 0x13;
  static const uint8_t CMD_DISPLAY_REFRESH = 0x12;
  static const uint8_t CMD_AUTO_SEQ = 0x17;
  static const uint8_t DATA_AUTO_PON_DSR_POF_DSLP = 0xA7;
  static const uint8_t CMD_PSR_PANEL_SETTING = 0x00;
  static const uint8_t CMD_UNDOCUMENTED_0x4D = 0x4D;  //  NOLINT
  static const uint8_t CMD_UNDOCUMENTED_0xAA = 0xaa;  //  NOLINT
  static const uint8_t CMD_UNDOCUMENTED_0xE9 = 0xe9;  //  NOLINT
  static const uint8_t CMD_UNDOCUMENTED_0xB6 = 0xb6;  //  NOLINT
  static const uint8_t CMD_UNDOCUMENTED_0xF3 = 0xf3;  //  NOLINT
  static const uint8_t CMD_TRES_RESOLUTION_SETTING = 0x61;
  static const uint8_t CMD_TCON_TCONSETTING = 0x60;
  static const uint8_t CMD_CDI_VCOM_DATA_INTERVAL = 0x50;
  static const uint8_t CMD_POF_POWER_OFF = 0x02;
  static const uint8_t CMD_DSLP_DEEP_SLEEP = 0x07;
  static const uint8_t DATA_DSLP_DEEP_SLEEP = 0xA5;
  static const uint8_t CMD_PWS_POWER_SAVING = 0xe3;
  static const uint8_t CMD_PON_POWER_ON = 0x04;
  static const uint8_t CMD_PTL_PARTIAL_WINDOW = 0x90;

  uint8_t *lastbuff_ = nullptr;
  void reset_();
  void clear_();
  void write_init_list_(const uint8_t *list);
  void init_internal_();
};

class WaveshareEPaper2P9InB : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper2P9InBV3 : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper4P2In : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND VCOM AND DATA INTERVAL SETTING
    this->command(0x50);
    this->data(0x17);  // border floating

    // COMMAND VCM DC SETTING
    this->command(0x82);
    // COMMAND PANEL SETTING
    this->command(0x00);

    delay(100);  // NOLINT

    // COMMAND POWER SETTING
    this->command(0x01);
    this->data(0x00);
    this->data(0x00);
    this->data(0x00);
    this->data(0x00);
    this->data(0x00);
    delay(100);  // NOLINT

    // COMMAND POWER OFF
    this->command(0x02);
    this->wait_until_idle_();
    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper4P2InBV2 : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND VCOM AND DATA INTERVAL SETTING
    this->command(0x50);
    this->data(0xF7);  // border floating

    // COMMAND POWER OFF
    this->command(0x02);
    this->wait_until_idle_();

    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check code
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper5P8In : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND POWER OFF
    this->command(0x02);
    this->wait_until_idle_();
    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper5P8InV2 : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND VCOM AND DATA INTERVAL SETTING
    this->command(0x50);
    this->data(0x17);  // border floating

    // COMMAND VCM DC SETTING
    this->command(0x82);
    // COMMAND PANEL SETTING
    this->command(0x00);

    delay(100);  // NOLINT

    // COMMAND POWER SETTING
    this->command(0x01);
    this->data(0x00);
    this->data(0x00);
    this->data(0x00);
    this->data(0x00);
    this->data(0x00);
    delay(100);  // NOLINT

    // COMMAND POWER OFF
    this->command(0x02);
    this->wait_until_idle_();
    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper7P5In : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND POWER OFF
    this->command(0x02);
    this->wait_until_idle_();
    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper7P5InBV2 : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND POWER OFF
    this->command(0x02);
    this->wait_until_idle_();
    // COMMAND DEEP SLEEP
    this->command(0x07);  // deep sleep
    this->data(0xA5);     // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper7P5InBV3 : public WaveshareEPaper {
 public:
  bool wait_until_idle_();

  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    this->command(0x02);  // Power off
    this->wait_until_idle_();
    this->command(0x07);  // Deep sleep
    this->data(0xA5);
  }

  void clear_screen();

 protected:
  int get_width_internal() override;

  int get_height_internal() override;

  void reset_() {
    if (this->reset_pin_ != nullptr) {
      this->reset_pin_->digital_write(true);
      delay(200);  // NOLINT
      this->reset_pin_->digital_write(false);
      delay(5);
      this->reset_pin_->digital_write(true);
      delay(200);  // NOLINT
    }
  };

  void init_display_();
};

class WaveshareEPaper7P5InBC : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND POWER OFF
    this->command(0x02);
    this->wait_until_idle_();
    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper7P5InV2 : public WaveshareEPaper {
 public:
  bool wait_until_idle_();

  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND POWER OFF
    this->command(0x02);
    this->wait_until_idle_();
    // COMMAND DEEP SLEEP
    this->command(0x07);
    this->data(0xA5);  // check byte
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;

  uint32_t idle_timeout_() override;
};

class WaveshareEPaper7P5InV2alt : public WaveshareEPaper7P5InV2 {
 public:
  bool wait_until_idle_();
  void initialize() override;
  void dump_config() override;

 protected:
  void reset_() {
    if (this->reset_pin_ != nullptr) {
      this->reset_pin_->digital_write(true);
      delay(200);  // NOLINT
      this->reset_pin_->digital_write(false);
      delay(2);
      this->reset_pin_->digital_write(true);
      delay(20);
    }
  };
};

class WaveshareEPaper7P5InHDB : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // deep sleep
    this->command(0x10);
    this->data(0x01);
  }

 protected:
  int get_width_internal() override;

  int get_height_internal() override;
};

class WaveshareEPaper2P13InDKE : public WaveshareEPaper {
 public:
  void initialize() override;

  void display() override;

  void dump_config() override;

  void deep_sleep() override {
    // COMMAND POWER DOWN
    this->command(0x10);
    this->data(0x01);
    // cannot wait until idle here, the device no longer responds
  }

  void set_full_update_every(uint32_t full_update_every);

 protected:
  int get_width_internal() override;

  int get_height_internal() override;

  uint32_t idle_timeout_() override;

  uint32_t full_update_every_{30};
  uint32_t at_update_{0};
};

enum WaveshareEPaperSSD1681Model {
  WAVESHARE_EPAPER_1_54_IN_B_V2
};

namespace SSD1681
{
  typedef enum Command  {
    DRIVER_OUTPUT_CONTROL = 0x01,
    // 0x02
    GATE_DRIVING_VOLTAGE_CONTROL = 0x03,
    SOURCE_DRIVING_VOLTAGE_CONTROL = 0x04,
    // 0x04 - 0x07
    PROGRAM_OTP_INITIAL_CODE_SETTING = 0x08,
    WRITE_INITIAL_CODE_SETTING_REGISTER = 0x09,
    READ_INITIAL_CODE_SETTING_REGISTER = 0x0A,
    // 0x0B
    BOOSTER_SOFT_START_CONTROL = 0x0C,
    // 0x0D - 0x0F
    DEEP_SLEEP_MODE = 0x10,
    DATA_ENTRY_MODE_SETTING = 0x11,
    SW_RESET = 0x12,
    // 0x13
    HV_READY_DETECTION = 0x14,
    VCI_DETECTION = 0x15,
    // 0x16-0x17
    TEMPERATURE_SENSOR_CONTROL = 0x18, 
    // 0x19
    TEMPERATURE_REGISTER_WRITE = 0x1A,
    TEMPERATURE_REGISTER_READ = 0x1B,
    TEMPERATURE_EXT_SENSOR_WRITE = 0x1C,
  // 0x1D-0x1F
    MASTER_ACTIVATION = 0x20,
    DISPLAY_UPDATE_CONTROL_1 = 0x21,
    DISPLAY_UPDATE_CONTROL_2 = 0x22,
    // 0x23
    WRITE_RAM_BW = 0x24,
    // 0x25
    WRITE_RAM_RED = 0x26,
    READ_RAM = 0x27,
    VCOM_SENSE = 0x28,
    VCOM_SENSE_DURATION = 0x29,
    PROGRAM_VCOM_OTP = 0x2A,
    WRITE_VCOM_CONTROL_REGISTER = 0x2B,
    WRITE_VCOM_REGISTER = 0x2C,
    READ_OTP_DISPLAY_OPTION_REGISTER = 0x2D,
    READ_OTP_USER_ID_REGISTER = 0x2E,
    READ_STATUS_BIT = 0x2F,
    PROGRAM_WS_OTP = 0x30,
    LOAD_WS_OTP = 0x31,
    WRITE_LUT_REGISTER = 0x32,
    // 0x33
    CRC_CALCULATION = 0x34,
    CRC_STATUS_READ = 0x35,
    PROGRAM_OTP_SELECTION = 0x36,
    WRITE_OTP_DISPLAY_OPTION_REGISTER = 0x37,
    WRITE_OTP_USE_ID_REGISTER = 0x38,
    PROGRAM_OTP_MOD = 0x39,
    // 0x3A-0x3B
    BORDER_WAVEFORM_CONTROL = 0x3C,
    // 0x3D-0x3E
    END_OPTION = 0x3F,
    // 0x40
    READ_RAM_OPTION = 0x41,
    // 0x42-0x43
    SET_RAM_X_ADDR_START_END = 0x44,
    SET_RAM_Y_ADDR_START_END = 0x45,
    AUTO_WRITE_RAM_RED = 0x46,
    AUTO_WRITE_RAM_BW = 0x47,
    // 0x48-0x4D
    SET_RAM_X_ADDR_COUNTER = 0x4E,
    SET_RAM_Y_ADDR_COUNTER = 0x4F,
    // 0x50-0x7E
    NOP,
    HW_RESET = 0xFF
  } Command_t; // enum Command

  typedef union LUT_u
  {
    struct 
    {
      union
      {
          struct
          {
              union {
                  struct {
                      uint8_t A : 2;
                      uint8_t B : 2;
                      uint8_t C : 2;
                      uint8_t D : 2;
                  } phase;

                  uint8_t raw;
              } group[12];
          } VS[5];

          uint8_t raw[60];
      } VS;
      
      struct
      {
        uint8_t TPA;
        uint8_t TPB;
        uint8_t SRAB;
        uint8_t TPC;
        uint8_t TPD;
        uint8_t SRCD;
        uint8_t RP;
      } group[12];

      union
      {
          struct
          {
              uint8_t FR0 : 4;
              uint8_t FR1 : 4;
              uint8_t FR2 : 4;
              uint8_t FR3 : 4;
              uint8_t FR4 : 4;
              uint8_t FR5 : 4;
              uint8_t FR6 : 4;
              uint8_t FR7 : 4;
              uint8_t FR8 : 4;
              uint8_t FR9 : 4;
              uint8_t FR10 : 4;
              uint8_t FR11 : 4;
          };

          uint8_t raw[6];
      } FR;

      union
      {
          struct 
          {
              uint8_t AB0 : 1;
              uint8_t CD0 : 1;
              uint8_t AB1 : 1;
              uint8_t CD1 : 1;
              uint8_t AB2 : 1;
              uint8_t CD2 : 1;
              uint8_t AB3 : 1;
              uint8_t CD3 : 1;
              uint8_t AB4 : 1;
              uint8_t CD4 : 1;
              uint8_t AB5 : 1;
              uint8_t CD5 : 1;
              uint8_t AB6 : 1;
              uint8_t CD6 : 1;
              uint8_t AB7 : 1;
              uint8_t CD7 : 1;
              uint8_t AB8 : 1;
              uint8_t CD8 : 1;
              uint8_t AB9 : 1;
              uint8_t CD9 : 1;
              uint8_t AB10 : 1;
              uint8_t CD10 : 1;
              uint8_t AB11 : 1;
              uint8_t CD11 : 1;
          };

          uint8_t raw[2];
      } XON;
      
      uint8_t EOPT;
      uint8_t VGH;
      uint8_t VSH1;
      uint8_t VSH2;
      uint8_t VSL;
      uint8_t VCOM;
    };
    
    uint8_t raw[159];
  } LUT_t;

}; // namespace SSD1681

class WaveshareEPaperSSD1681 : public WaveshareEPaper {
 public:
  WaveshareEPaperSSD1681(WaveshareEPaperSSD1681Model model);

  void initialize() override;
  void dump_config() override;
  void display() override;

  void deep_sleep() override {
    this->command(SSD1681::DEEP_SLEEP_MODE);
    this->data(0x01);
    delay(10);
  }

  void set_full_update_every(uint32_t full_update_every);

  //  Display
  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }

 protected:
  void reset_() override {
    if (this->reset_pin_ != nullptr) {
      this->reset_pin_->digital_write(true);
      delay(10);  // NOLINT
      this->reset_pin_->digital_write(false);
      delay(this->reset_duration_);
      this->reset_pin_->digital_write(true);
      delay(10);  // NOLINT
    }
  }

  int get_width_internal() override;
  int get_height_internal() override;

  void get_buffer_states_for_color(Color& color, bool& bw, bool& thirdColor);

  void fill(Color color) override;
  void HOT draw_absolute_pixel_internal(int x, int y, Color color) override;
  uint32_t get_buffer_length_() override;

  bool wait_until_idle_() override { return wait_until_idle_(SSD1681::NOP); }
  uint32_t idle_timeout_() override { return 10000; }

  bool wait_until_idle_(SSD1681::Command_t command);
  uint32_t idle_timeout_(SSD1681::Command_t command);

  void loadLUT(const SSD1681::LUT_t& lut);

  uint32_t init_start_ = 0xFFFFFFFF;
  uint32_t full_update_every_{30};
  uint32_t at_update_{0};
  WaveshareEPaperSSD1681Model model_;
  bool deep_sleep_between_updates_{false};
};

}  // namespace waveshare_epaper
}  // namespace esphome
