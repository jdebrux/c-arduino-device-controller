#include <Wire.h>
#include <Adafruit_RGBLCDShield.h> #include <utility/Adafruit_MCP23017.h>

//#define DEBUG

//states
#define INIT 0
#define FLOORONE 1
#define FLOORTWO 2
#define DEVICES 3
#define OPTIONS 4
#define LEVELS 5
#define TIME 6

//rooms
#define LIVINGROOM 0
#define KITCHEN 1
#define HALL 2
#define BED1 3
#define BED2 4
#define BED3 5

//devices
#define LIGHT 0
#define HEAT 1
#define LAMP 2 //new

//options
#define LEVEL 0
#define ON 1
#define OFF 2

//define data structure types
typedef struct {
  int8_t level, type;
  String onTime, offTime, type_name;
} device;

typedef struct {
  device devices[3];
  int8_t room_val;
  String floor_name, room_name;
} room;

//initialisation of global variables
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield ();
int8_t count = 0;
int8_t state = INIT;
int8_t mins = 0;
room house[6];
int8_t path[3];

//stack and heap pointer declaration
uint8_t *heap_ptr, *stack_ptr;

void setup() {
  // put your setup code here, to run once:
  initHouse();
  Serial.begin(9600);
  Serial.println(F("ENHANCED:LAMP,QUERY,MEMORY"));
  lcd.begin(16, 2);
  lcd.setBacklight(5);
  lcd.print("Floor 1");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    // read the incoming byte:
    String queryInput;
    queryInput = Serial.readString();
    if (queryInput == "Q\n") { //check input equals required format to call query of current state
      queryOutput();
    }
    else if (queryInput == "M\n") { //check input equals required format to call query of memory
      calculateFreeMemory();
    } //else if (queryInput == "V\n"){ //check input equals required format to call variable output
    //      #ifdef DEBUG
    //      variableCheck();
    //      #endif
    //    }
  }
  switch (state) { //switch based on the value of state, state is determined by the choice of floorMenu().
    case INIT:
      floorMenu();
      break;
    case FLOORONE:
      floorOneMenu();
      break;
    case FLOORTWO:
      floorTwoMenu();
      break;
    case DEVICES:
      deviceMenu();
      break;
    case OPTIONS:
      optionsMenu();
      break;
    case LEVELS:
      levelsMenu();
      break;
    case TIME:
      timeMenu();
      break;
  }
}

#ifdef DEBUG
void variableCheck() {
  for (int8_t i = 0; i < 3; i++) {
    Serial.print(F("Choice "));
    Serial.print(i);
    Serial.print(":");
    Serial.println(path[i]);
  }
  Serial.print(F("State:"));
  Serial.println(state);
  Serial.print(F("Count:"));
  Serial.println(count);
}
#endif

void calculateFreeMemory() {
  stack_ptr = (uint8_t *)malloc(4); //stack pointer memory allocation
  heap_ptr = stack_ptr; //set heap pointer value
  free(stack_ptr); // empty stack pointer
  stack_ptr = (uint8_t *)(SP); //set stack pointer value
  Serial.print(F("Heap Pointer:"));
  Serial.println((int)heap_ptr);
  Serial.print(F("Stack Pointer:"));
  Serial.println((int)stack_ptr);
  Serial.print(F("Available Bytes: "));
  Serial.println(stack_ptr - heap_ptr); //calculate difference between heap and stack
}

void serialOut() {
  for (int8_t i = 0; i < 6; i++) {
    for (int8_t x = 0; x < 3; x++) {
      if (house[i].devices[x].type == LAMP) { //seperate output for lamp device
        Serial.println(house[i].floor_name + F("/") + house[i].room_name + F("/") + house[i].devices[x].type_name + F("/Level:") + house[i].devices[x].level);
        Serial.println(house[i].floor_name + F("/") + house[i].room_name + F("/") + house[i].devices[x].type_name + F("/On:") + house[i].devices[x].onTime);
        Serial.println(house[i].floor_name + F("/") + house[i].room_name + F("/") + house[i].devices[x].type_name + F("/Off:") + house[i].devices[x].offTime);
      } else { //output for heat and light devices
        Serial.println(house[i].floor_name + F("/") + house[i].room_name + F("/") + house[i].devices[x].type_name + F("/Main/Level:") + house[i].devices[x].level);
        Serial.println(house[i].floor_name + F("/") + house[i].room_name + F("/") + house[i].devices[x].type_name + F("/Main/On:") + house[i].devices[x].onTime);
        Serial.println(house[i].floor_name + F("/") + house[i].room_name + F("/") + house[i].devices[x].type_name + F("/Main/Off:") + house[i].devices[x].offTime);
      }
    }
  }
}

void queryOutput() {
  if (house[path[0]].devices[path[1]].type == LAMP) { //lamp device output
    switch (path[2]) {
      case 0:
        Serial.println(house[path[0]].floor_name + F("/") + house[path[0]].room_name + F("/") + house[path[0]].devices[path[1]].type_name + F("/Level:") + house[path[0]].devices[path[1]].level);
        break;
      case 1:
        Serial.println(house[path[0]].floor_name + F("/") + house[path[0]].room_name + F("/") + house[path[0]].devices[path[1]].type_name + F("/On Time:") + house[path[0]].devices[path[1]].onTime);
        break;
      case 2:
        Serial.println(house[path[0]].floor_name + F("/") + house[path[0]].room_name + F("/") + house[path[0]].devices[path[1]].type_name + F("/Off Time:") + house[path[0]].devices[path[1]].offTime);
        break;
    }
  } else { //heat and light device output
    switch (path[2]) {
      case 0:
        Serial.println(house[path[0]].floor_name + F("/") + house[path[0]].room_name + F("/") + house[path[0]].devices[path[1]].type_name + F("/Main/Level:") + house[path[0]].devices[path[1]].level);
        break;
      case 1:
        Serial.println(house[path[0]].floor_name + F("/") + house[path[0]].room_name + F("/") + house[path[0]].devices[path[1]].type_name + F("/Main/On Time:") + house[path[0]].devices[path[1]].onTime);
        break;
      case 2:
        Serial.println(house[path[0]].floor_name + F("/") + house[path[0]].room_name + F("/") + house[path[0]].devices[path[1]].type_name + F("/Main/Off Time:") + house[path[0]].devices[path[1]].offTime);
        break;
    }
  }
}

void floorMenu() {
  String floors[] = {"Floor 1", "Floor 2", "Serial Output"};
  uint8_t buttons = lcd.readButtons();
  while (buttons & BUTTON_UP) {
    delay(200);
    if (count < 2) {
      count++;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(floors[count]); //count is 1
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_DOWN) {
    delay(200);
    if (count > 0) {
      count--;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(floors[count]); //count is 0
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_SELECT) {
    delay(200);
#ifdef DEBUG
    variableCheck();
#endif
    switch (count) {
      case 0://floor 1, count is 0
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Living Room");
        state = FLOORONE;
        break;
      case 1: //floor 2, count is 1
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Bedroom 1");
        state = FLOORTWO;
        break;
      case 2:
        lcd.clear();
        lcd.print("...");
        serialOut();
        delay(200);
        lcd.clear();
        lcd.print(floors[count]);
        break;
    }
    break;
  }
}

void floorOneMenu() {
  String floorOneRooms[] = {"Living Room", "Kitchen", "Hall"};
  String lightLbl = "Light";
  uint8_t buttons = lcd.readButtons();
  while (buttons & BUTTON_UP) {
    delay(200);
    if (count < 2) {
      count++;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(floorOneRooms[count]);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_DOWN) {
    delay(200);
    if (count > 0) {
      count--;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(floorOneRooms[count]);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_LEFT) {
    delay(200);
    lcd.clear();
    lcd.print("Floor 1");
    state = INIT;
    break;
  }
  while (buttons & BUTTON_SELECT) {
    delay(200);
#ifdef DEBUG
    variableCheck();
#endif
    switch (count) {
      case 0://count is 0
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(lightLbl);
        path[0] = LIVINGROOM;
        state = DEVICES;
        break;
      case 1: //count is 1
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(lightLbl);
        path[0] = KITCHEN;
        state = DEVICES;
        break;
      case 2:
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(lightLbl);
        path[0] = HALL;
        state = DEVICES;
        break;
    }
    break;
  }
}

void floorTwoMenu() {
  String floorTwoRooms[] = {"Bedroom 1", "Bedroom 2", "Bedroom 3"};
  String lightLbl = "Light";
  uint8_t buttons = lcd.readButtons();
  while (buttons & BUTTON_UP) {
    delay(200);
    if (count < 2) {
      count++;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(floorTwoRooms[count]);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_DOWN) {
    delay(200);
    if (count > 0) {
      count--;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(floorTwoRooms[count]);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_LEFT) {
    delay(200);
    lcd.clear();
    lcd.print("Floor 1");
    state = INIT;
    break;
  }
  while (buttons & BUTTON_SELECT) {
    delay(200);
#ifdef DEBUG
    variableCheck();
#endif
    switch (count) {
      case 0://count is 0
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(lightLbl);
        path[0] = BED1;
        state = DEVICES;
        break;
      case 1: //count is 1
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(lightLbl);
        path[0] = BED2;
        state = DEVICES;
        break;
      case 2:
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(lightLbl);
        path[0] = BED3;
        state = DEVICES;
        break;
    }
    break;
  }
}

void deviceMenu() {
  String devicesLbl[] = {"Light", "Heat", "Lamp"};
  String levelLbl = "Level";
  uint8_t buttons = lcd.readButtons();
  while (buttons & BUTTON_UP) {
    delay(200);
    if (count < 2) {
      count++;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(devicesLbl[count]);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_DOWN) {
    delay(200);
    if (count > 0) {
      count--;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(devicesLbl[count]);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_LEFT) {
    delay(200);
    lcd.clear();
    lcd.print("Floor 1");
    state = INIT;
    break;
  }
  while (buttons & BUTTON_SELECT) {
    delay(200);
#ifdef DEBUG
    variableCheck();
#endif
    switch (count) {
      case 0://count is 0
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(levelLbl);
        path[1] = LIGHT;
        state = OPTIONS;
        break;
      case 1: //count is 1
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(levelLbl);
        path[1] = HEAT;
        state = OPTIONS;
        break;
      case 2: //new
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(levelLbl);
        path[1] = LAMP;
        state = OPTIONS;
        break;
    }
    break;
  }
}

void optionsMenu() {
  String options[] = {"Level", "On Time", "Off Time"};
  uint8_t buttons = lcd.readButtons();
  while (buttons & BUTTON_UP) {
    delay(200);
    if (count < 2) {
      count++;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(options[count]);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_DOWN) {
    delay(200);
    if (count > 0) {
      count--;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(options[count]);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_LEFT) {
    delay(200);
    lcd.clear();
    lcd.print("Floor 1");
    state = INIT;
    break;
  }
  while (buttons & BUTTON_SELECT) {
    delay(200);
#ifdef DEBUG
    variableCheck();
#endif
    switch (count) {
      case 0://count is 0
        lcd.clear();
        lcd.print("Level Selected...");
        delay(500);
        count = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(count);
        path[2] = LEVEL;
        state = LEVELS;
        break;
      case 1: //count is 1
        lcd.clear();
        lcd.print("On Time Selected...");
        delay(500);
        count = mins = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(count);
        lcd.setCursor(0, 1);
        lcd.print(mins);
        path[2] = ON;
        state = TIME;
        break;
      case 2: //count is 2
        lcd.clear();
        lcd.print("Off Time Selected...");
        delay(500);
        count = mins = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(count);
        lcd.setCursor(0, 1);
        lcd.print(mins);
        path[2] = OFF;
        state = TIME;
        break;
    }
    break;
  }
}

void levelsMenu() {
  uint8_t buttons = lcd.readButtons();
  while (buttons & BUTTON_UP) {
    delay(50);
    if (count < 100) {
      count++;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(count);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_DOWN) {
    delay(200);
    if (count > 0) {
      count--;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(count);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_LEFT) {
    delay(200);
    lcd.clear();
    lcd.print("Floor 1");
    state = INIT;
    break;
  }
  while (buttons & BUTTON_SELECT) {
    delay(200);
#ifdef DEBUG
    variableCheck();
#endif
    setLevel(count, 0);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LEVEL SET...");
    delay(500);
    count = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(count);
    break;
  }
}

void timeMenu() {
  uint8_t buttons = lcd.readButtons();
  while (buttons & BUTTON_UP) {
    delay(100);
    if (count < 24) {
      if (mins < 50) {
        mins += 10;
      }
      else if (count != 23) {
        mins = 0;
        count++;
      }
      else {
        mins = 0;
        count = 0;
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(count);
      lcd.setCursor(0, 1);
      lcd.print(mins);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_DOWN) {
    delay(200);
    if (count > -1) {
      if (mins > 0) {
        mins -= 10;
      }
      else if (count != 0) {
        mins = 50;
        count--;
      }
      else {
        mins = 50;
        count = 23;
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(count);
      lcd.setCursor(0, 1);
      lcd.print(mins);
    }
    buttons = lcd.readButtons();
  }
  while (buttons & BUTTON_LEFT) {
    delay(200);
    lcd.clear();
    lcd.print("Floor 1");
    state = INIT;
    break;
  }
  while (buttons & BUTTON_SELECT) {
    delay(200);
#ifdef DEBUG
    variableCheck();
#endif
    setLevel(count, mins);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TIME SET...");
    delay(500);
    //reset display
    count = mins = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(count);
    lcd.setCursor(0, 1);
    lcd.print(mins);
    break;
  }
}

void initHouse() { //default values for the house
  house[0].room_val = LIVINGROOM;
  house[0].room_name = "Living Room";
  house[1].room_val = KITCHEN;
  house[1].room_name = "Kitchen";
  house[2].room_val = HALL;
  house[2].room_name = "Hall";
  house[3].room_val = BED1;
  house[3].room_name = "Bedroom 1";
  house[4].room_val = BED2;
  house[4].room_name = "Bedroom 2";
  house[5].room_val = BED3;
  house[5].room_name = "Bedroom 3";

  for (int8_t i = 0; i < 3; i++) {
    house[i].floor_name = "Ground";
  }

  for (int8_t i = 3; i < 6; i++) {
    house[i].floor_name = "First";
  }

  for (int8_t i = 0; i < 6; i++) {
    house[i].devices[0].type = LIGHT;
    house[i].devices[0].type_name = "Light";
    house[i].devices[1].type = HEAT;
    house[i].devices[1].type_name = "Heat";
    if (i < 3) {
      house[i].devices[2].type = LAMP;
      house[i].devices[2].type_name = "Lamp/Table";
    } else {
      house[i].devices[2].type = LAMP;
      house[i].devices[2].type_name = "Lamp/Desk";
    }
  }
}

void setLevel(int8_t level, int8_t mins) {
  char *timeVal;
  timeVal = (char*) malloc(8 * sizeof(char));
  for (int8_t i = 0; i < 6; i++) {
    if (house[i].room_val == path[0]) { //check room
      for (int8_t x = 0; x < 3; x++) {
        if (house[i].devices[x].type == path[1]) { //check device
          switch (path[2]) { //switch based on the value being set
            case LEVEL://"level"
              house[i].devices[x].level = level;//update data structure
              break;
            case ON://"onTime": 2622
              sprintf(timeVal, "%02d:%02d", level, mins); //format time
              house[i].devices[x].onTime = timeVal;//update data structure
              break;
            case OFF://"offTime": 2623
              sprintf(timeVal, "%02d:%02d", level, mins); //format time
              house[i].devices[x].offTime = timeVal; //update data structure
              break;
          }
        }
      }
    }
  }
  free(timeVal);
}
