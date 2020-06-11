#include "GUI_Control.h"

void GUI_Control::OnClick(int relX, int relY)
{
    pOwner->ControlClicked(controlID);
}
