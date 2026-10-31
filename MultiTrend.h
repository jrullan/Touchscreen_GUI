///////////////////////////////////////////////////////////
//  MultiTrend.h
//  A multi-series trend widget (sibling of Trend).
//
//  Shares all chart chrome/geometry with Trend via TrendBase,
//  but plots N colored series in ONE chart area (like the AI
//  Panelboard cards that overlay CPU/GPU/NPU). Each series owns
//  its own newest-first history and inherits the shared setWindow /
//  limits / thresholds / axis rendering.
//
//  Add values per series: multi.addValue(series, val).
///////////////////////////////////////////////////////////

#ifndef MULTITREND_H
#define MULTITREND_H

#include "TrendBase.h"

#ifndef MAX_MULTITREND_SERIES
#define MAX_MULTITREND_SERIES 4
#endif

class MultiTrend : public TrendBase{
	public:
		//Constructor
		MultiTrend(unsigned int width, unsigned int height,
		           unsigned int minLimit, unsigned int setpoint, unsigned int maxLimit,
		           uint8_t numSeries = 3, uint8_t max_trend_values = MAX_TREND_VALUES,
		           uint8_t maxSeries = MAX_MULTITREND_SERIES);
		~MultiTrend();

		//Methods
		void init();
		void clear();
		void addValue(uint8_t series, uint8_t val, bool updateTrend=true);
		void setSeriesColor(uint8_t series, int color);
		// Radius of the newest-sample marker puck. Insets the chart by the same
		// amount so the puck (center + radius) never overlaps the right border.
		void setMarkerRadius(uint8_t radius);
		void drawTrend();
		void eraseTrend();
		void forceUpdate(){
			forcedUpdate = true;
			update();
			forcedUpdate = false;
		}
		int  getMin(uint8_t series);
		int  getMax(uint8_t series);
		void autoFit(bool scale);
		// Newest sample (index 0 = rightmost) of a series, for host overlays
		// (severity dots, % readouts) without reaching into the history layout.
		uint8_t latest(uint8_t series) const;
		int  latestX() { return getXVal(trendWindow.minValue); }
		int  latestY(uint8_t series) { return getYVal(latest(series)); }

		//Overriden methods
		void show();
		void update();

		//Attributes
		uint8_t  numSeries;
		uint8_t  maxSeries;
		uint8_t* values = nullptr;        // [maxSeries][maxValues], series-major
		int*     seriesColor = nullptr;   // [maxSeries]
		bool     forcedUpdate = false;
		uint8_t  markerRadius = 0;   // draw a puck at each series' newest sample (0=off)
		// Optional severity tinting of the marker (0 disables): >=markerCrit uses
		// markerCritColor, else >=markerWarn uses markerWarnColor, else series color.
		uint8_t  markerWarn = 0;
		uint8_t  markerCrit = 0;
		int      markerWarnColor = 0xFD60;   // amber
		int      markerCritColor = 0xF8CE;   // red

	private:
		void fitScale(bool scale);
		//Methods
		void drawSeries(uint8_t series, uint16_t color);
		void drawMarkers();
		const uint8_t* row(uint8_t series) const{ return values && maxValues && series < maxSeries ? values + (uint32_t)series*maxValues : nullptr; }

};

#endif // MULTITREND_H
