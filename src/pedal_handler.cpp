#include "pedal_handler.hpp"

// initializes pedal's ADC
void PedalHandler::init_pedal_handler()
{
    pedal_ADC = ADC_SPI(DEFAULT_SPI_CS, DEFAULT_SPI_SPEED);
    pid_->setTimeStep(PID_TIMESTEP);
    //pid_->setBangBang(double(BANGBANG_RANGE));
}

int PedalHandler::calculate_torque(int16_t &motor_speed, int &max_torque)
{
    
    *current_ = (double)motor_speed;
    *set_ = (double)SET_RPM;
    
    pid_->run();
    int calculated_torque = 0;
    int torque1 = map(round(accel1_), START_ACCELERATOR_PEDAL_1, END_ACCELERATOR_PEDAL_1, 0, max_torque);
    int torque2 = map(round(accel2_), START_ACCELERATOR_PEDAL_2, END_ACCELERATOR_PEDAL_2, 0, max_torque);

    // torque values are greater than the max possible value, set them to max
    if (torque1 > max_torque)
    {
        torque1 = max_torque;
    }
    if (torque2 > max_torque)
    {
        torque2 = max_torque;
    }
    // compare torques to check for accelerator implausibility
    // calculated_torque = (torque1 + torque2) / 2; //TODO un-cheese this

    
    if(PID_MODE==true){
    if((torque1>(0.75*max_torque))) // TODO put here when we want cruise control control to take effect
    {
        calculated_torque = (int)*throttle_;
    } else {
        pid_->setIntegral(0);
        pid_->reset();
        calculated_torque = 0;
    }
    }else{
        calculated_torque=torque1;
    }

    if (calculated_torque > max_torque)
    {
        calculated_torque = max_torque;
    }
    if (calculated_torque < 0)
    {
        calculated_torque = 0;
    }
    //#if DEBUG
    if (timer_debug_raw_torque->check())
    {
        Serial.print("TORQUE REQUEST DELTA PERCENT: "); // Print the % difference between the 2 accelerator sensor requests
        Serial.println(abs(torque1 - torque2) / (double) max_torque * 100);
        Serial.print("MCU RAW TORQUE: ");
        Serial.println(calculated_torque);
        Serial.print("TORQUE 1: ");
        Serial.println(torque1);
        Serial.print("TORQUE 2: ");
        Serial.println(torque2);
        Serial.print("Accel 1: ");
        Serial.println(accel1_);
        Serial.print("Accel 2: ");
        Serial.println(accel2_);
        Serial.print("Brake1_ : ");
        Serial.println(brake1_);
        Serial.print("RPM vs TARGET: ");
        Serial.print(*current_);
        Serial.print(",");
        Serial.println(*set_);
    }
    //#endif
// if (abs(motor_speed) <= 1000)
//     {
//         if (calculated_torque >= 1600) //60NM
//         {
//             calculated_torque = 1600; // ideally limit torque at low RPMs, see how high this number can be raised
//         }
//     }
//     uint32_t calculated_power =  (calculated_torque/10)*motor_speed*0.104725;
//     if(calculated_power>80000){
//         calculated_torque=((80000*9.54)/motor_speed)*10;
//     }
//     if(calculated_torque<100){
//         calculated_torque=0;
//     }

 //   Serial.print(",");
 //   Serial.println(calculated_torque);
    return calculated_torque;
}

// returns true if the brake pedal is active
bool PedalHandler::read_pedal_values()
{
    /* Filter ADC readings */

    // accel1_ = ALPHA * accel1_ + (1 - ALPHA) * pedal_ADC.read_adc(ADC_ACCEL_1_CHANNEL);
    // accel2_ = ALPHA * accel2_ + (1 - ALPHA) * pedal_ADC.read_adc(ADC_ACCEL_2_CHANNEL);
    // brake1_ = ALPHA * brake1_ + (1 - ALPHA) * pedal_ADC.read_adc(ADC_BRAKE_1_CHANNEL);
    accel1_ = pedal_ADC.read_adc(ADC_ACCEL_1_CHANNEL);
    accel2_ = pedal_ADC.read_adc(ADC_ACCEL_2_CHANNEL);
    brake1_ = pedal_ADC.read_adc(ADC_BRAKE_1_CHANNEL);


#if DEBUG
    if (timer_debug_raw_torque->check()) 
    {
        Serial.print("ACCEL 1: ");
        Serial.print(accel1_);
        Serial.print(", ");
        Serial.println(accel1_,HEX);
        Serial.print("ACCEL 2: ");
        Serial.print(accel2_);
        Serial.print(", ");
        Serial.println(accel2_,HEX);
        Serial.print("BRAKE 1: ");
        Serial.print(brake1_);
        Serial.print(", ");
        Serial.println(brake1_,HEX);
        int max_torque = 2400;
        int torque1 = map(round(accel1_), START_ACCELERATOR_PEDAL_1, END_ACCELERATOR_PEDAL_1, 0, max_torque);
        int torque2 = map(round(accel2_), START_ACCELERATOR_PEDAL_2, END_ACCELERATOR_PEDAL_2, 0, max_torque);

        Serial.printf("TORQUE VALUES IF YOU WERE CALCULATING IT CURRENTLY: \n T1: %d\nT2: %d\n",torque1,torque2);
        Serial.print("percent difference: ");
        float torqSum =abs(torque1-torque2);
        float torqAvg = (torque1+torque2)/2;
        float asdf = torqSum/torqAvg;
        Serial.println(asdf);
   }
#endif
    VCUPedalReadings.set_accelerator_pedal_1(accel1_);
    VCUPedalReadings.set_accelerator_pedal_2(accel2_);
    VCUPedalReadings.set_brake_transducer_1(brake1_);
    VCUPedalReadings.set_brake_transducer_2(brake1_);
    CAN_message_t tx_msg;

    // Send Main Control Unit pedal reading message
    VCUPedalReadings.write(tx_msg.buf);
    tx_msg.id = ID_VCU_PEDAL_READINGS;
    tx_msg.len = sizeof(VCUPedalReadings);

    // write out the actual accel command over CAN
    if (pedal_out->check())
    {
        WriteToDaqCAN(tx_msg);
    }
    // only uses front brake pedal
    brake_is_active_ = (brake1_ >= BRAKE_ACTIVE);
    return brake_is_active_;
}

void PedalHandler::verify_pedals(bool &accel_is_plausible, bool &brake_is_plausible, bool &accel_and_brake_plausible, bool &impl_occ)
{
    int max_torque = TORQUE_1*10;
    int torque1 = map(round(accel1_), START_ACCELERATOR_PEDAL_1, END_ACCELERATOR_PEDAL_1, 0, max_torque);
    int torque2 = map(round(accel2_), START_ACCELERATOR_PEDAL_2, END_ACCELERATOR_PEDAL_2, 0, max_torque);
    float torqSum =abs(torque1-torque2);
    float torqAvg = (torque1+torque2)/2;
    float torqDiff = torqSum/torqAvg;

    if (accel1_ < MIN_ACCELERATOR_PEDAL_1 || accel1_ > MAX_ACCELERATOR_PEDAL_1)
    {
        accel_is_plausible = false;
#if DEBUG
        Serial.println("T.4.2.10 1 OUT OF RANGE");
        Serial.println(accel1_);
#endif
    }
    else if (accel2_ < MIN_ACCELERATOR_PEDAL_2 || accel2_ > MAX_ACCELERATOR_PEDAL_2)
    {
        accel_is_plausible = false;
#if DEBUG
        Serial.println("T.4.2.10 2 OUT OF RANGE");
        Serial.println(accel2_);
#endif
    }
    // check that the pedals are reading within 10% of each other
    // sum of the two readings should be within 10% of the average travel
    // T.4.2.4
//     else if (torqDiff*100 > 50)
//     {
// #if DEBUG
//         Serial.println("T.4.2.4");
//         Serial.printf("BAD computed deviation- %f%\n", torqDiff*100);
// #endif
//         accel_is_plausible = false;
//     }
    else
    {
#if DEBUG
        Serial.println("T.4.2.4");
        Serial.printf("OK computed deviation- %f%\n", torqDiff*100);
#endif
        // mcu_status.set_no_accel_implausability(true);
        accel_is_plausible = true;
    }

    // BSE check
    // EV.5.6
    // FSAE T.4.3.4
    if (brake1_ < 200 || brake1_ > 4000)
    {
        brake_is_plausible = false;
        Serial.println(brake1_);
    }
    else
    {
        brake_is_plausible = true;
    }

    // FSAE EV.5.7
    // APPS/Brake Pedal Plausability Check
    if (
        (
            (accel1_ > ((END_ACCELERATOR_PEDAL_1 - START_ACCELERATOR_PEDAL_1) / 4 + START_ACCELERATOR_PEDAL_1)) ||
            (accel2_ > ((END_ACCELERATOR_PEDAL_2 - START_ACCELERATOR_PEDAL_2) / 4 + START_ACCELERATOR_PEDAL_2))) &&
        brake_is_active_)
    {
        Serial.println("dawg, dont press em both");
        accel_and_brake_plausible = false;
    }
    else if (
        (accel1_ < ((END_ACCELERATOR_PEDAL_1 - START_ACCELERATOR_PEDAL_1) / 20 + START_ACCELERATOR_PEDAL_1)) &&
        (accel2_ < ((END_ACCELERATOR_PEDAL_2 - START_ACCELERATOR_PEDAL_2) / 20 + START_ACCELERATOR_PEDAL_2)))
    {
        accel_and_brake_plausible = true;
        implausibility_occured_ = false; // only here do we want to reset this flag
    } else
    {
        accel_and_brake_plausible = true;
    }

    if((!accel_and_brake_plausible) || (!brake_is_plausible) || (!accel_is_plausible))
    {
        implausibility_occured_ = true;
    }

    impl_occ =implausibility_occured_;

}