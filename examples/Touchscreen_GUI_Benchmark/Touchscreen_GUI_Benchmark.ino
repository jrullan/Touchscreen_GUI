/*
 * Touchscreen GUI — Performance Benchmark
 *
 * Automated benchmark that measures rendering performance of the
 * Dial and Slider widgets. Runs a 10-second animation cycle and
 * reports per-widget timing via Serial at 115200 baud.
 *
 * The benchmark auto-starts on boot:
 *   - Skips password entry
 *   - Goes straight to the Dial & Slider screen
 *   - Animates the slider back and forth (0-100, step 2, every 10ms)
 *   - Measures per-frame dial and slider draw times with micros()
 *   - Reports averages and totals after 10 seconds
 *
 * Hardware: ESP8266 (WEMOS D1 Mini) + ILI9341 (Lolin TFT-2.4)
 *           or ESP32 with equivalent wiring.
 *
 * Based on: Touchscreen_GUI_Demo
 * Created by: Jose Rullan
 */

// New touch system includes:
#include <Canvas_Touch.h>
#include <TouchDriver_XPT2046.h>
#include <TouchCalibration.h>
#include <Screen.h>
#include <Terminal.h>
#include <Display.h>
#include <Button.h>
#include <Slider.h>
#include <Dial.h>
#include <Trend.h>
#include <Numkey.h>
#include <IconButton.h>
#include <icons.h>
#include <neotimer.h>

// Architecture specific pins:
#ifdef ESP32
  #define TFT_CS 26
  #define TFT_DS 5
  #define TS_CS 17
#else
  #define TFT_CS 16
  #define TFT_DS 15
  #define TS_CS 0
#endif

#define BG_COLOR 0xDFE  //Background color (light blue)
TouchDriver_XPT2046 touchDriver(TS_CS);
TouchMapper mapper(&touchDriver, TouchCal::XPT2046_DEFAULT);
Canvas_Touch canvas(TFT_PORTRAIT, BG_COLOR, TFT_CS, TFT_DS, &mapper);
Display header = Display(240,40,BLUE,WHITE,WHITE,20);
Display title = Display(20);
Terminal terminal = Terminal(240,50,TERMINAL_SCROLL_UP);
Numkey numkey = Numkey();
Button btnMain = Button();
Button btnDial = Button();
Button btnButtons = Button();
Screen screen_main = Screen(&canvas,0,40,240,190);
Screen screen_dial = Screen(&canvas,0,40,240,190);
Screen screen_buttons = Screen(&canvas,0,40,240,190);
Dial dial = Dial();
Slider slider = Slider();
Trend trend = Trend(240, 90, 0, 50, 100);
IconButton btnBulb = IconButton(50,50,lightbulb_off,lightbulb_on);
IconButton btnSlider = IconButton(60,30,slider_off,slider_on);
Neotimer dialTimer = Neotimer(5000);
Neotimer incrementTimer = Neotimer(10);

bool passwordCorrect = false;
int8_t increment = 2;

// ============================================
// BENCHMARKING
// ============================================
#define BENCH_DURATION_MS 10000

unsigned long benchStart = 0;
unsigned long dialTotalUs = 0;
unsigned long sliderTotalUs = 0;
unsigned long loopTotalUs = 0;
unsigned long scanTotalUs = 0;
unsigned long dialMinUs = 0xFFFFFFFF;
unsigned long dialMaxUs = 0;
unsigned long sliderMinUs = 0xFFFFFFFF;
unsigned long sliderMaxUs = 0;
unsigned long trendTotalUs = 0;
unsigned long trendMinUs = 0xFFFFFFFF;
unsigned long trendMaxUs = 0;
uint16_t frameCount = 0;
uint16_t drawCount = 0;
bool benchDone = false;

//==================================
// EVENT HANDLING ROUTINES
//==================================

void btnIconEventHandler(Button* btn){
  if(btn == &btnBulb){
    terminal.print((btn->touched)?"Bulb pressed":"Bulb unpressed",YELLOW);
  }
  if(btn == &btnSlider){
    terminal.print((btn->touched)?"Slider On":"Slider Off",GREEN);
  }
  btn->update();
}

void btnMainEventHandler(Button* btn){
  header.setText("Main",true);
  canvas.setScreen(&screen_main);
  terminal.clear();
  terminal.print("Welcome to Touchscreen GUI demo",YELLOW);
  terminal.print("This demo shows up some features",GREEN);
  terminal.print("of the library. Enjoy!");
}

void btnDialEventHandler(Button* btn){
  if(!passwordCorrect){
    terminal.clear();
    terminal.print("Enter the numeric password:");
    terminal.print("1234",GREEN);
    canvas.add(&numkey,60,45);
  }else{
    numkeyEventHandler(&numkey);
  }
}

void btnButtonsEventHandler(Button* btn){
  header.setText("Buttons",true);
  canvas.setScreen(&screen_buttons);
  terminal.clear();
  terminal.print("Icon buttons use images to");
  terminal.print("represent the button state");
  terminal.print("Press a button to see it's state");
}

void sliderEventHandler(Slider* sld){
  dial.setCV(map(sld->currentValue,0,100,dial.scaleMin,dial.scaleMax));
  dialTimer.reset();
  dialTimer.start();
}

void numkeyEventHandler(Numkey* nk){
  if(passwordCorrect==false){
    char* password = "1234";
    bool match = true;
    if(nk->getTextSize() == Widget::getTextLength(password)){
      Serial.println(nk->getText());
      for(int i=0;i<nk->getTextSize();i++){
        if(nk->getText()[i] != password[i]){
          match = false;
          break;
        }
      }
    }else{
      match = false;
    }
    if(!match){
      terminal.print("Incorrect Password",RED);
      nk->clear();
      return;
    }
    passwordCorrect = true;
    dialTimer.start();
  }

  header.setText("Dial & Slider",true);
  canvas.setScreen(&screen_dial);
  terminal.clear();
  terminal.print("Dial reprents a value in a range");
  terminal.print("Touch anywhere on the slider bar");
  terminal.print("To change it's value");
}

//==================================
// SETUP
//==================================
void guiSetup(){
  // ===== MAIN SCREEN =====
  screen_main.bgColor = BG_COLOR;
  title.setSize(240,40);
  title.setColors(screen_main.bgColor,GRAY2,screen_main.bgColor);
  title.borderWidth = 0;
  title.setText("GUI DEMO",false);
  screen_main.add(&title,0,screen_main.h/2-FONT_Y*title.fontSize);

  // ===== SCREEN TWO =====
  screen_dial.bgColor = BG_COLOR;

  // Blue slider elements
  slider.setDebounce(0);
  slider.setSize(30,90);
  slider.setColors(BLACK,GRAY1,WHITE);
  slider.setEventHandler(&sliderEventHandler);

  dial.init();
  dial.setSize(35);
  dial.borderWidth = 5;
  dial.setColors(WHITE,GRAY2,GRAY2);
  dial.setLimits(60,70,90);
  dial.setHiLimit(75,RED);
  dial.setSP(70,BLUE);
  dial.setLowLimit(70,BLUE);
  dial.setCV(72,false);

  trend.setColors(BLACK,GREEN,WHITE);
  trend.setLimits(0,50,100);
  trend.enableAutoFit = false;

  screen_dial.add(&dial,70,35);
  screen_dial.add(&slider,170,5);
  screen_dial.add(&trend,0,100);

  // ===== SCREEN THREE - BUTTONS  =====
  screen_buttons.bgColor = BG_COLOR;

  btnBulb.setEventHandler(&btnIconEventHandler);
  btnBulb.transparentColor = BLACK;

  btnSlider.setEventHandler(&btnIconEventHandler);
  btnSlider.transparentColor = BLACK;
  btnSlider.fgColor = BLACK;
  btnSlider.labelPos = LABEL_RIGHT;
  btnSlider.setLabel("Simulate");

  screen_buttons.add(&btnBulb,95,70);
  screen_buttons.add(&btnSlider,10,10);

  // ===== CANVAS GENERAL ITEMS  =====
  btnMain.setSize(80,40);
  btnMain.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnMain.cornerRadius = 8;
  btnMain.setEventHandler(&btnMainEventHandler);
  btnMain.setText("Main");
  btnMain.setDebounce(200);

  btnDial.setSize(80,40);
  btnDial.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnDial.cornerRadius = 8;
  btnDial.setEventHandler(&btnDialEventHandler);
  btnDial.setText("Dial");
  btnDial.setDebounce(200);

  btnButtons.setSize(80,40);
  btnButtons.setColors(GRAY1,WHITE,ILI9341_LIGHTGREY);
  btnButtons.cornerRadius = 8;
  btnButtons.setEventHandler(&btnButtonsEventHandler);
  btnButtons.setText("Btns");
  btnButtons.setDebounce(200);

  numkey.setSize(120,180);
  numkey.setColors(GRAY1,BLACK,WHITE);
  numkey.init();
  numkey.setEventHandler(&numkeyEventHandler);
  numkey.setDebounce(100);

  header.setText("Main Screen",false);
  terminal.setColors(BLACK,WHITE,WHITE);

  canvas.add(&header,0,0);
  canvas.add(&btnMain,0,230);
  canvas.add(&btnDial,80,230);
  canvas.add(&btnButtons,160,230);
  canvas.add(&terminal,0,270);
  canvas.setScreen(&screen_main);
}

void setup() {
  Serial.begin(115200);
  guiSetup();

  // Auto-start: skip password, go straight to dial screen, start animation
  passwordCorrect = true;
  header.setText("Dial & Slider",true);
  canvas.setScreen(&screen_dial);
  terminal.clear();
  terminal.print("Benchmark running...");
  terminal.print("Results on Serial in 10s");
  btnSlider.touched = true;
  dialTimer.set(0);    // 0ms duration so done() returns true immediately
  dialTimer.start();
  benchStart = millis();
}

//==================================
// LOOP
//==================================
void loop() {
  unsigned long loopStart = micros();

  unsigned long s0 = micros();
  canvas.scan();
  unsigned long s1 = micros();
  scanTotalUs += (s1 - s0);

  if(dialTimer.done() && btnSlider.touched){
    if(incrementTimer.repeat()){
      int cv = constrain(slider.currentValue + increment, slider.scaleMin, slider.scaleMax);
      if(cv >= slider.scaleMax || cv <= slider.scaleMin) increment = increment * -1;
      bool onDialScreen = (canvas.currentScreen == &screen_dial);

      unsigned long t0 = micros();
      slider.setCV(cv, onDialScreen);
      unsigned long t1 = micros();
      dial.setCV(map(slider.currentValue,0,100,dial.scaleMin,dial.scaleMax), onDialScreen);
      unsigned long t2 = micros();
      trend.addValue((uint8_t)cv, onDialScreen);
      unsigned long t3 = micros();

      unsigned long sliderUs = t1 - t0;
      unsigned long dialUs = t2 - t1;
      unsigned long trendUs = t3 - t2;

      sliderTotalUs += sliderUs;
      dialTotalUs += dialUs;
      trendTotalUs += trendUs;

      if(sliderUs < sliderMinUs) sliderMinUs = sliderUs;
      if(sliderUs > sliderMaxUs) sliderMaxUs = sliderUs;
      if(dialUs < dialMinUs) dialMinUs = dialUs;
      if(dialUs > dialMaxUs) dialMaxUs = dialUs;
      if(trendUs < trendMinUs) trendMinUs = trendUs;
      if(trendUs > trendMaxUs) trendMaxUs = trendUs;

      drawCount++;
    }
  }

  frameCount++;
  loopTotalUs += (micros() - loopStart);

  // Report after benchmark duration
  if(!benchDone && millis() - benchStart >= BENCH_DURATION_MS){
    benchDone = true;
    Serial.println();
    Serial.println("========================================");
    Serial.println("  TOUCHSCREEN GUI BENCHMARK RESULTS");
    Serial.printf( "  Duration: %lu ms\n", BENCH_DURATION_MS);
    Serial.println("========================================");
    Serial.println();
    Serial.println("--- Frame Counts ---");
    Serial.printf("Total loop frames:  %u\n", frameCount);
    Serial.printf("Draw frames:        %u\n", drawCount);
    Serial.printf("Frames/sec (loop):  %u\n", (uint32_t)frameCount * 1000 / BENCH_DURATION_MS);
    Serial.printf("Frames/sec (draw):  %u\n", (uint32_t)drawCount * 1000 / BENCH_DURATION_MS);
    Serial.println();
    Serial.println("--- Timing (per draw frame) ---");
    if(drawCount > 0){
      Serial.printf("Dial   avg: %lu us  min: %lu us  max: %lu us\n",
                    dialTotalUs / drawCount, dialMinUs, dialMaxUs);
      Serial.printf("Slider avg: %lu us  min: %lu us  max: %lu us\n",
                    sliderTotalUs / drawCount, sliderMinUs, sliderMaxUs);
      Serial.printf("Trend  avg: %lu us  min: %lu us  max: %lu us\n",
                    trendTotalUs / drawCount, trendMinUs, trendMaxUs);
    }
    Serial.println();
    Serial.println("--- Timing (per loop frame) ---");
    Serial.printf("Loop avg:   %lu us\n", loopTotalUs / frameCount);
    Serial.printf("Scan avg:   %lu us\n", scanTotalUs / frameCount);
    Serial.println();
    Serial.println("--- Totals ---");
    Serial.printf("Total dial:    %lu us\n", dialTotalUs);
    Serial.printf("Total slider:  %lu us\n", sliderTotalUs);
    Serial.printf("Total trend:   %lu us\n", trendTotalUs);
    Serial.printf("Total scan:    %lu us\n", scanTotalUs);
    Serial.printf("Total loop:    %lu us\n", loopTotalUs);
    Serial.println("========================================");

    // Show on screen too
    terminal.clear();
    terminal.print("Benchmark complete!");
    terminal.print("See Serial for results");
  }
}
