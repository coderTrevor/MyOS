GUI, new
Gui, Add, Edit, R20 w400 vMyEdit

letterVar = A
var = `tMyOS_Keycodes[%letterVar%_PRESSED] = SDL_SCANCODE_%letterVar%;

Transform, numberVar, Asc, %letterVar%

Loop, 26
{
	Transform, letterVar, Chr, %numberVar%
	var = %var%`n`tMyOS_Keycodes[%letterVar%_PRESSED] = SDL_SCANCODE_%letterVar%;
	numberVar += 1
}



GuiControl,, MyEdit, %var%
Gui, Show

~^s::
Reload
Exit