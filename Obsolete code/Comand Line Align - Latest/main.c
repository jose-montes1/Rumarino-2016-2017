#include <msp430.h> 
#include "Serial_JMPv2.2.h"
#include "CMD_JMP.h"
#include <stdlib.h>
#include "Motors_JMP.h"
#include "Razor_AHRS.h"
#include "General_JMP.h"



#define FULL_SCALE_RATIO 20
#define FEET_TO_PWM 6.25
#define ENTER_ALIGN 2
#define EXIT_ALIGN 2
#define CONTROLLER_ALIGN 2


struct systemStatus{

    unsigned char terminalOutput : 1;
    unsigned char continuousOutput : 1;
    unsigned char alignController : 1;
    unsigned char systemRunning : 1;

    float align_setPoint;
    float align_curr;
    float align_error;
    float align_error_abss;
    float align_output;

    int align_gain;
    int align_bias;


    int ml_speed;
    int mr_speed;
    int m_polarity;


    int thresh_fw_enter;
    int thresh_fw_exit;
    int thresh_cont;


} status;



int helpMenu(char *command){
    if(strCmp("h", command, 1)){
        //Print menu
        USB_println("\n\r****************USER HELP GUIDE****************\n\rCommands: \n\r"
                "1. a - align controller commands\n\r"
                " Parameters:\n\r"
                "   0-9 | q,w,e,r,t,t - assigns the set point\n\r"
                "   g - sets the controller gain\n\r"
                "     Sub-Parameters: \n\r"
                "         0-9 - controller gain\n\r"
                "   m - Changes the set point to were you are\n\r"
                "   x - turns the controller off\n\r"
                "2. f - goes forward the amount you specify\n\r"
                "     Sub-Parameters: \n\r"
                "         0 - 9 sets the time to go forwards\n\r"
                "3. b - goes backwards the amount you specify\n\r"
                "     Sub-Parameters: \n\r"
                "         0 - 9 sets the time to go forwards\n\r"
                "3. u - prints information about the system\n\r"
                "4. k - prints actual align in feet\n\r"
                "5. l - prints align error\n\r"
                "6. p - prints indentification number\n\r"
                "5. x - turns the system off \n\r"
                "       - press s to start again\n\r"
                "6. h - help menu display\n\r"
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

int alignON(char *command){
    if(strCmp("an", command, 2)){
        status.alignController = 1;
        return 1;
    }
    return -1;
}

int alignOFF(char *command){
    if(strCmp("ax", command, 2)){
        status.alignController = 0;
        status.ml_speed = 0;
        status.mr_speed = 0;
        MOTOR_speed(0, H_MOTORS);
        return 1;
    }
    return -1;
}

int alignToCurrent(char *command){
    if(strCmp("am", command, 2)){
        RAZOR_refresh_atvalue();
        RAZOR_get_yaw(&status.align_setPoint);
        if(status.align_setPoint < 0){
            status.align_setPoint +=  360;
        }
        return 1;
    }
    return -1;
}


//int alignSetPoint(char *command){
//    if(strCmp("as", command, 2)){
//        char setPoint = *(command + 2);
//        switch (setPoint){
//        case '1':
//        case '2':
//        case '3':
//        case '4':
//        case '5':
//        case '6':
//        case '7':
//        case '8':
//        case '9':
//        case '0': status.align_setPoint += (setPoint - '0')*-10; break;
//        case 'q': status.align_setPoint += 10; break;
//        case 'w': status.align_setPoint += 20; break;
//        case 'e': status.align_setPoint += 30; break;
//        case 'r': status.align_setPoint += 40; break;
//        case 't': status.align_setPoint += 50; break;
//        case 'y': status.align_setPoint += 60; break;
//        default: return 0;
//        }
//        if(status.align_setPoint > 360){
//          status.align_setPoint -= 360;
//        }else if(status.align_setPoint < 0){
//          status.align_setPoint += 360;
//        }
//        return 1;
//    }
//    return -1;
//}

int alignSetPoint(char *command){
    if(strCmp("as", command, 2)){
        char* setPoint = (command + 2);

        status.align_setPoint += atoi(setPoint);
        if(status.align_setPoint > 360){
            status.align_setPoint -= 360;
        }else if(status.align_setPoint < 0){
            status.align_setPoint += 360;
        }
        return 1;
    }
    return -1;
}


int alignGain (char* command){
    if(strCmp("ag", command, 2)){
        char gain = *(command + 2);
        if( gain <= '9' && gain >= '0'){
            status.align_gain = gain - '0';
            return 1;
        }
    }
    return -1;
}

int alignPolarity(char* command){
    if(strCmp("ad", command, 2)){
        if(status.m_polarity == 1){
            status.m_polarity = -1;
        }else{
            status.m_polarity = 1;
        }
        return 1;
    }
    return -1;
}

int threshFwEnter (char* command){
    if(strCmp("tf", command, 2)){
        char threshold = *(command + 2);
        if( threshold <= '9' && threshold >= '0'){
            status.thresh_fw_enter = threshold - '0';
            return 1;
        }
    }
    return -1;
}
int threshFwExit (char* command){
    if(strCmp("tg", command, 2)){
        char threshold = *(command + 2);
        if( threshold <= '9' && threshold >= '0'){
            status.thresh_fw_exit = threshold - '0';
            return 1;
        }
    }
    return -1;
}

int threshController (char* command){
    if(strCmp("th", command, 2)){
        char threshold = *(command + 2);
        if( threshold <= '9' && threshold >= '0'){
            status.thresh_cont = threshold - '0';
            return 1;
        }
    }
    return -1;
}







void forwards(int time, int speed){
    // ^ Received time and speed ^ \\

    //Error check
    if((speed) > 20)
        speed = 20;
    if((speed) < -20)
        speed = -20;
    //Get operational point
    float opPoint, currPoint, error, output, error_abss;
    int m5_speed_f, m6_speed_f;
    RAZOR_refresh_atvalue();
    RAZOR_get_yaw(&opPoint);
    if(opPoint < 0){
        opPoint +=  360;
    }
    //Send Speed to Motors
    MOTOR_speed(speed, R_MOTOR);
    MOTOR_speed(speed, L_MOTOR);    //__delay_cycles(200000);
    //Start counting
    unsigned char timeout;
    timeExceed(time, &timeout);


    int align_sat = 0;

    while(!timeout){
        MOTOR_speed(speed, R_MOTOR);
        MOTOR_speed(speed, L_MOTOR);    //__delay_cycles(200000);
        __delay_cycles(800000);

        RAZOR_refresh_atvalue();
        RAZOR_get_yaw(&currPoint);
        if(currPoint < 0){
            currPoint +=  360;
        }

        error = opPoint - currPoint;

        if(abs(error) > status.thresh_fw_enter){
            align_sat = 1;
        }

        while(align_sat && !timeout){
            TB0CTL &= ~MC_1;                 // Use up mode
            RAZOR_refresh_atvalue();
            RAZOR_get_yaw(&currPoint);
            if(currPoint < 0){
                currPoint +=  360;
            }
            error = opPoint - currPoint;

            error_abss = abss(error);

            if(error > 0){
                if(error>180){
                    error = error - 360;
                }else{                                  //si no es juan es pablo
                    error = error;
                }
            }else{
                if(error_abss > 180){
                    error = 360 - error_abss;
                }else{                                //si no es juan es pedro
                    error = error;
                }
            }
            if (error <= status.thresh_fw_exit && error >= -1*status.thresh_fw_exit){
                error = 0;
                align_sat = 0;
                TB0CTL |= MC_1;                 // Use up mode
            }
            output = error*status.align_gain;

            m5_speed_f = status.m_polarity*output;
            m6_speed_f = -1*status.m_polarity*output;

            if((m5_speed_f) > 20)
                m5_speed_f = 20;
            if((m6_speed_f) > 20)
                m6_speed_f = 20;
            if((m5_speed_f) < -20)
                m5_speed_f = -20;
            if((m6_speed_f) < -20)
                m6_speed_f = -20;
            MOTOR_speed(m5_speed_f, R_MOTOR);
            MOTOR_speed(m6_speed_f, L_MOTOR);
            __delay_cycles(200000);

            if(status.continuousOutput){
                USB_print_float(" Forward set point: ", opPoint);
                USB_print_float(" F  Current angle: ", currPoint);
                USB_println(" ");

            }
            //            USB_print_float("Current point: ", currPoint);
            //            USB_print_float("  Operational point: ", opPoint);
            //            USB_println("");
        }
    }
    MOTOR_speed(0, R_MOTOR);
    MOTOR_speed(0, L_MOTOR);    //__delay_cycles(200000);
}


int forwardsCmd (char *command){
    if(strCmp("f", command, 1)){
        int tiem = *(command + 1) - '0';
        if (tiem > 10 || tiem < '0') tiem = 10;
        forwards(tiem, -20);
        return 1;
    }
    return -1;
}

int BackCmd (char *command){
    if(strCmp("b", command, 1)){
        int tiem = *(command + 1) - '0';
        if (tiem > 10 || tiem < '0') tiem = 10;
        forwards(tiem, 20);
        return 1;
    }
    return -1;
}

int outputMag = 0;

int outputAngles(char *command){
    if(strCmp("imua", command ,4) > 0){
        RAZOR_sngl_setup();
        RAZOR_out_angles_t();
        outputMag = 0;
        return 1;
    }
    return -1;
}

int outputMagn(char *command){
    if(strCmp("imum", command ,4) > 0){
        RAZOR_sngl_setup();
        RAZOR_out_mag();
        outputMag = 1;
        return 1;
    }
    return -1;
}





int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    USB_setup(19200);
    MOTOR_ultra_setup();
    _BIS_SR(GIE);
    RAZOR_sngl_setup();
    RAZOR_out_angles_t();


    struct commandList commands;

    initializeCommandList(&commands, 20);
    addCommand(&commands, &helpMenu);
    addCommand(&commands, &printFrame);
    addCommand(&commands, &eContFrames);
    addCommand(&commands, &dContFrames);
    addCommand(&commands, &alignON);
    addCommand(&commands, &alignOFF);
    addCommand(&commands, &alignSetPoint);
    addCommand(&commands, &alignGain);
    addCommand(&commands, &alignToCurrent);
    addCommand(&commands, &alignPolarity);
    addCommand(&commands, &forwardsCmd);
    addCommand(&commands, &outputAngles);
    addCommand(&commands, &outputMagn);
    addCommand(&commands, &threshFwExit);
    addCommand(&commands, &threshFwEnter);
    addCommand(&commands, &threshController);
    addCommand(&commands, &BackCmd);

    char cmd[COMMAND_BUFFER_SIZE], *transactionComplete = 0;
    transactionComplete = malloc(1);

    status.align_bias = 29;
    status.align_curr = 0;
    status.align_gain = 0;
    status.align_setPoint = 0;
    status.ml_speed = 0;
    status.mr_speed = 0;
    status.m_polarity = -1;
    status.thresh_fw_enter = ENTER_ALIGN;
    status.thresh_fw_exit = EXIT_ALIGN;
    status.thresh_cont = CONTROLLER_ALIGN;

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

        /* align controller section*/
        if(status.alignController && !outputMag){

            RAZOR_refresh_atvalue();
            RAZOR_get_yaw(&status.align_curr);
            //Cambiar a 360
            if(status.align_curr < 0){
                status.align_curr +=  360;
            }
            // equation of differences for proportional controller
            status.align_error = (status.align_setPoint - status.align_curr);      //calculate error
            status.align_error_abss = abss(status.align_error);
            if(status.align_error > 0){
                if(status.align_error>180){
                    status.align_error = status.align_error - 360;
                }else{                                  //si no es juan es pablo
                    status.align_error = status.align_error;
                }
            }else{
                if(status.align_error_abss > 180){
                    status.align_error = 360 - status.align_error_abss;
                }else{                                //si no es juan es pedro
                    status.align_error = status.align_error;
                }
            }

            // Proportional Controller
            if(status.align_error <= status.thresh_cont && status.align_error >= -1*status.thresh_cont){
                status.align_error = 0;
            }
            status.align_output = status.align_gain*status.align_error;

            // send controller output to motors
            status.mr_speed = status.m_polarity*status.align_output;
            status.ml_speed = -1*status.m_polarity*status.align_output;

            // m5_speed debe ser motor derecho --> pin 1.5
            /// m6_speed debe ser motor izquierdo --> pin 1.4

            // Saturation
            if((status.mr_speed) > 20) status.mr_speed = 20;
            if((status.ml_speed) > 20) status.ml_speed = 20;
            if((status.mr_speed) < -20) status.mr_speed = -20;
            if((status.ml_speed) < -20) status.ml_speed = -20;

            MOTOR_speed(status.mr_speed, R_MOTOR);
            MOTOR_speed(status.ml_speed, L_MOTOR);

            __delay_cycles(200000);

        }
        if(outputMag){
            RAZOR_refresh_atvalue();
        }


        if(status.terminalOutput | status.continuousOutput){
            if(!outputMag){
                USB_print_float(" align set point: ", status.align_setPoint);
                USB_print_float("  Current angle: ", status.align_curr);
                USB_print_value("  motor r: ", status.mr_speed);
                USB_print_value("  motor l: ", status.ml_speed);
                USB_print_value(" polarity: ", status.m_polarity);
                USB_println(" ");
            }

            status.terminalOutput = 0;
        }
    }
}


