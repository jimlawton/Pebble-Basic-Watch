#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "xprintf.h"
#include "my_math.h"
#include "suncalc.h"
#include "config.h"


#define MY_UUID { 0x3F, 0xED, 0x37, 0x37, 0xBA, 0x5D, 0x40, 0x15, 0x87, 0x7B, 0x76, 0x27, 0x2C, 0xED, 0xB4, 0x19 }
PBL_APP_INFO(MY_UUID,
             "Basic Watch", "Paul McNeil",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
TextLayer timeLayer; // The clock
TextLayer hourLayer; //hour
TextLayer minuteLayer;
TextLayer secondLayer;
TextLayer amLayer;
TextLayer dayLayer;
TextLayer monthLayer;
TextLayer riseLayer; // sunrise
TextLayer setLayer; // sunset
TextLayer moonLayer; // moon phase
#define HOUR_VIBRATION_START 8
#define HOUR_VIBRATION_END 0
	
GFont font_moon;
short currentData = -1;
/////moon phase/time stuff///////////////
//is it daylight time?
//need a better way
void adjustTimezone(float* time) 
{
  *time += TIMEZONE;
  if (*time > 24) *time -= 24;
  if (*time < 0) *time += 24;
}


//If 12 hour time, subtract 12 from hr if hr > 12
int thr(int hr)
{
    return !clock_is_24h_style() && (hr > 12) ? hr - 12 : hr;
}

//return julian day number for time
int tm2jd(PblTm *time)
{
    int y,m;
    y = time->tm_year + 1900;
    m = time->tm_mon + 1;
    return time->tm_mday-32075+1461*(y+4800+(m-14)/12)/4+367*(m-2-(m-14)/12*12)/12-3*((y+4900+(m-14)/12)/100)/4;
}
int moon_phase(int jdn)
{
    double jd;
    jd = jdn-2451550.1;
    jd /= 29.530588853;
    jd -= (int)jd;
    return (int)(jd*27 + 0.5); /* scale fraction from 0-27 and round by adding 0.5 */
}                          /* 0 = new, 14 = full */
void handle_day(AppContextRef ctx, PebbleTickEvent *t) {

    (void)t;
    (void)ctx;


    static char moon[] = "m";
    int moonphase_number = 0;

    PblTm *time = t->tick_time;
    if(!t)
        get_time(time);

    // moon
    moonphase_number = moon_phase(tm2jd(time));
    // correct for southern hemisphere
    if ((moonphase_number > 0) && (LATITUDE < 0))
        moonphase_number = 28 - moonphase_number;
    // select correct font char
    if (moonphase_number == 14)
    {
        moon[0] = (unsigned char)(48);
    } else if (moonphase_number == 0) {
        moon[0] = (unsigned char)(49);
    } else if (moonphase_number < 14) {
        moon[0] = (unsigned char)(moonphase_number+96);
    } else {
        moon[0] = (unsigned char)(moonphase_number+95);
    }
    text_layer_set_text(&moonLayer, moon);


}

void updateDayAndNightInfo()
{
  static char sunrise_text[] = "00:00";
  static char sunset_text[] = "00:00";

  PblTm pblTime;
  get_time(&pblTime);

  if(currentData != pblTime.tm_hour) 
  {
    char *time_format;

    if (clock_is_24h_style()) 
    {
      time_format = "%R";
    } 
    else 
    {
      time_format = "%I:%M";
    }

    float sunriseTime = calcSunRise(pblTime.tm_year, pblTime.tm_mon+1, pblTime.tm_mday, LATITUDE, LONGITUDE, 91.0f);
    float sunsetTime = calcSunSet(pblTime.tm_year, pblTime.tm_mon+1, pblTime.tm_mday, LATITUDE, LONGITUDE, 91.0f);
    adjustTimezone(&sunriseTime);
    adjustTimezone(&sunsetTime);
    
    if (!pblTime.tm_isdst) 
    {
      sunriseTime+=1;
      sunsetTime+=1;
    } 
    
    pblTime.tm_min = (int)(60*(sunriseTime-((int)(sunriseTime))));
    pblTime.tm_hour = (int)sunriseTime;
    string_format_time(sunrise_text, sizeof(sunrise_text), time_format, &pblTime);
if (sunrise_text[0] == '0') {
    memmove(sunrise_text, &sunrise_text[1], sizeof(sunrise_text) - 1);
  }
	  text_layer_set_text(&riseLayer, sunrise_text);
    
    pblTime.tm_min = (int)(60*(sunsetTime-((int)(sunsetTime))));
    pblTime.tm_hour = (int)sunsetTime;
    string_format_time(sunset_text, sizeof(sunset_text), time_format, &pblTime);
	  if (sunset_text[0] == '0') {
    memmove(sunset_text, &sunset_text[1], sizeof(sunset_text) - 1);
  }
    text_layer_set_text(&setLayer, sunset_text);

    currentData = pblTime.tm_hour;
  }
}

void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {
(void)t;
(void)ctx;
static char hourText[] = "00:00";
static char amText[] = "X";
static char dayText[] = "Xxxxxxxxx";
static char monthText[] = "Xxx 00";
static char secondText[] = "00";
PblTm currentTime;
get_time(&currentTime);
char *second_format;
second_format = "%S"; 

char *am_format;

char *day_format;
	day_format = "%A";
char *month_format;
	month_format = "%b %e";
char *time_format;
if (clock_is_24h_style()) {
        time_format = "%R";
	am_format = "";
    } else {
        time_format = "%I:%M";
	am_format = "%p";
    }
string_format_time(secondText, sizeof(secondText), second_format, &currentTime);
text_layer_set_text(&secondLayer, secondText);	
	
string_format_time(hourText, sizeof(hourText), time_format, &currentTime);
if (hourText[0] == '0') {
    memmove(hourText, &hourText[1], sizeof(hourText) - 1);
  }
text_layer_set_text_alignment(&hourLayer, GTextAlignmentCenter);
text_layer_set_text(&hourLayer, hourText);
string_format_time(amText, sizeof(amText), am_format, &currentTime);
	
	if (amText[0] == 'A') {
  layer_set_frame(&amLayer.layer,GRect(134,77,10 ,20));
		layer_mark_dirty(&amLayer.layer);
		
 }
	else {
layer_set_frame(&amLayer.layer,GRect(134,93,10,20));
		layer_mark_dirty(&amLayer.layer);
	}
text_layer_set_text(&amLayer, amText);	
	
string_format_time(dayText, sizeof(dayText), day_format, &currentTime);

text_layer_set_text_alignment(&dayLayer, GTextAlignmentCenter);
text_layer_set_text(&dayLayer, dayText);

	
string_format_time(monthText, sizeof(monthText), month_format, &currentTime);
text_layer_set_text_alignment(&monthLayer, GTextAlignmentCenter);
text_layer_set_text(&monthLayer, monthText);	

#if HOUR_VIBRATION
	if (currentTime.tm_min == 0 && currentTime.tm_sec == 0
                && currentTime.tm_hour>=HOUR_VIBRATION_START
		&&currentTime.tm_hour<=HOUR_VIBRATION_END) {
           
              vibes_double_pulse();
           }
#endif
	updateDayAndNightInfo();
}



// Handle the start-up of the app
void handle_init_app(AppContextRef app_ctx) {
(void)app_ctx;
  window_init(&window, "Basic Watch");
  window_stack_push(&window, true);
  window_set_background_color(&window, GColorBlack);
resource_init_current_app(&APP_RESOURCES);
font_moon = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MOON_PHASES_SUBSET_30));
	
	text_layer_init(&hourLayer, GRect(0, 70, 144 /* width */, 50 /* height */));
  text_layer_set_text_color(&hourLayer, GColorWhite);
  text_layer_set_background_color(&hourLayer, GColorClear);
  text_layer_set_font(&hourLayer, fonts_get_system_font(FONT_KEY_GOTHAM_42_BOLD));
	
		text_layer_init(&secondLayer, GRect(118, 138, 30 /* width */, 30 /* height */));
  text_layer_set_text_color(&secondLayer, GColorWhite);
  text_layer_set_background_color(&secondLayer, GColorClear);
  text_layer_set_font(&secondLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

	text_layer_init(&amLayer, window.layer.frame);
	//text_layer_init(&amLayer, GRect(134, 85, 10 /* width */, 20 /* height */));
	layer_set_frame(&amLayer.layer,GRect(134,85,10 ,20));
  text_layer_set_text_color(&amLayer, GColorWhite);
  text_layer_set_background_color(&amLayer, GColorClear);
  text_layer_set_font(&amLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));	

		text_layer_init(&dayLayer, GRect(0, 0, 144 /* width */, 40 /* height */));
  text_layer_set_text_color(&dayLayer, GColorWhite);
  text_layer_set_background_color(&dayLayer, GColorClear);
  text_layer_set_font(&dayLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	
		text_layer_init(&monthLayer, GRect(0, 30, 144 /* width */, 40 /* height */));
  text_layer_set_text_color(&monthLayer, GColorWhite);
  text_layer_set_background_color(&monthLayer, GColorClear);
  text_layer_set_font(&monthLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	
	
		text_layer_init(&moonLayer, GRect(50, 137, 31 /* width */, 31 /* height */));
    text_layer_set_text_color(&moonLayer, GColorWhite);
    text_layer_set_background_color(&moonLayer, GColorClear);
    text_layer_set_font(&moonLayer, font_moon);
	
		text_layer_init(&riseLayer, GRect(0, 128, 40 /* width */, 20 /* height */));
    text_layer_set_text_color(&riseLayer, GColorWhite);
    text_layer_set_background_color(&riseLayer, GColorClear);
    text_layer_set_font(&riseLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(&riseLayer, GTextAlignmentLeft);

   		 text_layer_init(&setLayer, GRect(0, 148, 40 /* width */, 20 /* height */));
    text_layer_set_text_color(&setLayer, GColorWhite);
    text_layer_set_background_color(&setLayer, GColorClear);
    text_layer_set_font(&setLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(&setLayer, GTextAlignmentLeft);
updateDayAndNightInfo();
handle_second_tick(app_ctx, NULL);
handle_day(app_ctx, NULL);
layer_add_child(&window.layer, &hourLayer.layer);
layer_add_child(&window.layer, &secondLayer.layer);
layer_add_child(&window.layer, &amLayer.layer);
layer_add_child(&window.layer, &dayLayer.layer);
layer_add_child(&window.layer, &monthLayer.layer);
layer_add_child(&window.layer, &moonLayer.layer);
layer_add_child(&window.layer, &riseLayer.layer);
layer_add_child(&window.layer, &setLayer.layer);

}




void pbl_main(void *params) {
  PebbleAppHandlers handlers = {

    // Handle app start
    .init_handler = &handle_init_app,

    // Handle time updates
    .tick_info = {
      .tick_handler = &handle_second_tick,
      .tick_units = SECOND_UNIT
		     }

  };
  app_event_loop(params, &handlers);
}







