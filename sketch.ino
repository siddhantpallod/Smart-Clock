#include<RTClib.h>
#include<ezButton.h>
#include <LiquidCrystal_I2C.h>

RTC_DS1307 rtc;

byte buzzer_pin = 5;
//  lcd object : setting register address 0x27
LiquidCrystal_I2C lcd(0x27, 20, 4);

//  clock variables
int year = 0;
int month = 0;
int day = 0;
int hour = 0;
int minute = 0;
int second = 0;

//  alarm variables
int alarm_hours = 0;
int alarm_minutes = 0;

//  encoder variables
byte clk = 2;
byte dt = 3;
byte sw = 4;

ezButton button(sw);

//  counting rotations
int counter = 0;
int prev_counter = -1;
int flag = 1;

String rtc_date = "";
String rtc_time = "";

void setup() {

  Serial.begin(9600);

  //  initializing LCD
  lcd.init();  //  initialize the lcd.
  lcd.clear();  //  clears the screen and moves the cursor to the top left corner.
  lcd.backlight();  //  turn on the backlight.

  //  initial messages
  lcd.clear();
  lcd_print(0, 0, "RTC found");
  delay(1000);
  lcd_print(0, 0, "Hi i am Cuckoo!");
  delay(2000);
  if (!rtc.begin()) {
    // Serial.println("RTC not initialised");
    while (true);
  }
  // Serial.println("RTC found");

  attachInterrupt(digitalPinToInterrupt(clk), encoder, FALLING);

  button.setDebounceTime(25);
}

void encoder() {
  prev_counter = counter;
  if (digitalRead(dt)  ==  HIGH)counter++;
  else counter--;
  flag = 1;
}

void mode_selector() {
  counter = constrain(counter, 0, 3);
  if (prev_counter != counter  &&  flag  ==  1) {
    if (counter  ==  0) {
      lcd.clear();
      lcd_print(0, 0, "Date and Time");
    }
    else if (counter  ==  1) {
      lcd.clear();
      lcd_print(0, 0, "Set Alarm");
    }
    else if (counter  ==  2) {
      lcd.clear();
      lcd_print(0, 0, "Stopwatch");
    }
    else if (counter  ==  3) {
      lcd.clear();
      lcd_print(0, 0, "Countdown Timer");
    }
    flag = 0;
  }
}



void loop() {



  button.loop();
  if (button.isPressed())select_mode();


  mode_selector();

  current_time();
  //  playing alarm
  if (alarm_minutes  ==  minute  &&  alarm_hours  ==  hour) {
    play_buzzer(1000, 5000);

    // resetting variables
    alarm_hours = 0;
    alarm_minutes = 0;
  }



  //  for better working of simulator
  delay(10);
}


void select_mode() {
  if (counter  ==  0)get_time();
  else if (counter  ==  1)set_alarm();
  else if (counter  ==  2)stopwatch();
  else countdown();
}

void get_time() {
  while (true) {
    //  looping button
    button.loop();
    if (button.isPressed()) {
      lcd.clear();

      //  let's run the mode_selector
      prev_counter = -1;
      flag = 1;

      //  break the loop
      break;
    }

    //  getting current time
    current_time();
    String current_date = "Date : " + String(day) + "/" + String(month) +
                          "/" + String(year);
    String current_time = "Time : " + String(hour) + ":" + String(minute) +
                          ":" + String(second);
    lcd_print(0, 0, "Date and Time");
    lcd_print(0, 1, current_date);
    lcd_print(0, 2, current_time);
  }
}



void set_alarm() {
  lcd_print(0, 0, "Enter hours : ");
  alarm_hours = set_value(0, 23);
  // Serial.print(alarm_hours);
  lcd_print(0, 0, "Enter Minutes : ");
  alarm_minutes = set_value(0, 59);
  //  printing data
  lcd.clear();
  lcd_print(0, 0, "Alarm is set for ");
  lcd_print(0, 1, String(alarm_hours));
  lcd_print(3, 1, " Hours and ");
  lcd_print(0, 2, String(alarm_minutes));
  lcd_print(3, 2, " Minutes");
  delay(5000);
}


void stopwatch() {

  int stopwatch_hour, stopwatch_minute, stopwatch_second = 0;
  int last_second = 0;

  while (true) {
    //  breaking loop if button pressed
    button.loop();
    if (button.isPressed()) {
      prev_counter = -1;
      flag = 1;

      delay(5000);
      lcd.clear();
      break;
    }

    //  tracking current time to get the 'seconds' variable
    current_time();

    //  stopwatch algo
    if (abs(second - last_second) >= 1) {
      last_second = second;
      String stopwatch_time = String(stopwatch_hour) +
                              " : " + String(stopwatch_minute) +
                              " : " + String(stopwatch_second);
      lcd.clear();
      lcd_print(0, 0, "Stopwatch");
      lcd_print(0, 1, String(stopwatch_time));
      stopwatch_second++;
    }

    //  condition check
    if (stopwatch_second > 59) {
      stopwatch_second = 0;
      stopwatch_minute++;
    }
    else if (stopwatch_minute > 59) {
      stopwatch_minute = 0;
      stopwatch_hour++;
    }
    else if (stopwatch_hour > 24) {
      stopwatch_second, stopwatch_minute, stopwatch_hour = 0;
    }
  }
}


void countdown() {
  int countdown_minutes = 0;
  int countdown_seconds = 0;
  //  setting minutes
  lcd_print(0, 0, "Enter Minutes : ");
  countdown_minutes = set_value(0, 59);
  //  setting seconds
  lcd_print(0, 0, "Enter Seconds : ");
  countdown_seconds = set_value(0, 59);

  //  start the stopwatch
  while (countdown_minutes  !=  0  ||  countdown_seconds  !=  0) {

    String t = String(countdown_minutes) + ":"
               + String(countdown_seconds);
    lcd.clear();
    lcd_print(0, 0, "Countdown Timer");
    lcd_print(0, 1, t);
    if (countdown_seconds ==  0) {
      if (countdown_minutes != 0) {
        countdown_seconds = 59;
        countdown_minutes--;
      }
    }
    countdown_seconds--;
    delay(1000);
  }
  lcd.clear();
  lcd_print(0, 0, "Countdown over");
  play_buzzer(200, 3000);

}


void lcd_print(int x, int y, String message) {
  lcd.setCursor(x, y);
  lcd.print(message);
}


void current_time() {

  //  getting current date and time
  DateTime current = rtc.now();
  year = current.year();
  month = current.month();
  day = current.day();
  hour = current.hour();
  minute = current.minute();
  second = current.second();
}


int set_value(int min_val, int max_val) {
  int value = 0;
  int last_counter = -1;
  counter = 0;

  while (true) {
    button.loop();
    if (button.isPressed())
      break;

    counter = constrain(counter, min_val, max_val);
    if (last_counter != counter) {
      lcd_print(0, 1, "  ");
      lcd_print(0, 1, String(counter));
      last_counter = counter;
    }
  }
  value = counter;
  counter = 0;
  last_counter = -1;
  return value;
}

void play_buzzer(int frequency, int duration) {
  tone(buzzer_pin, frequency);
  delay(duration);
  noTone(buzzer_pin);
}
