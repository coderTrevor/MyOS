#pragma once
class GUI_Object
{
public:
    GUI_Object();
    ~GUI_Object();

    virtual void Paint() {}
    virtual bool MouseOver(int relX, int relY) { return true; }
    
protected:


};

