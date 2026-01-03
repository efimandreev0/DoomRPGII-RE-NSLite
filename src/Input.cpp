#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Canvas.h"
#include "Input.h"
#include "SDLGL.h"
#include "GLES.h"
#include "TinyGL.h"
#include "MenuSystem.h"
#include "Player.h"
#include "Game.h"

#include <algorithm>
#include <cmath>
#include <vector>
#ifdef __aarch64__
#include <switch.h>
#elif __3DS__
#include <3ds.h>
#endif
char buttonNames[][NUM_GAMEPAD_INPUTS] = {
    "Gamepad A",
    "Gamepad B",
    "Gamepad X",
    "Gamepad Y",
    "Back",
    "Guide",
    "Start",
    "Left Stick",
    "Right Stick",
    "Left Bumper",
    "Right Bumper",
    "D-Pad Up",
    "D-Pad Down",
    "D-Pad Left",
    "D-Pad Right",
    "Axis Left X",
    "Axis Left Y",
    "Axis Right X",
    "Axis Right Y",
    "Left Trigger",
    "Right Trigger",
    "L-Stick Up",
    "L-Stick Down",
    "L-Stick Left",
    "L-Stick Right",
    "R-Stick Up",
    "R-Stick Down",
    "R-Stick Left",
    "R-Stick Right"
};
#ifdef __aarch64__
static PadState pad;
#endif
#if defined(__aarch64__) || defined(__3DS__)
static u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0;
#endif
static std::vector<uint16_t>        gKeyboardKeysPressed;
static std::vector<uint16_t>        gKeyboardKeysJustPressed;
static std::vector<uint16_t>        gKeyboardKeysJustReleased;
static float                        gGamepadInputs[NUM_GAMEPAD_INPUTS];
static std::vector<GamepadInput>    gGamepadInputsPressed;
static std::vector<GamepadInput>    gGamepadInputsJustPressed;
static std::vector<GamepadInput>    gGamepadInputsJustReleased;
static std::vector<JoystickAxis>    gJoystickAxes;
static std::vector<uint32_t>        gJoystickAxesPressed;
static std::vector<uint32_t>        gJoystickAxesJustPressed;
static std::vector<uint32_t>        gJoystickAxesJustReleased;
static std::vector<uint32_t>        gJoystickButtonsPressed;
static std::vector<uint32_t>        gJoystickButtonsJustPressed;
static std::vector<uint32_t>        gJoystickButtonsJustReleased;

static SDL_GameController* gpGameController;
static SDL_Joystick* gpJoystick;         // Note: if there is a game controller then this joystick will be managed by that and not closed manually by this module!
static SDL_JoystickID               gJoystickId;
static SDL_Haptic* gJoyHaptic;

int     gDeadZone;
int     gVibrationIntensity;
float   gBegMouseX;
float   gBegMouseY;
float   gCurMouseX;
float   gCurMouseY;

keyMapping_t keyMapping[KEY_MAPPIN_MAX];
keyMapping_t keyMappingTemp[KEY_MAPPIN_MAX];
keyMapping_t keyMappingDefault[KEY_MAPPIN_MAX] = {
    {AVK_UP | AVK_MENU_UP,				{SDL_SCANCODE_UP,SDL_SCANCODE_W,-1,-1,-1,-1,-1,-1,-1,-1}},	// Move forward
    {AVK_DOWN | AVK_MENU_DOWN,			{SDL_SCANCODE_DOWN,SDL_SCANCODE_S,-1,-1,-1,-1,-1,-1,-1,-1}},	// Move backward
    {AVK_LEFT | AVK_MENU_PAGE_UP,		{SDL_SCANCODE_LEFT,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	// Turn left/page up
    {AVK_RIGHT | AVK_MENU_PAGE_DOWN,	{SDL_SCANCODE_RIGHT,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	// Turn right/page down
    {AVK_MOVELEFT,						{SDL_SCANCODE_A,-1,-1,-1,-1,-1,-1,-1,-1,-1}},		// Move left
    {AVK_MOVERIGHT,						{SDL_SCANCODE_D,-1,-1,-1,-1,-1,-1,-1,-1,-1}},		// Move right
    {AVK_NEXTWEAPON,					{SDL_SCANCODE_Z,-1,-1,-1,-1,-1,-1,-1,-1,-1}},		// Next weapon
    {AVK_PREVWEAPON,					{SDL_SCANCODE_X,-1,-1,-1,-1,-1,-1,-1,-1,-1}},		// Prev weapon
    {AVK_SELECT | AVK_MENU_SELECT,		{SDL_SCANCODE_RETURN,-1,-1,-1,-1,-1,-1,-1,-1,-1}},  // Attack/Talk/Use
    {AVK_PASSTURN,						{SDL_SCANCODE_C,-1,-1,-1,-1,-1,-1,-1,-1,-1}},		// Pass Turn
    {AVK_AUTOMAP,						{SDL_SCANCODE_TAB,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	    // Automap
    {AVK_MENUOPEN | AVK_MENU_OPEN,		{SDL_SCANCODE_ESCAPE,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	// Open menu/back
    {AVK_ITEMS_INFO,                    {SDL_SCANCODE_I,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	    // Menu items and info
    {AVK_DRINKS,		                {SDL_SCANCODE_O,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	    // Menu Dirnks
    {AVK_PDA,		                    {SDL_SCANCODE_P,-1,-1,-1,-1,-1,-1,-1,-1,-1}},	    // Menu PDA
    {AVK_BOTDISCARD,		            {SDL_SCANCODE_B,-1,-1,-1,-1,-1,-1,-1,-1,-1}}	    // Bot discart
};

//--------------------

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL button to a controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput sdlButtonToInput(const uint8_t button) noexcept {
    switch (button) {
    case SDL_CONTROLLER_BUTTON_A:               return GamepadInput::BTN_A;
    case SDL_CONTROLLER_BUTTON_B:               return GamepadInput::BTN_B;
    case SDL_CONTROLLER_BUTTON_X:               return GamepadInput::BTN_X;
    case SDL_CONTROLLER_BUTTON_Y:               return GamepadInput::BTN_Y;
    case SDL_CONTROLLER_BUTTON_BACK:            return GamepadInput::BTN_BACK;
    case SDL_CONTROLLER_BUTTON_GUIDE:           return GamepadInput::BTN_GUIDE;
    case SDL_CONTROLLER_BUTTON_START:           return GamepadInput::BTN_START;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:       return GamepadInput::BTN_LEFT_STICK;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:      return GamepadInput::BTN_RIGHT_STICK;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:    return GamepadInput::BTN_LEFT_SHOULDER;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:   return GamepadInput::BTN_RIGHT_SHOULDER;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:         return GamepadInput::BTN_DPAD_UP;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:       return GamepadInput::BTN_DPAD_DOWN;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:       return GamepadInput::BTN_DPAD_LEFT;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:      return GamepadInput::BTN_DPAD_RIGHT;

    default:
        return GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL button to a joy controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t sdlJoyButtonToInput(const uint32_t button) noexcept {
    switch (button) {
    case 0: return (int)GamepadInput::BTN_A;
    case 1: return (int)GamepadInput::BTN_B;
    case 2: return (int)GamepadInput::BTN_X;
    case 3: return (int)GamepadInput::BTN_Y;
    case 4: return (int)GamepadInput::BTN_LEFT_SHOULDER;
    case 5: return (int)GamepadInput::BTN_RIGHT_SHOULDER;
    case 6: return (int)GamepadInput::BTN_BACK;
    case 7: return (int)GamepadInput::BTN_START;
    case 8: return (int)GamepadInput::BTN_LEFT_STICK;
    case 9: return (int)GamepadInput::BTN_RIGHT_STICK;
    case 10: return (int)GamepadInput::BTN_GUIDE;
    default:
        return (int)GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis to a controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput sdlAxisToInput(const uint8_t axis) noexcept {
    switch (axis) {
    case SDL_CONTROLLER_AXIS_LEFTX:             return GamepadInput::AXIS_LEFT_X;
    case SDL_CONTROLLER_AXIS_LEFTY:             return GamepadInput::AXIS_LEFT_Y;
    case SDL_CONTROLLER_AXIS_RIGHTX:            return GamepadInput::AXIS_RIGHT_X;
    case SDL_CONTROLLER_AXIS_RIGHTY:            return GamepadInput::AXIS_RIGHT_Y;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:       return GamepadInput::AXIS_TRIG_LEFT;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:      return GamepadInput::AXIS_TRIG_RIGHT;

    default:
        return GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis to a controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput sdlAxisToInput2(const GamepadInput axis, const float value) noexcept {
    //float deadZL = (float)deadZoneLeft / 100.f;
    //float deadZR = (float)deadZoneRight / 100.f;

    switch (axis) {
    case GamepadInput::AXIS_LEFT_X: {
        // X axis motion
        float xVal = value;
        // Below of dead zone
        if (xVal < -0) {
            return GamepadInput::BTN_LAXIS_LEFT;
        }
        // Above of dead zone
        else if (xVal > 0) {
            return GamepadInput::BTN_LAXIS_RIGHT;
        }
    }
    case GamepadInput::AXIS_LEFT_Y: {
        // Y axis motion
        float yVal = value;
        // Below of dead zone
        if (yVal < -0) {
            return GamepadInput::BTN_LAXIS_UP;
        }
        // Above of dead zone
        else if (yVal > 0) {
            return GamepadInput::BTN_LAXIS_DOWN;
        }
    }
    case GamepadInput::AXIS_RIGHT_X: {
        // X axis motion
        float xVal = value;
        // Left of dead zone
        if (xVal < -0) {
            return GamepadInput::BTN_RAXIS_LEFT;
        }
        // Right of dead zone
        else if (xVal > 0) {
            return GamepadInput::BTN_RAXIS_RIGHT;
        }
    }
    case GamepadInput::AXIS_RIGHT_Y: {
        // Y axis motion
        float yVal = value;
        // Below of dead zone
        if (yVal < -0) {
            return GamepadInput::BTN_RAXIS_UP;
        }
        // Above of dead zone
        else if (yVal > 0) {
            return GamepadInput::BTN_RAXIS_DOWN;
        }
    }
    case GamepadInput::AXIS_TRIG_LEFT:       return GamepadInput::AXIS_TRIG_LEFT;
    case GamepadInput::AXIS_TRIG_RIGHT:      return GamepadInput::AXIS_TRIG_RIGHT;
    default:
        return GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis to a joy controller input enum
//------------------------------------------------------------------------------------------------------------------------------------------
GamepadInput sdlJoyAxisToInput(const uint8_t axis, const float value) noexcept {
    //float deadZL = (float)deadZoneLeft / 100.f;
    //float deadZR = (float)deadZoneRight / 100.f;
    int numAxes = SDL_JoystickNumAxes(gpJoystick);

    switch (axis) {
    case 0: {
        // X axis motion
        float xVal = value;
        // Below of dead zone
        if (xVal < -0) {
            return (numAxes <= 2) ? GamepadInput::BTN_DPAD_LEFT : GamepadInput::BTN_LAXIS_LEFT;
        }
        // Above of dead zone
        else if (xVal > 0) {
            return (numAxes <= 2) ? GamepadInput::BTN_DPAD_RIGHT : GamepadInput::BTN_LAXIS_RIGHT;
        }
    }
    case 1: {
        // Y axis motion
        float yVal = value;
        // Below of dead zone
        if (yVal < -0) {
            return (numAxes <= 2) ? GamepadInput::BTN_DPAD_UP : GamepadInput::BTN_LAXIS_UP;
        }
        // Above of dead zone
        else if (yVal > 0) {
            return (numAxes <= 2) ? GamepadInput::BTN_DPAD_DOWN : GamepadInput::BTN_LAXIS_DOWN;
        }
    }
    case 3: {
        // X axis motion
        float xVal = value;
        // Left of dead zone
        if (xVal < -0) {
            return GamepadInput::BTN_RAXIS_LEFT;
        }
        // Right of dead zone
        else if (xVal > 0) {
            return GamepadInput::BTN_RAXIS_RIGHT;
        }
    }
    case 2: {
        // Y axis motion
        float yVal = value;
        // Below of dead zone
        if (yVal < -0) {
            return GamepadInput::BTN_RAXIS_UP;
        }
        // Above of dead zone
        else if (yVal > 0) {
            return GamepadInput::BTN_RAXIS_DOWN;
        }
    }
    case 4: return GamepadInput::AXIS_TRIG_LEFT;
    case 5: return GamepadInput::AXIS_TRIG_RIGHT;
    default:
        return GamepadInput::INVALID;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Vector utility functions
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static inline bool vectorContainsValue(const std::vector<T>& vec, const T val) noexcept {
    const auto endIter = vec.end();
    const auto iter = std::find(vec.begin(), endIter, val);
    return (iter != endIter);
}

template <class T>
static inline void removeValueFromVector(const T val, std::vector<T>& vec) noexcept {
    auto endIter = vec.end();
    auto iter = std::find(vec.begin(), endIter, val);

    while (iter != endIter) {
        iter = vec.erase(iter);
        endIter = vec.end();
        iter = std::find(iter, endIter, val);
    }
}

template <class T>
static inline void emptyAndShrinkVector(std::vector<T>& vec) noexcept {
    vec.clear();
    vec.shrink_to_fit();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Close the currently open game controller or generic joystick (if any).
// Also clears up any related inputs.
//------------------------------------------------------------------------------------------------------------------------------------------
static void closeCurrentGameController() noexcept {
    std::memset(gGamepadInputs, 0, sizeof(gGamepadInputs));
    gGamepadInputsPressed.clear();
    gGamepadInputsJustPressed.clear();
    gGamepadInputsJustReleased.clear();

    gJoystickAxes.clear();
    gJoystickAxesPressed.clear();
    gJoystickAxesJustPressed.clear();
    gJoystickAxesJustReleased.clear();

    gJoystickButtonsPressed.clear();
    gJoystickButtonsJustPressed.clear();
    gJoystickButtonsJustReleased.clear();

    // Close the current game controller, if there is any.
    // Note that closing a game controller closes the associated joystick automatically also.
    if (gpGameController) {
        SDL_GameControllerClose(gpGameController);
        gpGameController = nullptr;
        gpJoystick = nullptr;       // Managed by the game controller object, already closed!
    }

    // Close the current generic joystick, if that's all we have and not the 'game controller' interface
    if (gpJoystick) {
        SDL_JoystickClose(gpJoystick);
        gpJoystick = nullptr;

        if (gJoyHaptic) {
            SDL_HapticClose(gJoyHaptic);
            gpJoystick = nullptr;
        }
    }

    gJoystickId = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Rescans for SDL game controllers and generic joysticks to use: just uses the first available controller or joystick.
// This may choose wrong in a multi-gamepad/joystick situation but the user can always disconnect one to clarify which one is wanted.
// Most computer users would probably only want one gamepad or joystick connected at a time anyway?
//------------------------------------------------------------------------------------------------------------------------------------------
static void rescanGameControllers() noexcept {
    // If we already have a gamepad or generic joystick then just re-check that it is still connected.
    // Note that we can check if a gamepad is connected by checking if the associated joystick is connected.
    if (gpJoystick) {
        if (!SDL_JoystickGetAttached(gpJoystick)) {
            closeCurrentGameController();
        }
    }

    // See if there are any joysticks connected.
    // Note: a return of < 0 means an error, which we will ignore:
    const int numJoysticks = SDL_NumJoysticks();

    for (int joyIdx = 0; joyIdx < numJoysticks; ++joyIdx) {
        // If we find a valid game controller or generic joystick then try to open it.
        // If we succeed then our work is done!
        if (SDL_IsGameController(joyIdx)) {
            printf("IsGameController\n");
            // This is a game controller - try opening that way
            gpGameController = SDL_GameControllerOpen(joyIdx);

            if (gpGameController) {
                gpJoystick = SDL_GameControllerGetJoystick(gpGameController);
                gJoystickId = SDL_JoystickInstanceID(gpJoystick);

                // Check if joystick supports Rumble
                if (!SDL_GameControllerHasRumble(gpGameController)) {
                    printf("Warning: Game controller does not have rumble! SDL Error: %s\n", SDL_GetError());
                }
                break;
            }
        }

        // Fallback to opening the controller as a generic joystick if it's not supported through the game controller interface
        gpJoystick = SDL_JoystickOpen(joyIdx);

        if (gpJoystick) {
            gJoystickId = SDL_JoystickInstanceID(gpJoystick);

            // Check if joystick supports haptic
            if (!SDL_JoystickIsHaptic(gpJoystick)) {
                printf("Warning: Controller does not support haptics! SDL Error: %s\n", SDL_GetError());
            }
            else
            {
                // Get joystick haptic device
                gJoyHaptic = SDL_HapticOpenFromJoystick(gpJoystick);
                if (gJoyHaptic == NULL) {
                    printf("Warning: Unable to get joystick haptics! SDL Error: %s\n", SDL_GetError());
                }
                else
                {
                    // Initialize rumble
                    if (SDL_HapticRumbleInit(gJoyHaptic) < 0) {
                        printf("Warning: Unable to initialize haptic rumble! SDL Error: %s\n", SDL_GetError());
                    }
                }
            }
            break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an SDL axis value to a -1 to + 1 range float.
//------------------------------------------------------------------------------------------------------------------------------------------
static float sdlAxisValueToFloat(const int16_t axis) noexcept {
    if (axis >= 0) {
        return (float)axis / 32767.0f;
    }
    else {
        return (float)axis / 32768.0f;
    }
}

float getJoystickAxisValue(const uint32_t axis) noexcept {
    for (const JoystickAxis& axisAndValue : gJoystickAxes) {
        if (axisAndValue.axis == axis)
            return axisAndValue.value;
    }

    return 0.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update the value of a joystick axis in the vector of values: removes the value if it has reached '0'
//------------------------------------------------------------------------------------------------------------------------------------------
static void updateJoystickAxisValue(const uint32_t axis, const float value) noexcept {
    // Search for the existing value of this axis: will need to remove or update it if found
    auto iter = std::find_if(gJoystickAxes.begin(), gJoystickAxes.end(), [=](const JoystickAxis& axisValue) noexcept { return (axisValue.axis == axis); });

    if (value == 0.0f) {
        if (iter != gJoystickAxes.end()) {
            gJoystickAxes.erase(iter);
        }
    }
    else {
        if (iter != gJoystickAxes.end()) {
            iter->value = value;
        }
        else {
            gJoystickAxes.emplace_back(JoystickAxis{ axis, value });
        }
    }
}

bool isKeyboardKeyPressed(const uint16_t key) noexcept {
    return vectorContainsValue(gKeyboardKeysPressed, key);
}

bool isKeyboardKeyJustPressed(const uint16_t key) noexcept {
    return vectorContainsValue(gKeyboardKeysJustPressed, key);
}

bool isKeyboardKeyReleased(const uint16_t key) noexcept {
    return (!isKeyboardKeyPressed(key));
}

bool isKeyboardKeyJustReleased(const uint16_t key) noexcept {
    return vectorContainsValue(gKeyboardKeysJustReleased, key);
}

bool isGamepadInputPressed(const GamepadInput input) noexcept {
    return vectorContainsValue(gGamepadInputsPressed, input);
}

bool isGamepadInputJustPressed(const GamepadInput input) noexcept {
    return vectorContainsValue(gGamepadInputsJustPressed, input);
}

bool isGamepadInputJustReleased(const GamepadInput input) noexcept {
    return vectorContainsValue(gGamepadInputsJustReleased, input);
}

bool isJoystickAxisPressed(const uint32_t axis) noexcept {
    return vectorContainsValue(gJoystickAxesPressed, axis);
}

bool isJoystickAxisJustPressed(const uint32_t axis) noexcept {
    return vectorContainsValue(gJoystickAxesJustPressed, axis);
}

bool isJoystickAxisJustReleased(const uint32_t axis) noexcept {
    return vectorContainsValue(gJoystickAxesJustReleased, axis);
}

bool isJoystickButtonPressed(const uint32_t button) noexcept {
    return vectorContainsValue(gJoystickButtonsPressed, button);
}

bool isJoystickButtonJustPressed(const uint32_t button) noexcept {
    return vectorContainsValue(gJoystickButtonsJustPressed, button);
}

bool isJoystickButtonJustReleased(const uint32_t button) noexcept {
    return vectorContainsValue(gJoystickButtonsJustReleased, button);
}

bool isJoystickAxisButtonPressed(uint32_t button) noexcept { // [GEC]
    for (uint32_t i = 0; i <= 5; i++) {
        if (isJoystickAxisPressed(i)) {
            if (sdlJoyAxisToInput(i, getJoystickAxisValue(i)) == (GamepadInput)button) {
                removeValueFromVector(i, gJoystickAxesPressed);
                return true;
            }
        }
    }
    return false;
}

bool isJoystickAxisButtonJustPressed(uint32_t button) noexcept { // [GEC]
    for (uint32_t i = 0; i <= 5; i++) {
        if (isJoystickAxisJustPressed(i)) {
            if (sdlJoyAxisToInput(i, getJoystickAxisValue(i)) == (GamepadInput)button) {
                return true;
            }
        }
    }
    return false;
}

bool isJoystickAxisButtonJustReleased() noexcept { // [GEC]
    for (uint32_t i = 0; i <= 5; i++) {
        if (isJoystickAxisJustReleased(i)) {
            removeValueFromVector(i, gJoystickAxesJustReleased);
            return true;
        }
    }
    return false;
}


bool isGamepadAxisInputPressed(GamepadInput inputIn) noexcept { // [GEC]
    for (int i = (int)GamepadInput::AXIS_LEFT_X; i <= (int)GamepadInput::AXIS_TRIG_RIGHT; i++) {
        const GamepadInput input = (GamepadInput)i;
        const uint8_t inputIdx = (uint8_t)input;
        if (isGamepadInputPressed(input)) {
            if (sdlAxisToInput2(input, gGamepadInputs[inputIdx]) == inputIn) {
                removeValueFromVector(input, gGamepadInputsPressed);
                return true;
            }
        }
    }
    return false;
}

bool isGamepadAxisInputJustPressed(GamepadInput inputIn) noexcept { // [GEC]
    for (int i = (int)GamepadInput::AXIS_LEFT_X; i <= (int)GamepadInput::AXIS_TRIG_RIGHT; i++) {
        const GamepadInput input = (GamepadInput)i;
        const uint8_t inputIdx = (uint8_t)input;
        if (isGamepadInputJustPressed(input)) {
            if (sdlAxisToInput2(input, gGamepadInputs[inputIdx]) == inputIn) {
                return true;
            }
        }
    }
    return false;
}

bool isGamepadAxisInputJustReleased() noexcept { // [GEC]
    for (int i = (int)GamepadInput::AXIS_LEFT_X; i <= (int)GamepadInput::AXIS_TRIG_RIGHT; i++) {
        const GamepadInput input = (GamepadInput)i;
        if (isGamepadInputJustReleased(input)) {
            removeValueFromVector(input, gGamepadInputsJustReleased);
            return true;
        }
    }
    return false;
}
//----------------------

void controllerVibrate(int duration_ms) noexcept {
    float intensity = (float)gVibrationIntensity / 100.f;
    // Use game controller
    if (gpGameController) {
        float frequency = (float)0xFFFF * intensity;
        SDL_GameControllerRumble(gpGameController, 0, 0, 1); // Stop
        SDL_GameControllerRumble(gpGameController, (int)frequency, (int)frequency, duration_ms);
    }
    //Use haptics
    else if (gJoyHaptic) {
        SDL_HapticRumbleStop(gJoyHaptic);
        SDL_HapticRumblePlay(gJoyHaptic, 1.0 * intensity, duration_ms);
    }
}

Input::Input() {
}

Input::~Input() {
    emptyAndShrinkVector(gGamepadInputsJustReleased);
    emptyAndShrinkVector(gGamepadInputsJustPressed);
    emptyAndShrinkVector(gGamepadInputsPressed);
}

// Port: set default Binds
void Input::init() {
    std::memcpy(keyMapping, keyMappingDefault, sizeof(keyMapping));
    std::memcpy(keyMappingTemp, keyMappingDefault, sizeof(keyMapping));

    gDeadZone = 50;
    gVibrationIntensity = 80;

    gGamepadInputsPressed.reserve(NUM_GAMEPAD_INPUTS);
    gGamepadInputsJustPressed.reserve(NUM_GAMEPAD_INPUTS);
    gGamepadInputsJustReleased.reserve(NUM_GAMEPAD_INPUTS);
    // Configure our supported input layout: a single player with standard controller styles
#ifdef __aarch64__
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
    #endif
}

void Input::unBind(int* keyBinds, int index)
{
    int temp[KEYBINDS_MAX], next, i;

    next = 0;
    keyBinds[index] = -1;

    // Reorder the list
    for (i = 0; i < KEYBINDS_MAX; i++) {
        temp[i] = -1;
        if (keyBinds[i] == -1) {
            continue;
        }
        temp[next++] = keyBinds[i];
    }

    std::memcpy(keyBinds, temp, sizeof(temp));
}

void Input::setBind(int* keyBinds, int keycode) {
    int i;

    // Examina si existe anteriormente, si es as�, se desvincular� de la lista
    // Examines whether it exists previously, if so, it will be unbind from the list
    for (i = 0; i < KEYBINDS_MAX; i++) {
        if (keyBinds[i] == keycode) {
            this->unBind(keyBinds, i);
            return;
        }
    }

    // Se guarda el key code en la lista
    // The key code is saved in the list
    for (i = 0; i < KEYBINDS_MAX; i++) {
        if (keyBinds[i] == -1) {
            keyBinds[i] = keycode;
            return;
        }
    }
}

void Input::setInputBind(int scancode) {
    Applet* app = CAppContainer::getInstance()->app;
    int keyMapId = app->menuSystem->items[app->menuSystem->selectedIndex].param;
    this->setBind(keyMappingTemp[keyMapId].keyBinds, scancode);
}

void Input::handleEvents() noexcept {
    Applet* app = CAppContainer::getInstance()->app;
	SDL_Event sdlEvent;
    SDLGL* sdlGL = CAppContainer::getInstance()->sdlGL;
    Canvas* canvas = CAppContainer::getInstance()->app->canvas;

#ifdef __aarch64__
padUpdate(&pad);
    u64 kDown = padGetButtonsDown(&pad);
    u64 kHeld = padGetButtons(&pad);
    u64 kUp   = padGetButtonsUp(&pad);
    if (kDown != kDownOld || kHeld != kHeldOld || kUp != kUpOld) {

        if (kDown & HidNpadButton_A)
            canvas->addEvents(AVK_SELECT | AVK_MENU_SELECT);
        if (kDown & HidNpadButton_B)
            canvas->addEvents(AVK_PASSTURN);
        if (kDown & HidNpadButton_X)
            canvas->addEvents(AVK_PDA);
        if (kDown & HidNpadButton_Y)
            canvas->addEvents(AVK_BOTDISCARD | AVK_ITEMS_INFO);

        if (kDown & HidNpadButton_Plus)
            canvas->addEvents(AVK_MENU_OPEN | AVK_MENUOPEN);
        if (kDown & HidNpadButton_Minus)
            canvas->addEvents(AVK_AUTOMAP);

        if (kDown & HidNpadButton_L)
            canvas->addEvents(AVK_PREVWEAPON);
        if (kDown & HidNpadButton_R)
            canvas->addEvents(AVK_NEXTWEAPON);
        if (kDown & HidNpadButton_ZL)
            canvas->addEvents(AVK_MOVELEFT);
        if (kDown & HidNpadButton_ZR)
            canvas->addEvents(AVK_MOVERIGHT);
        if (kDown & HidNpadButton_StickRLeft)
            canvas->addEvents(AVK_MOVELEFT);
        if (kDown & HidNpadButton_StickRRight)
            canvas->addEvents(AVK_MOVERIGHT);

        if (kDown & HidNpadButton_Up)
            canvas->addEvents(AVK_UP | AVK_MENU_UP);
        else if (kDown & HidNpadButton_Down)
            canvas->addEvents(AVK_DOWN | AVK_MENU_DOWN);
        if (kDown & HidNpadButton_Left)
            canvas->addEvents(AVK_LEFT | AVK_MENU_PAGE_UP);
        else if (kDown & HidNpadButton_Right)
            canvas->addEvents(AVK_RIGHT | AVK_MENU_PAGE_DOWN);
        else if (kDown & HidNpadButton_StickLUp)
            canvas->addEvents(AVK_UP | AVK_MENU_UP);
        else if (kDown & HidNpadButton_StickLDown)
            canvas->addEvents(AVK_DOWN | AVK_MENU_DOWN);
        else if (kDown & HidNpadButton_StickLLeft)
            canvas->addEvents(AVK_LEFT | AVK_MENU_PAGE_UP);
        else if (kDown & HidNpadButton_StickLRight)
            canvas->addEvents(AVK_RIGHT | AVK_MENU_PAGE_DOWN);

        else if (kDown & HidNpadButton_StickR)
            canvas->addEvents(AVK_DRINKS);
        else if (kDown & HidNpadButton_StickL)
            canvas->addEvents(AVK_ITEMS_INFO);
        else if (kDown & HidNpadButton_StickR)
            canvas->addEvents(AVK_DRINKS);
        #elif __3DS__
    hidScanInput();
    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();
    u32 kUp   = hidKeysUp();
    if (kDown != kDownOld || kHeld != kHeldOld || kUp != kUpOld) {

        if (kDown & KEY_A)
            canvas->addEvents(AVK_SELECT | AVK_MENU_SELECT);
        if (kDown & KEY_B)
            canvas->addEvents(AVK_PASSTURN);
        if (kDown & KEY_X)
            canvas->addEvents(AVK_PDA);
        if (kDown & KEY_Y)
            canvas->addEvents(AVK_BOTDISCARD | AVK_ITEMS_INFO);

        if (kDown & KEY_START)
            canvas->addEvents(AVK_MENU_OPEN | AVK_MENUOPEN);
        if (kDown & KEY_SELECT)
            canvas->addEvents(AVK_AUTOMAP);

        if (kDown & KEY_L)
            canvas->addEvents(AVK_PREVWEAPON);
        if (kDown & KEY_R)
            canvas->addEvents(AVK_NEXTWEAPON);
        if (kDown & KEY_ZL)
            canvas->addEvents(AVK_MOVELEFT);
        if (kDown & KEY_ZR)
            canvas->addEvents(AVK_MOVERIGHT);
        if (kDown & KEY_CSTICK_LEFT)
            canvas->addEvents(AVK_MOVELEFT);
        if (kDown & KEY_CSTICK_RIGHT)
            canvas->addEvents(AVK_MOVERIGHT);

        if (kDown & KEY_UP)
            canvas->addEvents(AVK_UP | AVK_MENU_UP);
        else if (kDown & KEY_DOWN)
            canvas->addEvents(AVK_DOWN | AVK_MENU_DOWN);
        if (kDown & KEY_LEFT)
            canvas->addEvents(AVK_LEFT | AVK_MENU_PAGE_UP);
        else if (kDown & KEY_RIGHT)
            canvas->addEvents(AVK_RIGHT | AVK_MENU_PAGE_DOWN);
        else if (kDown & KEY_CPAD_UP)
            canvas->addEvents(AVK_UP | AVK_MENU_UP);
        else if (kDown & KEY_CPAD_DOWN)
            canvas->addEvents(AVK_DOWN | AVK_MENU_DOWN);
        else if (kDown & KEY_CPAD_LEFT)
            canvas->addEvents(AVK_LEFT | AVK_MENU_PAGE_UP);
        else if (kDown & KEY_CPAD_RIGHT)
            canvas->addEvents(AVK_RIGHT | AVK_MENU_PAGE_DOWN);

        //else if (kDown & KEY_StickR)
          //  canvas->addEvents(AVK_DRINKS);
        //else if (kDown & KEY_StickL)
          //  canvas->addEvents(AVK_ITEMS_INFO);
        //else if (kDown & KEY_StickR)
          //  canvas->addEvents(AVK_DRINKS);
#endif
        kDownOld = kDown;
        kHeldOld = kHeld;
        kUpOld = kUp;

    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Discards input events and movements.
// Should be called whenever inputs have been processed for a frame.
//------------------------------------------------------------------------------------------------------------------------------------------
void Input::consumeEvents() noexcept {
    const Canvas* canvas = CAppContainer::getInstance()->app->canvas;
    // Clear all events
    if (!canvas->keyDownCausedMove) {
        gKeyboardKeysJustPressed.clear();
        gKeyboardKeysJustReleased.clear();

        gGamepadInputsJustPressed.clear();
        gGamepadInputsJustReleased.clear();

        gJoystickAxesJustPressed.clear();
        gJoystickAxesJustReleased.clear();
        gJoystickButtonsJustPressed.clear();
        gJoystickButtonsJustReleased.clear();
    }

    // Clear other events
    //gbWindowFocusJustLost = false;
}