#include <pebble.h>

static Window *_mainWindow;
static TextLayer *_timeLayer;
static TextLayer *_dateLayer;
static TextLayer *_batteryLayer;
static TextLayer *_yearTimeLayer;
static GFont _asrFont;

static void update_battery(BatteryChargeState charge_state){
    static char batteryStatusText[] = "100% charged";
    
    if (charge_state.is_charging) {
        snprintf(batteryStatusText, sizeof(batteryStatusText), "charging");
    } else {
        snprintf(batteryStatusText, sizeof(batteryStatusText), "%d%% charged", charge_state.charge_percent);
    }
    text_layer_set_text(_batteryLayer, batteryStatusText);
}

static void update_time(){
    time_t curr_time = time(NULL); 
    struct tm *tick_time = localtime(&curr_time);
	
	static char time_buffer[] = "00:00";
    static char date_buffer[40];
    static char year_buffer[100];
	strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
    strftime(date_buffer, sizeof(date_buffer), "%d/%m/%Y %a", tick_time);
    strftime(year_buffer, sizeof(year_buffer), "week %V day %j", tick_time);
	
	text_layer_set_text(_timeLayer, time_buffer);
    text_layer_set_text(_dateLayer, date_buffer);
    text_layer_set_text(_yearTimeLayer, year_buffer);
}

static void tick_handler(struct tm *tickTime, TimeUnits unitsChanged) {
	update_time();
    update_battery(battery_state_service_peek());
}

static void draw_std_layer(GFont layerFont, TextLayer *layerToDraw, Window *window){
    text_layer_set_background_color(layerToDraw, GColorClear);
    text_layer_set_text_color(layerToDraw, GColorBlack);
    text_layer_set_text(layerToDraw, "loading...");
    text_layer_set_font(layerToDraw, layerFont);
    text_layer_set_text_alignment(layerToDraw, GTextAlignmentCenter);
    
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(layerToDraw));
}

static void main_window_load(Window *window){
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);
    _asrFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ASR_FONT_24));
	
    //time
	_timeLayer = text_layer_create(GRect(0, 10, bounds.size.w, 50));
    draw_std_layer(fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD), _timeLayer, window);
    
    //date
    _dateLayer = text_layer_create(GRect(0, 60, bounds.size.w, 40));
    draw_std_layer(fonts_get_system_font(FONT_KEY_GOTHIC_18), _dateLayer, window);
    
    //year time
    _yearTimeLayer = text_layer_create(GRect(0, 100, bounds.size.w, 40));
    draw_std_layer(fonts_get_system_font(FONT_KEY_GOTHIC_18), _yearTimeLayer, window);

    //battery
    _batteryLayer = text_layer_create(GRect(0, 140, bounds.size.w, 40));
    draw_std_layer(fonts_get_system_font(FONT_KEY_GOTHIC_18), _batteryLayer, window);
	
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(update_battery);
}

static void main_window_unload(Window *window){
    tick_timer_service_unsubscribe();
    battery_state_service_unsubscribe();
    
	text_layer_destroy(_timeLayer);
    text_layer_destroy(_batteryLayer);
}

static void init(){
	_mainWindow = window_create();
    window_set_window_handlers(_mainWindow, (WindowHandlers) {
      .load = main_window_load,
      .unload = main_window_unload
    });
	window_stack_push(_mainWindow, true);
	update_time();
}

static void deinit(){
	window_destroy(_mainWindow);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
