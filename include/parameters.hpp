#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#define HT_DEBUG_EN
#define DEBUG 0
// TODO may wanna do this another way

// Pedalbox stuff

//CRUISE CONTROL
#define SET_RPM 1000
#define D_KP 4.1
#define D_KI 0
#define D_KD 0
#define D_OUTPUT_MIN 0.0
#define D_OUTPUT_MAX (TORQUE_1 * 10)
#define BANGBANG_RANGE 1000.0
#define PID_TIMESTEP 10.0
#define PID_MODE false

#define BRAKE_ACTIVE 2000             // Threshold for brake pedal active
#define MIN_ACCELERATOR_PEDAL_1 200    // Low accelerator implausibility threshold
#define START_ACCELERATOR_PEDAL_1 2267  // Position to start acceleration
#define END_ACCELERATOR_PEDAL_1 2700    // Position to max out acceleration
#define MAX_ACCELERATOR_PEDAL_1 4000    // High accelerator implausibility threshold
#define MIN_ACCELERATOR_PEDAL_2 200    // Low accelerator implausibility threshold
#define START_ACCELERATOR_PEDAL_2 1504  // Position to start acceleration
#define END_ACCELERATOR_PEDAL_2 1750    // Position to max out acceleration
#define MAX_ACCELERATOR_PEDAL_2 4000    // High accelerator implausibility threshold
#define HALF_ACCELERATOR_PEDAL_1 ((START_ACCELERATOR_PEDAL_1 + END_ACCELERATOR_PEDAL_1) / 2)
#define HALF_ACCELERATOR_PEDAL_2 ((START_ACCELERATOR_PEDAL_2 + END_ACCELERATOR_PEDAL_2) / 2)


#define MIN_HV_VOLTAGE 600 // apparently this is divided by ten?

// #define HT_DEBUG_EN
// Torque Calculation Defines
#define ALPHA 0.9772
#define TORQUE_1 20
#define TORQUE_2 240
// Pump speed
#define PUMP_SPEED 4090

// neo-pixel specific
#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0x10EF0078
#define ORANGE 0xE05800
#define WHITE  0xFF000000
#define BLACK  0x0
#define BRIGHTNESS 255

#endif