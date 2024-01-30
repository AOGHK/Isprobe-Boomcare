#define DEBUG_LOG 1

// GPIO
#define PW_STA_PIN 7
#define PW_BTN_PIN 10
#define PW_CTRL_PIN 8
#define BOT_TC_PIN 20  // TC1
#define TOP_TC_PIN 21  // TC2
#define SDA_PIN 4
#define SCL_PIN 5

// BLE
#define BLEC_AD_DELAY 100
#define BLES_AD_DELAY 100
#define BLEC_SCAN_SEC 5
#define BLEC_SCAN_DELAY 100
#define BLEC_CONN_TIMEOUT 5000

// WiFi
#define WIFI_STA_SYNC_TIMER 1000
#define WIFI_CONNECT_ERR_CNT 5
#define WIFI_PING_TIMER (1000 * 60)

// LED
#define PW_LED_PIN 3
#define RED_LED_PIN 1
#define GREEN_LED_PIN 2
#define BLUE_LED_PIN 0
#define STA_LED_PIN 6
#define RGB_LED_FREQ 5000  // (1000000 * 20)
#define RGB_LED_BIT 8
#define THEME_SIZE 5
#define LED_MIN_BRIGHTNESS 100
#define LED_MAX_BRIGHTNESS 255
#define THERMO_LIGHT_TIMEOUT 3000
#define STA_LED_BRIGHTNESS 64
#define CTRL_STEP_SIZE 5

// BATTERY
#define LOW_BATTERY_LIMIT 10
#define LOW_BAT_BLINK_SIZE 4
#define LOW_BAT_BLINK_DELAY 200