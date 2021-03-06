#define DATETIME &timeinfo
#define BIRTHDAY &b_timeinfo
#define RETIREMENT &r_timeinfo
#define UPTIME &uptime_sec

#define MAX_YR 65535
#define MIN_YR 0

#define RET_AGE 65

#include <time.h>

// Datetime
volatile struct tm timeinfo;
// Birth date
volatile struct tm b_timeinfo;
// Retirement date
volatile struct tm r_timeinfo;
// Uptime in seconds
volatile uint32_t uptime_sec;
// Time to retirement
volatile uint32_t time_to_ret_sec;
// Boolean if person is retired or not
volatile uint8_t is_retired;

/**
 * Sets initial date, birth date and retirement date
 */
void DATE_init(void);
/** 
 * 
 * @param method "SET" or "GET"
 * @param type "DATETIME", "BIRTHDAY", "BACKLIGHT"
 * @param date Date to set in dd.MM.yyyy format
 * @param time Time to set in hh:mm:ss format
 * @return 0 if operation successful
 */
uint8_t DATE_handle_date_cmd(char *method, char *type, char *date, char *time);
/**
 * 
 * @param date Date to set in dd.MM.yyyy format
 * @param time Time to set in hh:mm:ss format
 * @param selected_tm Destination tm
 */
void DATE_update_date(char date[], char time[], struct tm *selected_tm);
/**
 * Checks if date exists and if its in dd.MM.yyyy format
 * @param date Date to check
 * @return 1 if valid, 0 if invalid
 */
uint8_t DATE_is_valid_date(char date[]);
/**
 * Checks if time is valid and in dd:mm:ss format
 * @param date Time to check
 * @return 1 if valid, 0 if invalid
 */
uint8_t DATE_is_valid_time(char time[]);
/**
 * Increments datetime and uptime by 1 second.
 * Decrements time to retire by 1 second.
 */
void DATE_incr_one_sec();
/**
 * Gets uptime counter in format: dDhhHmmMssS
 * @param dest Destination string
 */
void DATE_get_uptime(char *dest);
/**
 * Gets countdown to retirement in format: dDhhHmmMssS
 * @param dest Destination string
 */
void DATE_get_countdown(char *dest);
/**
 * Retirement = birthday.years + RET_AGE
 * @param dest_tm Pointer to retirement timeinfo
 */
void DATE_calc_ret_date(struct tm *dest_tm);
/**
 * Converts seconds countdown format (dDhhHmmMssS) string
 * @param seconds Seconds to convert
 * @param dest Destination string
 */
void DATE_sec_to_countdown_format(uint32_t seconds, char *dest);
/**
 * Calculates difference between two datetimes in seconds
 * @param start
 * @param end
 * @return Difference in seconds
 */
uint32_t DATE_diff_in_seconds(struct tm *start, struct tm *end);
/**
 * Calculates new retirement date based on birth date. Also updates countdown
 * to retirement based on datetime.
 */
void DATE_update_ret_date(void);