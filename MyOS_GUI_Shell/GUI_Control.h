#pragma once
#include "GUI_Object.h"
#include "GUI_Window.h"

class GUI_Window;

class GUI_Control : public GUI_Object
{
public:
    GUI_Control(GUI_Window *pOwner, uint32_t controlID)
    {
        this->pOwner = pOwner;
        this->controlID = controlID;
    }

    virtual void OnHover(int relX, int relY) {};
    virtual void OnClick(int relX, int relY);

    virtual void OnDrag(int relClickX, int relClickY, int relDestX, int relDestY) {};

protected:
    GUI_Window *pOwner;
    uint32_t controlID;
};

// version of GUI_Control that will call external functions when public methods are called
// TODO: Maybe implement version with lambda functions?
// It's unclear to me how / if this will work between the shell and other apps. RPC?
class GUI_Control_Hooks : public GUI_Control
{
public:
    //void OnHover(int relX, int relY);
    void OnClick(int relX, int relY);
    void SetOnClickCallback(void(*Callback)(uint32_t param), uint32_t param);

    // OnClick callback that allows for passing a custom parameter (not sure if I'd need or use this)
    //void OnClick(int relX, int relY, uint32_t customParam);

protected:
    //void(*OnClickCallback)(uint32_t param, uint32_t customParam);

    void (*OnClickCallback)(uint32_t param);
    uint32_t onClickParam;  // parameter that will be passed to the callback. Could be a button ID for example.
};