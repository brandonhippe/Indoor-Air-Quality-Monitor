#ifndef _CGANEM_H_
#define _CGANEM_H_
/*Warning! After the power on the sensor need 10-15 seconds for heating. During that period, module doesn't' update data*/
#include <NewWire.h>
#include <stdint.h>
#include "Energia.h"
/*I2C set*/
#define ANEM_I2C_ADDR 0x11
/*I2C set*/

/*I2C REGISTERS ADDRESSES begin*/
#define i2c_reg_FACTORY_ID_1 0x00
#define i2c_reg_FACTORY_ID_2 0x01
#define i2c_reg_FACTORY_ID_3 0x02
#define i2c_reg_FACTORY_ID_4 0x03
#define i2c_reg_VERSION 0x04
#define i2c_reg_WHO_I_AM 0x05
#define i2c_reg_STATUS 0x06
#define i2c_reg_WIND_H 0x07
#define i2c_reg_WIND_L 0x08
#define i2c_reg_TEST_COLD_H 0x09
#define i2c_reg_TEST_COLD_L 0x0A
#define i2c_reg_TEST_HOT_H 0x0B
#define i2c_reg_TEST_HOT_L 0x0C
#define i2c_reg_SUPPLY_V 0x0D
#define i2c_reg_PWR_WT 0x0E
#define i2c_reg_TEMP_COLD_H 0x10
#define i2c_reg_TEMP_COLD_L 0x11
#define i2c_reg_TEMP_HOT_H 0x12
#define i2c_reg_TEMP_HOT_L 0x13
#define i2c_reg_dT_H 0x14
#define i2c_reg_dT_L 0x15
#define i2c_reg_ADDRESS 0x20
#define i2c_reg_WIND_MAX_H 0x21
#define i2c_reg_WIND_MAX_L 0x22
#define i2c_reg_WIND_MIN_H 0x23
#define i2c_reg_WIND_MIN_L 0x24
#define i2c_reg_RESET_WIND 0x25

/*I2C REGISTERS ADDRESSES end*/
#define STUP 0x0  // bit num - unsteady process

/*STATUS REGISTER BITS end*/


// Next function definitions
#define WAKEUP 0
#define TAKE_MEASUREMENT 1


class CG_Anem
{
private:
    uint8_t _sensor_address;
    uint8_t _chip_id;
    uint8_t _firmware_ver;
    
    // Additional variables / functions for IAQ node
    uint64_t lastMeasurement;
    int scheduledFunc;

public:
    // Additional variables / functions for IAQ Node
    uint32_t max_clock;
    uint64_t period_ms, time_ms;
    boolean measurement_ready, measurement_started, debug;
    int sleep_pin;
    void startNextFunc(uint64_t currTime_ms);
    void cg_sleep();
    void cg_wakeup();
    void setPeriod(uint64_t newPeriod_ms);

    CG_Anem();

    // Fields of data, for update use data_update() function
    float temperature;    // temperature
    float airConsumption; // flow consumption
    float airflowRate;    // flow rate
    float ductArea;              // duct area in sm^2. Necessary to set a value in the main code for air flow calculations

    // Methods for get or set data
    bool begin(int sleepPin, boolean _debug);
    bool data_update(void);
    uint8_t getChipId();
    float getFirmwareVersion();
    float getTemperature();
    float getAirflowRate();
    void set_duct_area(float area);
    float calculateAirConsumption();
    bool getSensorStatus();
    bool register_read_byte(uint8_t regAddr, uint8_t *retrieveData);
    bool register_write_byte(uint8_t regAddr, uint8_t regData);
    bool setI2Caddr(uint8_t newAddr);
    bool resetMinMaxValues();
    float getMaxAirFlowRate();
    float getMinAirFlowRate();
};

#endif // _CGANEM_H_
