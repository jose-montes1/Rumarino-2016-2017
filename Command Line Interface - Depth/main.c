#include <msp430.h> 
#include "Serial_JMPv2.2.h"
#include "CMD_JMP.h"
#include <stdlib.h>
#include "MS5837-30BA.h"
#include "Motors_JMP.h"




#define FULL_SCALE_RATIO 20
#define FEET_TO_PWM 6.25
#define BAR_TO_FEET 33.4552565551477
#define MBAR_TO_BAR 10000.00


struct systemStatus{

    unsigned char terminalOutput : 1;
    unsigned char continuousOutput : 1;
    unsigned char depthController : 1;
    unsigned char systemRunning : 1;

    float depth_setPoint;
    float depth_curr;
    float depth_error;
    float depth_bias;
    int depth_gain;
    float actual_depth_bars;
    float actual_depth_milibars;
    float actual_depth_feet;
    float actual_depth_pwm;


    int16 pressure;




    int m_speed;


} status;



int helpMenu(char *command){
    if(strCmp("h", command, 1)){
        //Print menu
        USB_println("\n\r****************USER HELP GUIDE****************\n\rCommands: \n\r"
                "1. o - depth controller commands\n\r"
                " Parameters:\n\r"
                "   n - turns on controller\n\r"
                "   s - sets the set point\n\r"
                "		Sub parameters:\n\r"
                "			0-9 | q,w,e,r,t,t - assigns the set point\n\r"
                "   g - sets the controller gain\n\r"
                "     Sub-Parameters: \n\r"
                "         0-9 - controller gain\n\r"
                "   d - Changes the sub bias point\n\r"
                "       Sub-Parameters: \n\r"
                "         1 - sets the bias to 29 (default)\n\r"
                "         2 - sets the bias to 14.5 (sens error)\n\r"
                "   x - turns the controller off\n\r"
                "2. u - prints information about the system\n\r"
                "3. j - Enables continuous output of the system \n\r"
                "4.	k - Disables continuous output\n\r"
                "5. p - prints identification number\n\r"
                "6. x - turns the system off \n\r"
                "       - press s to start again\n\r"
                "7. h - help menu display\n\r"
                "***********************************************\n\r");
        return 1;
    }
    return -1;
}

int printFrame(char *command){
    if(strCmp("u", command, 1)){
        status.terminalOutput = 1;
        return 1;
    }
    return -1;
}

int eContFrames(char *command){
    if(strCmp("j", command, 1)){
        status.continuousOutput = 1;
        return 1;
    }
    return -1;
}

int dContFrames(char *command){
    if(strCmp("k", command, 1)){
        status.continuousOutput = 0;
        return 1;
    }
    return -1;
}

int depthON(char *command){
    if(strCmp("on", command, 2)){
        status.depthController = 1;
        return 1;
    }
    return -1;
}

int depthOFF(char *command){
    if(strCmp("ox", command, 2)){
        status.depthController = 0;           //creo que debe ser depth controllers - 0
        MOTOR_speed(0, V_MOTORS);
        return 1;
    }
    return -1;
}

int depthBias(char *command){
    if(strCmp("od", command, 2)){
        if(status.depth_bias == 29){
            status.depth_bias = 14.5;
        }else{
            status.depth_bias = 29;
        }return 1;
    }
    return -1;
}

int depthBiasM(char *command){
    if(strCmp("ob", command, 2)){
        PRESSURE_start_conversion();
        int16 tempPressure = PRESSURE_get_pressure();
        status.depth_bias = (float) (BAR_TO_FEET*((tempPressure)/MBAR_TO_BAR ));
        return 1;
    }
    return -1;
}


int depthSetPoint(char *command){
    if(strCmp("os", command, 2)){
        char setPoint = *(command + 2);
        switch (setPoint){
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0': status.depth_setPoint = (setPoint - '0'); break;
        case 'q': status.depth_setPoint = 11; break;
        case 'w': status.depth_setPoint = 12; break;
        case 'e': status.depth_setPoint = 13; break;
        case 'r': status.depth_setPoint = 14; break;
        case 't': status.depth_setPoint = 15; break;
        case 'y': status.depth_setPoint = 16; break;
        default: return 0;
        }
        return 1;
    }
    return -1;
}


int depthGain (char* command){
    if(strCmp("og", command, 2)){
        char gain = *(command + 2);
        if( gain <= '9' && gain >= '0'){
            status.depth_gain = *(command+2) - '0';
            return 1;
        }
    }
    return -1;
}




int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    USB_setup(9600);
    I2C_setup(400000);
    MOTOR_ultra_setup();
    _BIS_SR(GIE);

    PRESSURE_calibrate();

    PRESSURE_start_conversion();
    status.pressure = PRESSURE_get_pressure();

    struct commandList commands;

    initializeCommandList(&commands, 15);
    addCommand(&commands, &helpMenu);
    addCommand(&commands, &printFrame);
    addCommand(&commands, &eContFrames);
    addCommand(&commands, &dContFrames);
    addCommand(&commands, &depthON);
    addCommand(&commands, &depthOFF);
    addCommand(&commands, &depthSetPoint);
    addCommand(&commands, &depthGain);
    addCommand(&commands, &depthBias);
    addCommand(&commands, &depthBiasM);


    char cmd[COMMAND_BUFFER_SIZE], *transactionComplete = 0;
    transactionComplete = malloc(1);

    status.depth_bias = 29;
    status.depth_curr = 0;
    status.depth_gain = 0;
    status.depth_setPoint = 0;
    status.m_speed = 0;

    USB_getline_a(cmd, transactionComplete);


    while(1){
        if(*transactionComplete){
            if(verifyCommand(&commands, cmd) > 0){
                USB_print("Entered a valid command: ");
            }else{
                USB_print("Entered an invalid command: ");
            }
            USB_println(cmd);
            *cmd = '\0';
            USB_getline_a(cmd, transactionComplete);
        }

        /* Depth controller section*/
        if(status.depthController){
            PRESSURE_start_conversion();
            status.pressure = PRESSURE_get_pressure();

            //            status.depth_curr = (float) ((BAR_TO_FEET*((status.pressure)/MBAR_TO_BAR ))- status.depth_bias);       // convert bars to milibars
            //            status.depth_error = (float) (status.depth_setPoint - status.depth_curr);      //calculate error
            //            status.m_speed = status.depth_error*status.depth_gain;


            status.actual_depth_bars = (float)33.4552565551477*(status.pressure);       // convert bars to milibars
            status.actual_depth_milibars = (float)(status.actual_depth_bars/10000); //convert milibars to feet
            status.actual_depth_feet = (float)(status.actual_depth_milibars - status.depth_bias);  // calibrate depending on altitude above sea
            status.actual_depth_pwm = (status.actual_depth_feet);

            status.depth_error = (status.depth_setPoint - status.actual_depth_pwm)*FEET_TO_PWM;      //calculate error

            status.m_speed = status.depth_gain*status.depth_error;

            if((status.m_speed) > MOTOR_CAP)
                status.m_speed = MOTOR_CAP;
            if(status.m_speed < -1*MOTOR_CAP)
                status.m_speed = -1*MOTOR_CAP;

            MOTOR_speed(status.m_speed, V_MOTORS);

            __delay_cycles(200000);
        }


        if(status.terminalOutput | status.continuousOutput){
            USB_print_float("  pressurre: ", status.pressure);
            USB_print_value(" depth set point: ", status.depth_setPoint);
            USB_print_float("  depth: ", status.actual_depth_feet);
            USB_print_float("  error: ", status.depth_error);
            USB_print_value("  motors: ", status.m_speed);

            USB_println(" ");

            status.terminalOutput = 0;
        }
    }
}











