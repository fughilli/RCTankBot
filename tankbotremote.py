import pygame, sys, serial, base64, struct, time

ser = serial.Serial("COM7", 9600, timeout=1)

pygame.init()

xjs = 0

for i in range(pygame.joystick.get_count()):
    js = pygame.joystick.Joystick(i)
    if(not "XBOX" in js.get_name()):
        del js
    else:
        xjs = js
        xjs.init()

FLAG_MOTOR_ON = (0x01)

def clamp(val, mn, mx):
    return(max((mn, min((mx, val)))))

def genControlString(ls, rs, sy, sp, flag):
    return (base64.b64encode(struct.pack("hhbbB", ls, rs, sy, sp, flag)) + '\n')

def genControlStringf(ls, rs, sy, sp, flag):
    return genControlString(
    clamp(int(ls * 32768), -32768, 32767),
    clamp(int(rs * 32768), -32768, 32767),
    clamp(int(sy * 128), -128, 127),
    clamp(int(sp * 128), -128, 127),
    flag
    )

def writeControl(ls, rs, sy, sp):
    commstr = genControlStringf(ls, rs, sy, sp, 0 if (ls == 0 and rs == 0) else 1)
    ser.write(commstr)
    print commstr

def spitser():
    while(1):
        print ser.readline()

def deadzComp(v):
    if(abs(v) < 0.2):
        return 0
    return v

LXC = 0
LYC = 0

LXRC = 0
LYRC = 0

while(1):
    pygame.event.pump()

    XC = deadzComp(xjs.get_axis(0))
    YC = deadzComp(xjs.get_axis(1))

    XRC = deadzComp(xjs.get_axis(3))
    YRC = deadzComp(xjs.get_axis(4))

    if(not (LXC == XC and LYC == YC and XRC == LXRC and YRC == LYRC)):
        print "X: %+01.2f Y: %+01.2f RX: %+01.2f RY: %+01.2f" % (XC, YC, XRC, YRC)
        #print "L: %+01.2f R: %+01.2f" % (-YC + XC,-YC - XC)
        writeControl(-YC-XC,-YC+XC,-XRC,-YRC)

        #sys.stdout.write(ser.readline())

    LXC = XC
    LYC = YC

    LXRC = XRC
    LYRC = YRC

    time.sleep(0.02)
