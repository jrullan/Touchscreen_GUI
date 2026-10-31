///////////////////////////////////////////////////////////
//  TrendBase.h
//  Shared internals for the Trend family of widgets.
//
//  Holds everything the single-series Trend and the
//  multi-series MultiTrend have in common: the chart chrome
//  (border, Y-scale, X-scale with optional time labels,
//  threshold lines), the coordinate mappings, and a generic
//  polyline plotters that draws a series' ring window using
//  the (fast, run-length) Guitft::drawLine.
//
//  Trend and MultiTrend each own their ring buffer(s) and
//  provide show()/update()/addValue(); this base is where
//  the shared rendering lives.
///////////////////////////////////////////////////////////

#ifndef TRENDBASE_H
#define TRENDBASE_H

#include "Canvas.h"
#include "Indicator.h"

#ifndef MAX_TREND_VALUES
#if defined(__STM32F1__) || defined(ESP32)
#define MAX_TREND_VALUES 32
#else
#define MAX_TREND_VALUES 16
#endif
#endif

class TrendBase : public Indicator{
	public:
		TrendBase();
		virtual ~TrendBase();

		// Methods (shared)
		void init();
		int getXVal(int index);
		int getYVal(int value);
		int getYVal(unsigned int value);
		void setWindow(int min, int max);
		void setMaxX(int maxX);      // 0 = raw sample indices, else elapsed s

		// Attributes (shared)
		uint8_t maxValues = MAX_TREND_VALUES;           // samples per series ring
		bool    enableAutoFit = false;
		bool    forceSquareWaveform = false;
		int     maxX = 0;            // seconds per whole window; 0=indices
		struct TrendWindow { int minValue; int maxValue; } trendWindow = {0, MAX_TREND_VALUES ? MAX_TREND_VALUES-1 : 0};

		// Chrome toggles (defaults reproduce the full classic look; set false
		// for a bare/dashboard variant where the host draws its own chrome).
		bool showYScale      = true;
		bool showXScale      = true;
		bool showBorder      = true;
		bool showThresholds  = true;
		bool showGridlines   = false;   // draw 25/50/75 dashed-ish % guide lines
		int  rightInset      = 0;       // px to pull the rightmost sample off the border
		                                // (so a marker puck never overlaps a border)
		int  topInset        = 0;       // px of clearance above the topmost sample
		int  bottomInset     = 0;       // px of clearance below the bottommost sample

	protected:
		// Shared chrome + plotting. values = pointer to this series' ring.
		int getYValValue(int64_t value);
		void drawBorder();
		void drawYScale();
		void drawXScale();
		void drawThresholdLines(bool colors);
		void drawGridlines();
		// Plot one series' current window. Pass bgColor to erase it.
		void drawValuesSeries(const uint8_t* values, uint16_t color);
		int  minOf(const uint8_t* values);
		int  maxOf(const uint8_t* values);

		// Effective chart geometry (0-margins when the respective scale is off).
		int chartX()  const { return showYScale ? yScaleWidth : 0; }
		int chartH()  const { return showXScale ? (h - xScaleHeight) : h; }

		byte yScaleWidth  = 4 * FONT_X + 10; // four chars + 10px tick
		byte xScaleHeight = 2 * FONT_Y + 10 + borderWidth;
};

#endif // TRENDBASE_H
