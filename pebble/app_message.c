#include <pebble.h>
//common
static StatusBarLayer *statusBar;

static void sendMsg(uint8_t type, uint16_t data){   
      // Declare the dictionary's iterator
  DictionaryIterator *out_iter;
  
  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if(result == APP_MSG_OK) {
    // Add an item to ask for weather data
    dict_write_uint8(out_iter,0,type);
    dict_write_uint16(out_iter, 1, data);
  
    // Send this message
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
  } else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}

///loading window
static Window *loadingWindow;
static TextLayer *loadingText;

static void loading_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  statusBar = status_bar_layer_create();
  status_bar_layer_set_colors(statusBar,GColorBlack,GColorWhite);
  layer_set_frame(status_bar_layer_get_layer(statusBar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  loadingText = text_layer_create(GRect(0, 72, bounds.size.w, 200));
  text_layer_set_text(loadingText, "Loading...");
  text_layer_set_text_alignment(loadingText, GTextAlignmentCenter);
  layer_add_child(window_layer, status_bar_layer_get_layer(statusBar));
  layer_add_child(window_layer, text_layer_get_layer(loadingText));

}
static void loading_window_unload(Window *window) {
  status_bar_layer_destroy(statusBar);
  text_layer_destroy(loadingText);
}
static Window *checkOutWindow;
static void init_loading_window(void){
  loadingWindow = window_create();
  window_set_window_handlers(loadingWindow, (WindowHandlers) {
    .load = loading_window_load,
    .unload = loading_window_unload,
  });
  window_stack_remove(checkOutWindow,false);
  window_stack_push(loadingWindow, true);
}
//checkOut window
static Window *checkOutWindow;
static TextLayer *checkOutText;
static char* checkOutTexterino;
static TextLayer *retryTextLayer;
static char* retryText="RETRY";

static void check_out_up_click_handler(){
    sendMsg(7,0);
    init_loading_window();
}
static void check_out_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, check_out_up_click_handler);
}

static void checkOut_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  statusBar = status_bar_layer_create();
  status_bar_layer_set_colors(statusBar,GColorBlack,GColorWhite);
  layer_set_frame(status_bar_layer_get_layer(statusBar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  
  retryTextLayer = text_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w,20));
  text_layer_set_text(retryTextLayer, retryText);
  text_layer_set_text_alignment(retryTextLayer, GTextAlignmentRight);
  text_layer_set_font(retryTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(retryTextLayer));
  
  checkOutText = text_layer_create(GRect(0, 72, bounds.size.w, 200));
  text_layer_set_text(checkOutText, checkOutTexterino);
  text_layer_set_text_alignment(checkOutText, GTextAlignmentCenter);
  layer_add_child(window_layer, status_bar_layer_get_layer(statusBar));
  layer_add_child(window_layer, text_layer_get_layer(checkOutText));

}
static void checkOut_window_unload(Window *window) {
  status_bar_layer_destroy(statusBar);
  text_layer_destroy(checkOutText);
  text_layer_destroy(retryTextLayer);
}
static void init_checkOut_window(uint32_t *data){
  switch(*data){
    case 0:
      checkOutTexterino = "Please close the doors";
    break;
    case 1:
      checkOutTexterino = "Please put the key in the ignition";
    break;
    case 2:
      checkOutTexterino = "Please plug in the charger";
    break;
    case 3:
      checkOutTexterino = "Please park in the correct station";
    break;
    default:
      snprintf(checkOutTexterino,20,"Your Score is:%lu",*data);
       retryText="";
    break;
  }
  checkOutWindow = window_create();
  window_set_window_handlers(checkOutWindow, (WindowHandlers) {
    .load = checkOut_window_load,
    .unload = checkOut_window_unload,
  });
  window_set_click_config_provider_with_context(checkOutWindow, check_out_click_config_provider, NULL);
  window_stack_push(checkOutWindow, true);
  
}

//ok window
static Window *okWindow;
static TextLayer *okText;

static void ok_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  statusBar = status_bar_layer_create();
  status_bar_layer_set_colors(statusBar,GColorBlack,GColorWhite);
  layer_set_frame(status_bar_layer_get_layer(statusBar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  okText = text_layer_create(GRect(0, 72, bounds.size.w, 200));
  text_layer_set_text(okText, "Request successful");
  text_layer_set_text_alignment(okText, GTextAlignmentCenter);
  layer_add_child(window_layer, status_bar_layer_get_layer(statusBar));
  layer_add_child(window_layer, text_layer_get_layer(okText));

}
static void ok_window_unload(Window *window) {
  status_bar_layer_destroy(statusBar);
  text_layer_destroy(okText);
}
static void init_ok_window(void){
  okWindow = window_create();
  window_set_window_handlers(okWindow, (WindowHandlers) {
    .load = ok_window_load,
    .unload = ok_window_unload,
  });
  window_stack_push(okWindow, true);
}

////////CONTROL
///ControlWindow

static Window *controlWindow;
static TextLayer *controlHLayer;
static TextLayer *controlColonLayer;
static TextLayer *controlMLayer;
static TextLayer *controlCheckIn;
static tm *timerino;
static char controlHs[3] = "00";
static char controlMs[3] = "00";
static bool checkOut = false;

static void control_up_click_handler(){
  if(checkOut){
    sendMsg(7,0);
    init_loading_window();
  }else{
      sendMsg(6,0);
      text_layer_set_text(controlCheckIn, "CHECK OUT");
      checkOut=true;
  }
}
static void control_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, control_up_click_handler);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  timerino->tm_min--;
  mktime(timerino);
  snprintf(controlHs,3,"%02d",timerino->tm_hour);
  snprintf(controlMs,3,"%02d",timerino->tm_min);
  text_layer_set_text(controlHLayer, controlHs);
  text_layer_set_text(controlMLayer, controlMs);
}

static void control_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  statusBar = status_bar_layer_create();
  status_bar_layer_set_colors(statusBar,GColorBlack,GColorWhite);
  layer_set_frame(status_bar_layer_get_layer(statusBar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  layer_add_child(window_layer, status_bar_layer_get_layer(statusBar));

  GFont font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  controlCheckIn = text_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w,20));
  text_layer_set_text(controlCheckIn, "CHECK IN");
  text_layer_set_text_alignment(controlCheckIn, GTextAlignmentRight);
  text_layer_set_font(controlCheckIn, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(controlCheckIn));
  
  controlHLayer = text_layer_create(GRect(5, 62, 60,60));
  text_layer_set_text(controlHLayer, controlHs);
  text_layer_set_font(controlHLayer, font);
  layer_add_child(window_layer, text_layer_get_layer(controlHLayer));
  
  controlColonLayer = text_layer_create(GRect(65, 60, 10, 60));
  text_layer_set_text(controlColonLayer, ":");
  text_layer_set_font(controlColonLayer, font);
  layer_add_child(window_layer, text_layer_get_layer(controlColonLayer));
  
  controlMLayer = text_layer_create(GRect(75, 62, 60,60));
  text_layer_set_text(controlMLayer, controlMs);
  text_layer_set_font(controlMLayer, font);
  layer_add_child(window_layer, text_layer_get_layer(controlMLayer));

}
static void control_window_unload(Window *window) {
  status_bar_layer_destroy(statusBar);
  text_layer_destroy(controlHLayer);
  text_layer_destroy(controlColonLayer);
  text_layer_destroy(controlMLayer);
}
static void init_control_window(uint32_t* data){
  *data = *data - time(NULL)+2*3600;  
  timerino = gmtime((time_t*)data);
  snprintf(controlHs,3,"%02d",timerino->tm_hour);
  snprintf(controlMs,3,"%02d",timerino->tm_min);
  controlWindow = window_create();
  
  window_set_window_handlers(controlWindow, (WindowHandlers) {
    .load = control_window_load,
    .unload = control_window_unload,
  });
  window_set_click_config_provider_with_context(controlWindow, control_click_config_provider, NULL);
  window_stack_remove(loadingWindow,false);
  window_stack_push(controlWindow, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

////////BOOKING
///Select KM
static Window *selectKMWindow;
static SimpleMenuLayer *selectKMMenu;
static SimpleMenuSection selectKMMenuSection[1];
static SimpleMenuItem selectKMMenuItems[3];

static void km_menu_callback(int index, void *ctx) {
  sendMsg(1,(uint8_t)index);
  init_loading_window();
}
static void initialize_km_menu(){
  selectKMMenuItems[0] = (SimpleMenuItem){
        .title = "10-20",
        .callback = km_menu_callback
      };
  selectKMMenuItems[1] = (SimpleMenuItem){
        .title = "20-50",
        .callback = km_menu_callback
      };
  selectKMMenuItems[2] = (SimpleMenuItem){
        .title = "50+",
        .callback = km_menu_callback
      };
  selectKMMenuSection[0] = (SimpleMenuSection){
    .title = "Pick an amount of KM",
    .items = selectKMMenuItems,
    .num_items = 3
  };
}

static void select_km_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  statusBar = status_bar_layer_create();
  status_bar_layer_set_colors(statusBar,GColorBlack,GColorWhite);
  layer_set_frame(status_bar_layer_get_layer(statusBar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  initialize_km_menu();
  selectKMMenu = simple_menu_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h),selectKMWindow,selectKMMenuSection,1,NULL);
  
  layer_add_child(window_layer, status_bar_layer_get_layer(statusBar));
  layer_add_child(window_layer, simple_menu_layer_get_layer(selectKMMenu));

}
static void select_km_window_unload(Window *window) {
  status_bar_layer_destroy(statusBar);
  simple_menu_layer_destroy(selectKMMenu);
}
static void init_km_window(void){
  selectKMWindow = window_create();
  window_set_window_handlers(selectKMWindow, (WindowHandlers) {
    .load = select_km_window_load,
    .unload = select_km_window_unload,
  });
  window_stack_remove(loadingWindow,false);
  window_stack_push(selectKMWindow, true);
}
///SelectDate
static Window *selectStartDateWindow;
static SimpleMenuLayer *selectStartDateMenu;
static SimpleMenuSection selectStartDateMenuSection[1];
static SimpleMenuItem selectStartDateMenuItems[7];

static void select_start_date_menu_callback(int index, void *ctx) {
  sendMsg(2,(uint8_t)index);
  init_loading_window();
}
char startDateOptions[5][6];
static void initialize_start_date_menu(){
  for(int i=0;i<(int)(sizeof(selectStartDateMenuItems) / sizeof(SimpleMenuItem));i++){
    if(i==0){
      selectStartDateMenuItems[i] = (SimpleMenuItem){
        .title = "today",
        .callback = select_start_date_menu_callback
      };
    }else if(i==1){
      selectStartDateMenuItems[i] = (SimpleMenuItem){
        .title = "tomorrow",
        .callback = select_start_date_menu_callback
      };
    }else{
      time_t timee = time(NULL);
      tm* timeee = localtime(&timee);
      timeee->tm_mday += i;
      mktime(timeee);
      strftime(startDateOptions[i-2],6,"%d/%m",timeee);
      selectStartDateMenuItems[i] = (SimpleMenuItem){
        .title = startDateOptions[i-2],
        .callback = select_start_date_menu_callback
      };
    }
  }
  selectStartDateMenuSection[0] = (SimpleMenuSection){
    .title = "Pick Start Date",
    .items = selectStartDateMenuItems,
    .num_items = 7
  };
}

static void select_start_date_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  statusBar = status_bar_layer_create();
  status_bar_layer_set_colors(statusBar,GColorBlack,GColorWhite);
  layer_set_frame(status_bar_layer_get_layer(statusBar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  initialize_start_date_menu();
  selectStartDateMenu = simple_menu_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h),selectStartDateWindow,selectStartDateMenuSection,1,NULL);
  
  layer_add_child(window_layer, status_bar_layer_get_layer(statusBar));
  layer_add_child(window_layer, simple_menu_layer_get_layer(selectStartDateMenu));

}
static void select_start_date_window_unload(Window *window) {
  status_bar_layer_destroy(statusBar);
  simple_menu_layer_destroy(selectStartDateMenu);
}
static void init_start_date_window(void){
  selectStartDateWindow = window_create();
  window_set_window_handlers(selectStartDateWindow, (WindowHandlers) {
    .load = select_start_date_window_load,
    .unload = select_start_date_window_unload,
  });
  window_stack_remove(loadingWindow,false);
  window_stack_push(selectStartDateWindow, true);
}


///SelectStartTime
static Window *selectStartTimeWindow;
static TextLayer *selectStartTimeHourLayer;
static TextLayer *selectStartTimeColonLayer;
static TextLayer *selectStartTimeMinuteLayer;
static TextLayer *selectStartTimePlusLayer;
static TextLayer *selectStartTimeMinuteLayer;
static int tempSH = 0;
static char tempSHs[3] = "00";
static int tempSM = 0;
static char tempSMs[3] = "00";
static bool hour = true;

static void select_start_time_select_click_handler(){
  if(hour){
    hour = false;
  }else{
    sendMsg(3,(tempSH*100)+tempSM);
    init_loading_window();
  }
}
static void select_start_time_up_click_handler(){
  if(hour){
    if(tempSH==23){
      tempSH = -1;
    }
    tempSH++;
    snprintf(tempSHs,3,"%02d",tempSH);
    text_layer_set_text(selectStartTimeHourLayer, tempSHs);
  }else{
    if(tempSM==59){
      tempSM = -1;
    }
    tempSM++;
    snprintf(tempSMs,3,"%02d",tempSM);
    text_layer_set_text(selectStartTimeMinuteLayer, tempSMs);
  }
}
static void select_start_time_down_click_handler(){
    if(hour){
      if(tempSH==0){
        tempSH = 24;
      }
      tempSH--;
      snprintf(tempSHs,3,"%02d",tempSH);
      text_layer_set_text(selectStartTimeHourLayer, tempSHs);
  }else{
      if(tempSM==0){
        tempSM = 60;
      }
    tempSM--;
    snprintf(tempSMs,3,"%02d",tempSM);
    text_layer_set_text(selectStartTimeMinuteLayer, tempSMs);
  }
}
static void select_start_time_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_start_time_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, select_start_time_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, select_start_time_down_click_handler);
}

static void select_start_timeH_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  statusBar = status_bar_layer_create();
  status_bar_layer_set_colors(statusBar,GColorBlack,GColorWhite);
  layer_set_frame(status_bar_layer_get_layer(statusBar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  layer_add_child(window_layer, status_bar_layer_get_layer(statusBar));
  GFont font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);

  selectStartTimeHourLayer = text_layer_create(GRect(5, 62, 60,60));
  text_layer_set_text(selectStartTimeHourLayer, tempSHs);
  text_layer_set_font(selectStartTimeHourLayer, font);
  layer_add_child(window_layer, text_layer_get_layer(selectStartTimeHourLayer));
  
  selectStartTimeColonLayer = text_layer_create(GRect(65, 60, 10, 60));
  text_layer_set_text(selectStartTimeColonLayer, ":");
  text_layer_set_font(selectStartTimeColonLayer, font);
  layer_add_child(window_layer, text_layer_get_layer(selectStartTimeColonLayer));
  
  selectStartTimeMinuteLayer = text_layer_create(GRect(75, 62, 60,60));
  text_layer_set_text(selectStartTimeMinuteLayer, tempSMs);
  text_layer_set_font(selectStartTimeMinuteLayer, font);
  layer_add_child(window_layer, text_layer_get_layer(selectStartTimeMinuteLayer));
  
  selectStartTimePlusLayer = text_layer_create(GRect(bounds.size.w-10, STATUS_BAR_LAYER_HEIGHT, 10,20));
  text_layer_set_text(selectStartTimePlusLayer, "+");
  layer_add_child(window_layer, text_layer_get_layer(selectStartTimePlusLayer));
  
  selectStartTimeMinuteLayer = text_layer_create(GRect(bounds.size.w-10, bounds.size.h-25, 10,20));
  text_layer_set_text(selectStartTimeMinuteLayer, "-");
  layer_add_child(window_layer, text_layer_get_layer(selectStartTimeMinuteLayer));
}
static void select_start_timeH_window_unload(Window *window) {
  status_bar_layer_destroy(statusBar);
  text_layer_destroy(selectStartTimeHourLayer);
  text_layer_destroy(selectStartTimeColonLayer);
  text_layer_destroy(selectStartTimeMinuteLayer);
  text_layer_destroy(selectStartTimePlusLayer);
  text_layer_destroy(selectStartTimeMinuteLayer);
}
static void init_start_timeH_window(void){
  hour = true;
  selectStartTimeWindow = window_create();
  window_set_click_config_provider(selectStartTimeWindow, select_start_time_click_config_provider);
  window_set_window_handlers(selectStartTimeWindow, (WindowHandlers) {
    .load = select_start_timeH_window_load,
    .unload = select_start_timeH_window_unload,
  });
  window_stack_remove(loadingWindow,false);
  window_stack_push(selectStartTimeWindow, true);
}

///SelectDate
static Window *selectEndDateWindow;
static SimpleMenuLayer *selectEndDateMenu;
static SimpleMenuSection selectEndDateMenuSection[1];
static SimpleMenuItem selectEndDateMenuItems[7];

static void select_end_date_menu_callback(int index, void *ctx) {
  sendMsg(4,(uint8_t)index);
  init_loading_window();
}
char EndDateOptions[5][6];
static void initialize_end_date_menu(){
  for(int i=0;i<(int)(sizeof(selectEndDateMenuItems) / sizeof(SimpleMenuItem));i++){
    if(i==0){
      selectEndDateMenuItems[i] = (SimpleMenuItem){
        .title = "today",
        .callback = select_end_date_menu_callback
      };
    }else if(i==1){
      selectEndDateMenuItems[i] = (SimpleMenuItem){
        .title = "tomorrow",
        .callback = select_end_date_menu_callback
      };
    }else{
      time_t timee = time(NULL);
      tm* timeee = localtime(&timee);
      timeee->tm_mday += i;
      mktime(timeee);
      strftime(EndDateOptions[i-2],6,"%d/%m",timeee);
      selectEndDateMenuItems[i] = (SimpleMenuItem){
        .title = EndDateOptions[i-2],
        .callback = select_end_date_menu_callback
      };
    }
  }
  selectEndDateMenuSection[0] = (SimpleMenuSection){
    .title = "Pick End Date",
    .items = selectEndDateMenuItems,
    .num_items = 7
  };
}

static void select_end_date_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  statusBar = status_bar_layer_create();
  status_bar_layer_set_colors(statusBar,GColorBlack,GColorWhite);
  layer_set_frame(status_bar_layer_get_layer(statusBar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  initialize_end_date_menu();
  selectEndDateMenu = simple_menu_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h),selectEndDateWindow,selectEndDateMenuSection,1,NULL);
  
  layer_add_child(window_layer, status_bar_layer_get_layer(statusBar));
  layer_add_child(window_layer, simple_menu_layer_get_layer(selectEndDateMenu));

}
static void select_end_date_window_unload(Window *window) {
  status_bar_layer_destroy(statusBar);
  simple_menu_layer_destroy(selectEndDateMenu);
}
static void init_end_date_window(void){
  selectEndDateWindow = window_create();
  window_set_window_handlers(selectEndDateWindow, (WindowHandlers) {
    .load = select_end_date_window_load,
    .unload = select_end_date_window_unload,
  });
  window_stack_remove(loadingWindow,false);
  window_stack_push(selectEndDateWindow, true);
}

///SelectEndTime
static Window *selectEndTimeWindow;
static TextLayer *selectEndTimeHourLayer;
static TextLayer *selectEndTimeColonLayer;
static TextLayer *selectEndTimeMinuteLayer;
static TextLayer *selectEndTimePlusLayer;
static TextLayer *selectEndTimeMinuteLayer;
static int tempESH = 0;
static char tempESHs[3] = "00";
static int tempESM = 0;
static char tempESMs[3] = "00";
static bool Ehour = true;

static void select_end_time_select_click_handler(){
  if(Ehour){
    Ehour = false;
  }else{
    sendMsg(5,tempESH*100+tempESM);
    init_loading_window();
  }
}
static void select_end_time_up_click_handler(){
  if(Ehour){
    if(tempESH==23){
      tempESH = -1;
    }
    tempESH++;
    snprintf(tempESHs,3,"%02d",tempESH);
    text_layer_set_text(selectEndTimeHourLayer, tempESHs);
  }else{
    if(tempSM==59){
      tempSM = -1;
    }
    tempESM++;
    snprintf(tempESMs,3,"%02d",tempESM);
    text_layer_set_text(selectEndTimeMinuteLayer, tempESMs);
  }
}
static void select_end_time_down_click_handler(){
    if(Ehour){
      if(tempESH==0){
        tempESH = 24;
      }
      tempSH--;
      snprintf(tempESHs,3,"%02d",tempESH);
      text_layer_set_text(selectEndTimeHourLayer, tempESHs);
  }else{
      if(tempESM==0){
        tempESM = 60;
      }
    tempESM--;
    snprintf(tempSMs,3,"%02d",tempESM);
    text_layer_set_text(selectEndTimeMinuteLayer, tempESMs);
  }
}
static void select_end_time_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_end_time_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, select_end_time_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, select_end_time_down_click_handler);
}

static void select_end_timeH_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  statusBar = status_bar_layer_create();
  status_bar_layer_set_colors(statusBar,GColorBlack,GColorWhite);
  layer_set_frame(status_bar_layer_get_layer(statusBar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  layer_add_child(window_layer, status_bar_layer_get_layer(statusBar));
  GFont font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);

  selectEndTimeHourLayer = text_layer_create(GRect(5, 62, 60,60));
  text_layer_set_text(selectEndTimeHourLayer, tempESHs);
  text_layer_set_font(selectEndTimeHourLayer, font);
  layer_add_child(window_layer, text_layer_get_layer(selectEndTimeHourLayer));
  
  selectEndTimeColonLayer = text_layer_create(GRect(65, 60, 10, 60));
  text_layer_set_text(selectEndTimeColonLayer, ":");
  text_layer_set_font(selectEndTimeColonLayer, font);
  layer_add_child(window_layer, text_layer_get_layer(selectEndTimeColonLayer));
  
  selectEndTimeMinuteLayer = text_layer_create(GRect(75, 62, 60,60));
  text_layer_set_text(selectEndTimeMinuteLayer, tempESMs);
  text_layer_set_font(selectEndTimeMinuteLayer, font);
  layer_add_child(window_layer, text_layer_get_layer(selectEndTimeMinuteLayer));
  
  selectEndTimePlusLayer = text_layer_create(GRect(bounds.size.w-10, STATUS_BAR_LAYER_HEIGHT, 10,20));
  text_layer_set_text(selectEndTimePlusLayer, "+");
  layer_add_child(window_layer, text_layer_get_layer(selectEndTimePlusLayer));
  
  selectEndTimeMinuteLayer = text_layer_create(GRect(bounds.size.w-10, bounds.size.h-25, 10,20));
  text_layer_set_text(selectEndTimeMinuteLayer, "-");
  layer_add_child(window_layer, text_layer_get_layer(selectEndTimeMinuteLayer));
}
static void select_end_timeH_window_unload(Window *window) {
  status_bar_layer_destroy(statusBar);
  text_layer_destroy(selectEndTimeHourLayer);
  text_layer_destroy(selectEndTimeColonLayer);
  text_layer_destroy(selectEndTimeMinuteLayer);
  text_layer_destroy(selectEndTimePlusLayer);
  text_layer_destroy(selectEndTimeMinuteLayer);
}
static void init_end_timeH_window(void){
  hour = true;
  selectEndTimeWindow = window_create();
  window_set_click_config_provider(selectEndTimeWindow, select_end_time_click_config_provider);
  window_set_window_handlers(selectEndTimeWindow, (WindowHandlers) {
    .load = select_end_timeH_window_load,
    .unload = select_end_timeH_window_unload,
  });
  window_stack_remove(loadingWindow,false);
  window_stack_push(selectEndTimeWindow, true);
}





static void in_received_handler(DictionaryIterator *received, void *context) {
  Tuple* res1 = dict_find(received,0);
  Tuple* res2 = dict_find(received,1);
  if(res1 != NULL && res2 != NULL){
    
    int* type = (int*) res1->value;
    uint32_t* data = (uint32_t*)res2->value;
    switch(*type){
      case 0:
        if(*data!=0){
            init_control_window(data);
          }else{
            init_km_window();
        }
      break;
      case 1:
        if(*data==1){
            init_start_date_window();
          }else{
            init_km_window();
        }
      break;
      case 2:
        if(*data==1){
            init_start_timeH_window();
          }else{
             init_start_date_window();
        }
      break;
      case 3:
        if(*data==1){
             init_end_date_window();
          }else{
            init_start_timeH_window();
        }
      break;
      case 4:
        if(*data==1){
             init_end_timeH_window();
          }else{
            init_end_date_window();
        }
      break;
      case 5:
        if(*data==1){
            init_ok_window();
          }else{
            init_end_timeH_window();
        }
      break;
      case 6:
            init_checkOut_window(data);
      break;
      
    }
    
  }
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}



static void init(void) {
  const int inbox_size = 128;
  const int outbox_size = 128;
	app_message_open(inbox_size, outbox_size);
  

	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
  sendMsg(0,0);
  init_loading_window();
}

static void deinit(void) {
	app_message_deregister_callbacks();
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}