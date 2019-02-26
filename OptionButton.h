///////////////////////////////////////////////////////////
//  Button.h
//  Implementation of the Class Button
//  Created on:      02-Mar-2015 9:17:12 PM
///////////////////////////////////////////////////////////

#if !defined(OPTION_H)
#define OPTION_H

#include "Canvas.h"
#include "Button.h"

#if !defined(DEBOUNCE)
#define DEBOUNCE 200
#endif

#define OPTION_GAP 4

class OptionButton : public Button
{

public:
	OptionButton();
	OptionButton(unsigned int diameter, int backgroundColor, int textColor, int borderColor);
	~OptionButton();

	void init();
	void show();
	void update();
	
private:

};
#endif
