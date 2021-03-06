#include <time.h>
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "date.h"
#include "../USART/usart.h"
#include "../RTC/rtc.h"
#include "../LCD/lcd.h"



volatile struct tm timeinfo;  // Datetime
volatile struct tm b_timeinfo;  // Birth date
volatile struct tm r_timeinfo; // Retirement date
volatile uint32_t uptime_sec = 0; // Uptime in seconds
volatile uint32_t time_to_ret_sec; // Time to retirement in seconds
volatile uint8_t is_retired = 0; // 0 = not retired, 1 = yes retired

void DATE_init()
{    
    cli();
    // Init datetime
    timeinfo.tm_sec = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_hour = 12;
    timeinfo.tm_mday = 21;
    timeinfo.tm_mon = 12 - 1;
    timeinfo.tm_year = 2020 - 1900;
    // Init birth date
    b_timeinfo.tm_sec = 30;
    b_timeinfo.tm_min = 59;
    b_timeinfo.tm_hour = 23;
    b_timeinfo.tm_mday = 20;
    b_timeinfo.tm_mon = 12 - 1;
    b_timeinfo.tm_year = 1955 - 1900;
    // Init retirement (birth date + RET_AGE)
    DATE_update_ret_date();    
    sei();
}

void DATE_incr_one_sec()
{
    cli();
    uptime_sec++;
    time_to_ret_sec -= time_to_ret_sec == 0 ? 0:1;
    timeinfo.tm_sec++;
    // Handle time overflow
    if(timeinfo.tm_sec >= 60)
    {
        timeinfo.tm_sec = 0;
        timeinfo.tm_min++;
    }
    if(timeinfo.tm_min >= 60)
    {
        timeinfo.tm_min = 0;
        timeinfo.tm_hour++;
    }
    if(timeinfo.tm_hour >= 24)
    {
        timeinfo.tm_hour = 0;
        timeinfo.tm_mday++;
    }
    
    // Handle month day overflow
    if((timeinfo.tm_mon + 1) == 2) // Is February
    {
        if(is_leap_year(timeinfo.tm_year + 1900))
        {
            if(timeinfo.tm_mday > 29)
            {
                timeinfo.tm_mday = 1;
                timeinfo.tm_mon++;
            }
        }
        else if(timeinfo.tm_mday > 28)
        {
            timeinfo.tm_mday = 1;
            timeinfo.tm_mon++;
        }
    }
    else // Not February
    {
        if(timeinfo.tm_mday > 31)
        {
            timeinfo.tm_mday = 1;
            timeinfo.tm_mon++;
        }        
        // Months w/ 30 days
        switch (timeinfo.tm_mon + 1)
        {
            case 4: // Apr
            case 6: // Jun
            case 9: // Sep
            case 11: // Nov
                if(timeinfo.tm_mday > 30)
                {
                    timeinfo.tm_mday = 1;
                    timeinfo.tm_mon++;
                }
                break;
        }
    }
    
    // Handle month overflow
    if((timeinfo.tm_mon + 1) > 12)
    {
        // Set January first day
        timeinfo.tm_mday = 1;
        timeinfo.tm_mon = 1 - 1;
        timeinfo.tm_year++;
    }
    
    // Activate buzzer and change to retirement view
    if ((time_to_ret_sec == 0) && !is_retired)
    {
        is_retired = 1;
        PORTC.OUTSET = PIN4_bm;
        LCD_view = RETIREMENT_VIEW;
    }
    sei();
}

uint8_t DATE_handle_date_cmd(char *method, char *type, char *date, char *time)
{
    struct tm *selected_tm;
    if(strcmp(type, "DATETIME") == 0)
    {
        selected_tm = (struct tm*) &timeinfo;
    }
    else if(strcmp(type, "BIRTHDAY") == 0)
    {
        selected_tm = (struct tm*) &b_timeinfo;
    }
    else
    {
        return 1;
    }
    if(strcmp(method, "GET") == 0)
    {
        char msg_str[100];
        strftime(msg_str, sizeof(msg_str),
                "%d.%m.%Y %H:%M:%S\r\n", selected_tm);
        USART0_sendString(msg_str);
        return 0;
    }
    if(strcmp(method, "SET") == 0)
    {
        if(!DATE_is_valid_date(date))
        {
            return 1;
        }
        if((strcmp(type, "DATETIME") == 0) && !DATE_is_valid_time(time))
        {
            return 1;
        }
        DATE_update_date(date, time, selected_tm);
        DATE_update_ret_date();
        is_retired = time_to_ret_sec < 1;
        return 0;
    }
    return 1;
}

void DATE_update_date(char date[], char time[], struct tm *selected_tm)
{
    char* year_str = malloc(4);
    char* month_str = malloc(2);
    char* day_str = malloc(2);
    char* hour_str = malloc(2);
    char* min_str = malloc(2);
    char* sec_str = malloc(2);
    strncpy(year_str, date + 6, 4);
    strncpy(month_str, date + 3, 2);
    strncpy(day_str, date, 2);
    strncpy(hour_str, time, 2);
    strncpy(min_str, time + 3, 2);
    strncpy(sec_str, time + 6, 2);
    cli();
    selected_tm->tm_sec = atoi(sec_str);
    selected_tm->tm_min = atoi(min_str);
    selected_tm->tm_hour = atoi(hour_str);
    selected_tm->tm_mday = atoi(day_str);
    selected_tm->tm_mon = atoi(month_str) - 1;
    selected_tm->tm_year = atoi(year_str) - 1900;
    sei();
}

uint8_t DATE_is_valid_date(char date[])
{
    char* year_str = malloc(4);
    char* month_str = malloc(2);
    char* day_str = malloc(2);
    strncpy(year_str, date + 6, 4);
    strncpy(month_str, date + 3, 2);
    strncpy(day_str, date, 2);
    uint16_t year = atoi(year_str);
    uint8_t month = atoi(month_str);
    uint8_t day = atoi(day_str);

    // Check range of year, month and day
    if ((year > MAX_YR) || (year < MIN_YR))
    {
        return 0;
    }
    if ((month < 1) || (month > 12))
    {
        return 0;
    }
    if ((day < 1) || (day > 31))
    {
        return 0;
    }

    // Handle Feb days in leap year
    if (month == 2)
    {
        if (is_leap_year(year))
        {
            return (day <= 29);
        }
        return (day <= 28);
    }
    
    // Handle months w/ 30 days
    if ((month == 4) || (month == 6) ||
            (month == 9) || (month == 11))
    {
        return (month <= 30);
    }
    return 1;
}

uint8_t DATE_is_valid_time(char time[])
{
    char* hour_str = malloc(2);
    char* min_str = malloc(2);
    char* sec_str = malloc(2);
    strncpy(hour_str, time, 2);
    strncpy(min_str, time + 3, 2);
    strncpy(sec_str, time + 6, 2);
    uint8_t hour = atoi(hour_str);
    uint8_t min = atoi(min_str);
    uint8_t sec = atoi(sec_str);
    if(!((0 <= hour) && (hour <= 23)))
    {
        return 0;
    }
    if(!((0 <= min) && (min <= 59)))
    {
        return 0;
    }
    if(!((0 <= sec) && (sec <= 59)))
    {
        return 0;
    }
    return 1;
}

void DATE_get_uptime(char *dest)
{
    if(uptime_sec < 1)
    {
        return;
    }    
    cli();
    DATE_sec_to_countdown_format(uptime_sec, dest);
    sei();
}

void DATE_calc_ret_date(struct tm *dest_tm)
{
    dest_tm->tm_sec = 0;
    dest_tm->tm_min = 0;
    dest_tm->tm_hour = 0;
    dest_tm->tm_mday = b_timeinfo.tm_mday;
    dest_tm->tm_mon = b_timeinfo.tm_mon;
    dest_tm->tm_year = b_timeinfo.tm_year + RET_AGE;
}

void DATE_get_countdown(char *dest)
{    
    cli();
    DATE_sec_to_countdown_format(time_to_ret_sec, dest);
    sei();
}

void DATE_sec_to_countdown_format(uint32_t seconds, char *dest)
{
    uint32_t remaining_seconds = seconds;
    uint16_t days = remaining_seconds / 86400;
    remaining_seconds %= 86400;
    uint8_t hours = (remaining_seconds / 3600);
    remaining_seconds %= 3600;
    uint8_t minutes = (remaining_seconds / 60);
    remaining_seconds %= 60;    
    snprintf(dest, 100, "%02dD%02dH%02dM%02dS", 
            (int)days, (int)hours, (int)minutes, (int)remaining_seconds);
}

uint32_t DATE_diff_in_seconds(struct tm *start, struct tm *end)
{  
    uint32_t seconds = 0;
    uint8_t is_first_year = 1;
    uint8_t is_first_month = 1;
    uint16_t year = start->tm_year + 1900;
    // Loop years
    for(; year <= (end->tm_year + 1900); year++)
    {
        uint8_t is_last_year = year == (end->tm_year + 1900);
        uint8_t month = is_first_year ? (start->tm_mon + 1) : 1;
        uint8_t last_month = is_last_year ? (end->tm_mon + 1) : 12;
        // Loop months
        for(; month <= last_month; month++)
        {
            uint8_t days_in_month = 31;
            if(month == 2)
            {
                days_in_month = is_leap_year(year) ? 29 : 28;
            }
            if((month == 4) || (month == 6) || (month == 9) || (month == 11))
            {
                days_in_month = 30;
            }
            if(is_last_year && (month == last_month))
            {
                // Remove days from the end of the last month
                seconds -= (days_in_month * 86400) - 
                        ((end->tm_mday + 1) * 86400);
            }
            if(is_first_month)
            {
                // Remove days from the start of the first month
                days_in_month -= start->tm_mday + 1;
                // Remove extra hours, minutes and seconds
                uint32_t hours = start->tm_hour;
                seconds -= hours * 3600;
                uint16_t mins = start->tm_min;
                seconds -= mins * 60;
                seconds -= start->tm_sec;
                is_first_month = 0;
            }
            seconds += days_in_month * 86400;         
        }
        is_first_year = 0;
    }
    return seconds;
}

void DATE_update_ret_date()
{
    DATE_calc_ret_date((struct tm*) RETIREMENT);
    time_to_ret_sec = DATE_diff_in_seconds((struct tm*) DATETIME,
                                            (struct tm*) RETIREMENT);
}