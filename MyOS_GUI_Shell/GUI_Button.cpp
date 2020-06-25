#include "GUI_Button.h"

GUI_Button::GUI_Button(const char * buttonText, uint32_t controlID, GUI_Window * pOwner)
    : GUI_Control(pOwner, controlID)
{
    this->buttonText = SDL_strdup(buttonText);
    this->controlID = controlID;

    // Create the surface for the text
    SDL_Surface *pFontSurface = FNT_Render(buttonText, SDL_BLACK);

    // Create the surface for the button
    pSurface = SDL_CreateRGBSurface(0, // Flags
                                    pFontSurface->w + (GUI_BUTTON_TEXT_MARGIN * 2), // Width
                                    pFontSurface->h + (GUI_BUTTON_TEXT_MARGIN * 2), // Height
                                    32, // Depth
                                    0, 0, 0, // R, G, and B masks (default)
                                    0x000000FF);    // alpha mask

    // Fill Surface with the default button color
    uint32_t buttonColor = SDL_MapRGB(pSurface->format,
                                      SDL_DEFAULT_BUTTON_COLOR.r,
                                      SDL_DEFAULT_BUTTON_COLOR.g,
                                      SDL_DEFAULT_BUTTON_COLOR.b);
    SDL_FillRect(pSurface, NULL, buttonColor);

    // Draw font on button    
    SDL_Rect dstRect = { GUI_BUTTON_TEXT_MARGIN, GUI_BUTTON_TEXT_MARGIN, pFontSurface->w, pFontSurface->h };
    SDL_BlitSurface(pFontSurface, NULL, pSurface, &dstRect);

    SDL_FreeSurface(pFontSurface);

    // Draw border around the button
    Draw3D_Box(pSurface, 0, 0, pSurface->w, pSurface->h);

    // Center the box on the window
    dimensions.width = pSurface->w;
    dimensions.height = pSurface->h;
    dimensions.left = (pOwner->dimensions.width / 2) - (dimensions.width / 2);
    dimensions.top = (pOwner->dimensions.height / 2) - (dimensions.height / 2);
}

GUI_Button::GUI_Button(const char * buttonText, uint32_t controlID, GUI_Window * pOwner, GUI_Rect dimensions)
    : GUI_Control(pOwner, controlID)
{
    this->buttonText = SDL_strdup(buttonText);
    this->controlID = controlID;
    this->dimensions = dimensions;

    pSurface = CreateSurface();
}

void GUI_Button::PaintToSurface(SDL_Surface *pTargetSurface)
{
    SDL_BlitSurface(pSurface, NULL, pTargetSurface, dimensions.GetSDL_Rect());
}

void GUI_Button::OnClick(int relX, int relY)
{
    Draw3D_InsetBox(pSurface, 0, 0, dimensions.width, dimensions.height);

    PaintToSurface(pOwner->pSurface);

    pOwner->ControlClicked(controlID);
}

void GUI_Button::OnMouseUp(int relX, int relY)
{
    Draw3D_Box(pSurface, 0, 0, dimensions.width, dimensions.height);

    PaintToSurface(pOwner->pSurface);
}

// TODO: Error-checking
void GUI_Button::Resize(GUI_Rect newDimensions)
{
    SDL_Surface *pOldSurface = pSurface;

    dimensions = newDimensions;
    SDL_Surface *pNewSurface = CreateSurface();

    pSurface = pNewSurface;

    SDL_FreeSurface(pOldSurface);
}

// TODO: Error-checking
SDL_Surface *GUI_Button::CreateSurface()
{
    // Create the surface for the text
    SDL_Surface *pFontSurface = FNT_Render(buttonText, SDL_BLACK);

    // Create the surface for the button
    SDL_Surface *pNewSurface = SDL_CreateRGBSurface(0, // Flags
                                    dimensions.width, // Width
                                    dimensions.height, // Height
                                    32, // Depth
                                    0, 0, 0, // R, G, and B masks (default)
                                    0x000000FF);    // alpha mask

    // Fill Surface with the default button color    
    uint32_t buttonColor = SDL_MapRGB(pNewSurface->format,
                                      SDL_DEFAULT_BUTTON_COLOR.r,
                                      SDL_DEFAULT_BUTTON_COLOR.g,
                                      SDL_DEFAULT_BUTTON_COLOR.b);
    SDL_FillRect(pNewSurface, NULL, buttonColor);

    // Draw font on button    
    SDL_Rect dstRect = { GUI_BUTTON_TEXT_MARGIN, GUI_BUTTON_TEXT_MARGIN, pFontSurface->w, pFontSurface->h };
    SDL_BlitSurface(pFontSurface, NULL, pNewSurface, &dstRect);

    SDL_FreeSurface(pFontSurface);

    // Draw border around the button
    Draw3D_Box(pNewSurface, 0, 0, pNewSurface->w, pNewSurface->h);

    return pNewSurface;
}
