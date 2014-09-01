// Written by Kenny Kim, August 2014.
// For HFASt Wearable Devices Project.
// SMS App v.3


/////////////////////////////////////////// HEADERS //////////////////////////////////////////
#include <pebble.h>
#include <pebble_fonts.h>  
  
/////////////////////////////////////////// VARIABLES //////////////////////////////////////////  
int exit_counter, call_counter;

char firstpersonname[20], firstpersontext[999];

char scroll_text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam quam tellus, fermentu  m quis vulputate quis, vestibulum interdum sapien. Vestibulum lobortis pellentesque pretium. Quisque ultricies purus e  u orci convallis lacinia. Cras a urna mi. Donec convallis ante id dui dapibus nec ullamcorper erat egestas. Aenean a m  auris a sapien commodo lacinia. Sed posuere mi vel risus congue ornare. Curabitur leo nisi, euismod ut pellentesque se  d, suscipit sit amet lorem. Aliquam eget sem vitae sem aliquam ornare. In sem sapien, imperdiet eget pharetra a, lacin  ia ac justo. Suspendisse at ante nec felis facilisis eleifend.";

/////////////////////////////////////////// POINTERS //////////////////////////////////////////
Window *menu_window, *text_window, *reply_window;
  
TextLayer *first_person_name, *first_person_text;
TextLayer *busy_text, *callback_text, *lol_text;

MenuLayer *menu_layer;
ScrollLayer *scroll_layer;

GBitmap *options_bitmap, *msgicon_bitmap, *replyoptions_bitmap; // Contains data
BitmapLayer *options_layer, *msgicon_layer, *replyoptions_layer; // Presents it as a layer

/////////////////////////////////////////// Structs //////////////////////////////////////////  
enum {
  KEY_BUTTON_EVENT = 0,
  BUTTON_EVENT_UP = 1,
  BUTTON_EVENT_DOWN = 2,
  BUTTON_EVENT_SELECT = 3,

  REPLY_BUSY = 8,
  REPLY_CALLBACK = 9,
  REPLY_LOL = 10,
  KEY_BUTTON_REPLY = 11,

  BUTTON_GO_BACK = 12,
  
  BUTTON_FIRST_PERSON = 13,
  
  KEY_BUTTON_PERSON = 17,
  NAME_FIRST_PERSON = 18,
  
  MESSAGE_FIRST_PERSON = 22,
  NEXT_MESSAGE_FIRST_PERSON = 23
};

////////////////////////////////////////// VIBRATOR //////////////////////////////////////////
void confirm_vibe()
{
  //Create an array of ON-OFF-ON etc durations in milliseconds (TOTAL: 30 seconds)
  uint32_t segments[] = {100, 100, 100};
 
  //Create a VibePattern structure with the segments and length of the pattern as fields
  VibePattern pattern = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
};
  //Trigger the custom pattern to be executed
  vibes_enqueue_custom_pattern(pattern);
}


/////////////////////////////////////////// TIMER //////////////////////////////////////////
static void exit_timer(struct tm* tick_time, TimeUnits units_changed) {
        if (exit_counter-- <= 0) {
          window_stack_pop(true);
          tick_timer_service_unsubscribe();
        }
}


static void timer(struct tm* tick_time, TimeUnits units_changed) {
        if (call_counter-- <= 0) {
          exit_counter = 3;
          tick_timer_service_subscribe(SECOND_UNIT, exit_timer);
        }
}


static void reply_timer(struct tm* tick_time, TimeUnits units_changed) {
        if (call_counter-- <= 0) {
          exit_counter = 3;
          tick_timer_service_subscribe(SECOND_UNIT, exit_timer);
        }
}


////////////////////////////////////////// APPMSG STUFF //////////////////////////////////////////
void send_int(uint8_t key, uint8_t cmd)
{
  //Sends data to Android as a tuple.
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
      
  Tuplet value = TupletInteger(key, cmd);
  dict_write_tuplet(iter, &value);
      
  app_message_outbox_send();
}


void process_tuple(Tuple *t)
{
  //Process the tuple received from Android.
  //Get key
  int key = t->key;
 
  //Get integer value, if present
  int value = t->value->int32;
 
  //Get string value, if present
  char string_value[100];
  strcpy(string_value, t->value->cstring);
 
  //Decide what to do
  switch(key) {
    //////////////////////////////////////////////// FIRST PERSON ////////////////////////////////////////////////
    case NAME_FIRST_PERSON:
      //Texter name received
      snprintf(firstpersonname, sizeof("firstname lastname"), "%s", string_value);
      text_layer_set_text(first_person_name, (char*) &firstpersonname);
      break;
    
    case MESSAGE_FIRST_PERSON:
      //Text context received
      snprintf(firstpersontext, 500, "> %s", string_value);
      text_layer_set_text(first_person_text, (char*) &firstpersontext);
      break; 
    
    case NEXT_MESSAGE_FIRST_PERSON:
      //Text context received
      snprintf(firstpersontext, 500, "%s\n> %s", firstpersontext, string_value);
      text_layer_set_text(first_person_text, (char*) &firstpersontext);
      break; 
  }
}


void in_received_handler(DictionaryIterator *iter, void *context) 
{
  (void) context;
    
  vibes_short_pulse();
    
    //Get data
    Tuple *t = dict_read_first(iter);
    while(t != NULL)
    {
        process_tuple(t);
         
        //Get next
        t = dict_read_next(iter);
    }
} 


/////////////////////////////////////////// WINDOWS //////////////////////////////////////////
void text_window_load(Window *window)
{
  // Texter name 
  first_person_name = text_layer_create(GRect(17, 0, 130, 168));
  text_layer_set_background_color(first_person_name, GColorClear);
  text_layer_set_text_color(first_person_name, GColorBlack);
  text_layer_set_font(first_person_name, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_get_root_layer(text_window), text_layer_get_layer(first_person_name));
//  text_layer_set_text(first_person_name, "Carl Lam");
  
  // Text context 
  first_person_text = text_layer_create(GRect(2, 23, 135, 168));
  text_layer_set_background_color(first_person_text, GColorClear);
  text_layer_set_text_color(first_person_text, GColorBlack);
  text_layer_set_font(first_person_text, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  layer_add_child(window_get_root_layer(text_window), text_layer_get_layer(first_person_text));
 // text_layer_set_text(first_person_line_one, "> Hey");
  
  //Load bitmaps into GBitmap structures
  options_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MENU);
  msgicon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MSGICON);
  // Create BitmapLayers to show GBitmaps and add to Window. "Option" image is 21 x 146 pixels
  options_layer = bitmap_layer_create(GRect(127, 4, 21, 146));
  msgicon_layer = bitmap_layer_create(GRect(3, 10, 12, 12));
  bitmap_layer_set_bitmap(options_layer, options_bitmap);
  bitmap_layer_set_bitmap(msgicon_layer, msgicon_bitmap);
  layer_add_child(window_get_root_layer(text_window), bitmap_layer_get_layer(options_layer));
  layer_add_child(window_get_root_layer(text_window), bitmap_layer_get_layer(msgicon_layer));
  
  /*// Initialize the scroll layer
  scroll_layer = scroll_layer_create(layer_get_frame(window_get_root_layer(text_window)));
  
  // This binds the scroll layer to the window so that up and down map to scrolling
  // You may use scroll_layer_set_callbacks to add or override interactivity
  scroll_layer_set_click_config_onto_window(scroll_layer, text_window);

  text_layer_set_text(first_person_text, scroll_text);

  // Add the layers for display
  scroll_layer_add_child(scroll_layer, text_layer_get_layer(first_person_text));
  layer_add_child(window_get_root_layer(text_window), scroll_layer_get_layer(scroll_layer));*/
}


void text_window_unload(Window *window) 
{
  // Technically you should unload all these, but I'm not for development perposes.
  /*text_layer_destroy(first_person_name);
  text_layer_destroy(first_person_text);
  gbitmap_destroy(options_bitmap);     // Destroy GBitmaps
  gbitmap_destroy(msgicon_bitmap);     // Destroy GBitmaps
  bitmap_layer_destroy(options_layer); // Destroy BitmapLayers
  bitmap_layer_destroy(msgicon_layer); // Destroy BitmapLayers*/
}


void reply_window_load(Window *reply_window)
{
  // Creating texts. Pebble screen is 144x168 pixels
  // "Busy" text
  busy_text = text_layer_create(GRect(5, 2, 144, 168));
  text_layer_set_background_color(busy_text, GColorClear);
  text_layer_set_text_color(busy_text, GColorBlack);
  text_layer_set_font(busy_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_get_root_layer(reply_window), text_layer_get_layer(busy_text));
  text_layer_set_text(busy_text, "1. Busy");
  
  // "Will call back" text
  callback_text = text_layer_create(GRect(5, 56, 144, 168));
  text_layer_set_background_color(callback_text, GColorClear);
  text_layer_set_text_color(callback_text, GColorBlack);
  text_layer_set_font(callback_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_get_root_layer(reply_window), text_layer_get_layer(callback_text));
  text_layer_set_text(callback_text, "2. Will call back");
  
  // "lol" text
  lol_text = text_layer_create(GRect(5, 115, 144, 168));
  text_layer_set_background_color(lol_text, GColorClear);
  text_layer_set_text_color(lol_text, GColorBlack);
  text_layer_set_font(lol_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_get_root_layer(reply_window), text_layer_get_layer(lol_text));
  text_layer_set_text(lol_text, "3. lol");
  
  //Load bitmaps into GBitmap structures
  replyoptions_bitmap = gbitmap_create_with_resource(RESOURCE_ID_REPLYOPTIONS);
  // Create BitmapLayers to show GBitmaps and add to Window. "Option" image is 21 x 146 pixels
  replyoptions_layer = bitmap_layer_create(GRect(123, 4, 21, 146));
  bitmap_layer_set_bitmap(replyoptions_layer, replyoptions_bitmap);
  layer_add_child(window_get_root_layer(reply_window), bitmap_layer_get_layer(replyoptions_layer)); 
}


void reply_window_unload(Window *message_window)
{
  text_layer_destroy(busy_text);   // Destroy Text
  text_layer_destroy(callback_text);   // Destroy Text
  text_layer_destroy(lol_text);   // Destroy Text
  gbitmap_destroy(replyoptions_bitmap);     // Destroy GBitmaps
  bitmap_layer_destroy(replyoptions_layer); // Destroy BitmapLayers
}


/////////////////////////////////////////// BUTTON BEHAVIOURS //////////////////////////////////////////
void reply_up_click_handler(ClickRecognizerRef recognizer, void *context)
{
  text_layer_set_text(busy_text, "Sending Msg...");
  text_layer_set_text(callback_text, " ");
  text_layer_set_text(lol_text, " ");
  confirm_vibe();
  send_int(KEY_BUTTON_REPLY, REPLY_BUSY);
  call_counter = 2;
  tick_timer_service_subscribe(SECOND_UNIT, reply_timer);
}
 

void reply_down_click_handler(ClickRecognizerRef recognizer, void *context)
{
  text_layer_set_text(busy_text, "Sending Msg...");
  text_layer_set_text(callback_text, " ");
  text_layer_set_text(lol_text, " ");
  confirm_vibe();
  send_int(KEY_BUTTON_REPLY, REPLY_LOL);
  call_counter = 2;
  tick_timer_service_subscribe(SECOND_UNIT, reply_timer);
}
 

void reply_select_click_handler(ClickRecognizerRef recognizer, void *context)
{
  text_layer_set_text(busy_text, "Sending Msg...");
  text_layer_set_text(callback_text, " ");
  text_layer_set_text(lol_text, " ");
  confirm_vibe();
  send_int(KEY_BUTTON_REPLY, REPLY_CALLBACK);
  call_counter = 2;
  tick_timer_service_subscribe(SECOND_UNIT, reply_timer);

}


void reply_back_click_handler(ClickRecognizerRef recognizer, void *context)
{
  send_int(KEY_BUTTON_REPLY, BUTTON_GO_BACK);
  window_stack_pop(true);
}  


void reply_click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, reply_up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, reply_down_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, reply_select_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, reply_back_click_handler);
}


void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
  confirm_vibe();
  send_int(KEY_BUTTON_EVENT, BUTTON_EVENT_UP);
}
 

void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
  confirm_vibe();
  send_int(KEY_BUTTON_EVENT, BUTTON_EVENT_DOWN);
}
 

void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
  send_int(KEY_BUTTON_EVENT, BUTTON_EVENT_SELECT);
  confirm_vibe();
   
  window_set_click_config_provider(reply_window, reply_click_config_provider);
  window_stack_push(reply_window, true);
}


void back_click_handler(ClickRecognizerRef recognizer, void *context)
{
  send_int(KEY_BUTTON_EVENT, BUTTON_GO_BACK);
  window_stack_pop_all(true); // Change this to window_stack_pop(true) if you want to see the menu! 
}  


void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}


/////////////////////////////////////////// MENU LAYERS //////////////////////////////////////////
void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
    //Which row is it?
  switch(cell_index->row)
  {
    case 0:
    menu_cell_title_draw(ctx, cell_layer, (char*) &firstpersonname);
    break;
    case 1:

    break;
    case 2:
    //menu_cell_title_draw(ctx, cell_layer, "Wayne");
    break;
    case 3:
    //menu_cell_title_draw(ctx, cell_layer, "Areeba");
    break;
  }
}
 

uint16_t num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
{
  //How many rows?
  return 4;
}
 

void select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
  //Get which row. What does each row do?
  int which = cell_index->row;
 
  if (which == 0)
  {
    send_int(KEY_BUTTON_PERSON, BUTTON_FIRST_PERSON);
    window_set_click_config_provider(text_window, click_config_provider);
    window_stack_push(text_window, true);
  }
  if (which == 1)
  {
  // send_int(KEY_BUTTON_PERSON, BUTTON_SECOND_PERSON);
  // window_set_click_config_provider(text_window_two, click_config_provider);
  // window_stack_push(text_window_two, true);
  }
  if (which == 2)
  {
   // send_int(KEY_BUTTON_PERSON, BUTTON_THIRD_PERSON);
   // window_set_click_config_provider(text_window_three, click_config_provider);
   // window_stack_push(text_window_three, true);
  }
  if (which == 3)
  {
   // send_int(KEY_BUTTON_PERSON, BUTTON_FOURTH_PERSON);
   // window_set_click_config_provider(text_window_four, click_config_provider);
   // window_stack_push(text_window_four, true);
  }
}


/////////////////////////////////////////// MENU WINDOW //////////////////////////////////////////
void menu_window_load(Window *window)
{
  //Create it - 12 is approx height of the top bar
  menu_layer = menu_layer_create(GRect(0, 0, 131, 168 - 16));
 
  //Let it receive clicks
  menu_layer_set_click_config_onto_window(menu_layer, window);
 
  //Give it its callbacks
  MenuLayerCallbacks callbacks = {
      .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
      .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
      .select_click = (MenuLayerSelectCallback) select_click_callback
    };
  menu_layer_set_callbacks(menu_layer, NULL, callbacks);
 
  //Add to Window
  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer)); 
  
  //Load bitmaps into GBitmap structures
  options_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MENU);
  // Create BitmapLayers to show GBitmaps and add to Window. "Option" image is 21 x 146 pixels
  options_layer = bitmap_layer_create(GRect(127, 4, 21, 146));
  bitmap_layer_set_bitmap(options_layer, options_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(options_layer));
}
 

void menu_window_unload(Window *window)
{
  menu_layer_destroy(menu_layer);
}


/////////////////////////////////////////// INIT //////////////////////////////////////////
void init()
{ 
  menu_window = window_create();
  window_set_window_handlers(menu_window, (WindowHandlers) {
        .load = menu_window_load,
        .unload = menu_window_unload
    });
  
  text_window = window_create();  
  window_set_window_handlers(text_window, (WindowHandlers) {
    .load = text_window_load,
    .unload = text_window_unload
    });
  
  reply_window = window_create();
  window_set_window_handlers(reply_window, (WindowHandlers) {
    .load = reply_window_load,
    .unload = reply_window_unload
  });
  
  //Register AppMessage events
  app_message_register_inbox_received(in_received_handler);           
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());    //Large input and output buffer sizes
  
  window_stack_push(menu_window, true);
  window_set_click_config_provider(text_window, click_config_provider);
  window_stack_push(text_window, true);
}
 

/////////////////////////////////////////// DEINIT //////////////////////////////////////////
void deinit()
{
  window_destroy(menu_window);
  window_destroy(text_window);
  window_destroy(reply_window);
}
 

/////////////////////////////////////////// MAIN //////////////////////////////////////////
int main(void)
{
    init();
    app_event_loop();
    deinit();
}