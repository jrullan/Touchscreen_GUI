// Object layout probes: read symbol sizes from the ELF with target nm -S.
#include <Canvas_XPT2046.h>
#include <Display.h>
#include <Button.h>
#include <Terminal.h>
#include <Trend.h>
#include <MultiTrend.h>
#include <Gauge.h>
#include <Dial.h>
#include <Screen.h>
#define SIZE_PROBE(T) volatile char sizeof_##T[sizeof(T)] __attribute__((used));
SIZE_PROBE(Widget)
SIZE_PROBE(Text)
SIZE_PROBE(Display)
SIZE_PROBE(Button)
SIZE_PROBE(Indicator)
SIZE_PROBE(TrendBase)
SIZE_PROBE(Trend)
SIZE_PROBE(MultiTrend)
SIZE_PROBE(Gauge)
SIZE_PROBE(Dial)
SIZE_PROBE(Canvas_XPT2046)
SIZE_PROBE(Screen)
void setup(){
// Referencing every probe prevents linker section garbage collection.
 sizeof_Widget[0]=sizeof_Text[0]=sizeof_Display[0]=sizeof_Button[0]=0;
 sizeof_Indicator[0]=sizeof_TrendBase[0]=sizeof_Trend[0]=sizeof_MultiTrend[0]=0;
 sizeof_Gauge[0]=sizeof_Dial[0]=sizeof_Canvas_XPT2046[0]=sizeof_Screen[0]=0;
}
void loop(){}
