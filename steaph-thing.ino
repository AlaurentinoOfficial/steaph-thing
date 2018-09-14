#include <KNoTThing.h>
#include <EmonLib.h>

const int LIGHT_BULB_PINS[4] = {8, 9, 10, 11};
#define LIGHT_BULB_ID       1
#define LIGHT_BULB_NAME     "Light bulb"

EnergyMonitor emon1;
#define CURRENT_PIN    2
#define POWER_PIN      A0
#define POWER_ID       2
#define POWER_NAME     "Power sensor"
int voltage = 220;

KNoTThing thing;

static int light_read(uint8_t *val)
{
    *val = digitalRead(LIGHT_BULB_PINS[0]);
    
    Serial.print(F("Light Status: "));
    if (*val) Serial.println(F("ON"));
    else Serial.println(F("OFF"));
    
    return ;
}

static int light_write(uint8_t *val)
{
    for(int i = 0; i < sizeof(LIGHT_BULB_PINS)/sizeof(int); i++) {
      digitalWrite(LIGHT_BULB_PINS[i], *val);
    }
    
    Serial.print(F("Light Status: "));
    if (*val) Serial.println(F("ON"));
    else Serial.println(F("OFF"));
    
      /* TODO: Save light status in EEMPROM in to handle when reboot */
    
    return 0;
}


static int power_read(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{
    float power = float(emon1.calcIrms(1480)) * voltage;
    char *pwr = String(power).c_str();

    bool decimal = false;
    String buff_int = "";
    String buff_dec = "";

    for(int i = 0; sizeof(pwr)/sizeof(char); i++) {
        if(pwr[i] == '.') decimal = true;
        else if(!decimal) buff_int += pwr[i];
        else buff_dec += pwr[i];
    }
    
    *val_int = buff_int.toInt();
    *val_dec = buff_dec.toInt();
    
    Serial.print(F("Power: ")); Serial.print(power); Serial.print(F("W"));
    return 0;
}

static int power_write(int32_t *val_int, uint32_t *val_dec, int32_t *multiplier)
{   
    return 0;
}

void setup()
{
    Serial.begin(9600);

    // Config the Power Sensor
    emon1.current(POWER_PIN, 60);

    // Config the I/O
    pinMode(CURRENT_PIN, INPUT);
    for(int i = 0; i < sizeof(LIGHT_BULB_PINS)/sizeof(int); i++)
      pinMode(LIGHT_BULB_PINS[i], OUTPUT);

    // Verify if the corrent is 220 or 110
    if(digitalRead(CURRENT_PIN) == HIGH) voltage = 220;
    else voltage = 110;
    
    /* TODO: Read lamp status from eeprom for reboot cases */

    // Register the Schema of this device
    thing.init("KNoTThing");
    thing.registerBoolData(LIGHT_BULB_NAME, LIGHT_BULB_ID, KNOT_TYPE_ID_SWITCH,
        KNOT_UNIT_NOT_APPLICABLE, light_read, light_write); // Light Bulb
    thing.registerFloatData(POWER_NAME, POWER_ID, KNOT_TYPE_ID_POWER,
                    KNOT_UNIT_POWER_W, power_read, power_write); // Power sensor

    // Send data every 60 seconds
    thing.registerDefaultConfig(LIGHT_BULB_ID, KNOT_EVT_FLAG_TIME, 60, 0, 0, 0, 0);
    thing.registerDefaultConfig(POWER_ID, KNOT_EVT_FLAG_TIME, 60, 0, 0, 0, 0);

    Serial.println(F("Steaph - Smart Energy"));
}


void loop()
{
    thing.run();
}
