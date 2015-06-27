#include "extmain.h"

ExtServo yawServo(Wire), pitchServo(Wire);

uint8_t cbuf[128];
uint32_t cbufidx;
uint8_t dcbuf[128];


Base64StreamEncoder bse;

void extsetup()
{
    Wire.begin();
    //initMotors(21,20,17,16,22,23);
    initMotors(21,17,20,16,22,23);

    setMotor(MOTOR_A, 0);

    setMotor(MOTOR_B, 0);

    disableMotor(MOTOR_A | MOTOR_B);

    yawServo.attach(0x40, 4);
    pitchServo.attach(0x40, 5);

    Serial1.begin(9600);

    //Serial1.println("RC TankBot V0.1");

    cbufidx = 0;
}

float pos = 0.0f;

typedef struct
{
    int16_t lmotor_speed;
    int16_t rmotor_speed;
    int8_t yaw;
    int8_t pitch;
    uint8_t flags;
} control_t;

#define FLAG_ENABLE_MOTORS 0x01

void parseCommand(uint8_t* dcbuf)
{
    control_t* cont = (control_t*)dcbuf;

    int mYaw = map(cont->yaw, -128, 127, EXTSERVO_DEFAULT_MIN, EXTSERVO_DEFAULT_MAX);
    int mPitch = map(cont->pitch, -128, 127, EXTSERVO_DEFAULT_MIN, EXTSERVO_DEFAULT_MAX);
    int mLS = 0, mRS = 0;

    yawServo.write(mYaw);
    pitchServo.write(mPitch);

    if(cont->flags & FLAG_ENABLE_MOTORS)
    {
        enableMotor(MOTOR_A | MOTOR_B);

        mLS = map(cont->lmotor_speed, -32768, 32767, MOTOR_FULL_REVERSE, MOTOR_FULL_FORWARD);
        mRS = map(cont->rmotor_speed, -32768, 32767, MOTOR_FULL_REVERSE, MOTOR_FULL_FORWARD);

        setMotor(MOTOR_A, mLS);
        setMotor(MOTOR_B, mRS);
    }
    else
    {
        disableMotor(MOTOR_A | MOTOR_B);
    }

//    Serial1.println(cont->lmotor_speed);
//    Serial1.println(cont->rmotor_speed);
//    Serial1.println(cont->yaw);
//    Serial1.println(cont->pitch);
//    Serial1.println(cont->flags);

//    char printbuf[128];
//
//    char* idx = printbuf;
//
//    snprintf(printbuf, 128, "LS: %+5d RS: %+5d YAW: %+5d PITCH: %+5d FLAGS: %02x\r\n", mLS, mRS, mYaw, mPitch, cont->flags);
//
//    while(*idx)
//        Serial1.write(*(idx++));
}

void extloop()
{
    while(Serial1.available())
    {
        if(cbufidx == 128)
            cbufidx = 0;
        cbuf[cbufidx] = Serial1.read();
        //Serial1.println((int)cbuf[cbufidx]);
        if(cbuf[cbufidx] == '\n' || cbuf[cbufidx] == '\r')
        {
//            for(int i = 0; i < cbufidx; i++)
//            {
//                Serial1.write(cbuf[i]);
//            }
            if(bse.decodeBuffer(dcbuf, 128, cbuf, cbufidx))
                parseCommand(dcbuf);

            cbufidx = 0;
        }
        else
            cbufidx++;
    }

    //Serial1.write('a');

    delay(10);
}
