int counter = 0;
int t4Max, t5Max, t8Max, t9Max = 0;
int t4Count, t5Count, t8Count, t9Count = 0;
const float diff = 0.9;
const int maxCount = 6;
const int trigVal = 3;
const int fontWidth = 8;
const int maxPosOffset = 128 - fontWidth;

struct touchPad {
    bool a, b, up, down, left, right, touched;
} readings;

bool printable[256] = {false};

void resetReadings() {
    readings.a = readings.b = readings.up = readings.down = readings.left = readings.right = readings.touched = false;
}

struct touchPad& getReadings() {
    resetReadings();

    int t4 = touchRead(T4);
    int t5 = touchRead(T5);
    int t8 = touchRead(T8);
    int t9 = touchRead(T9);
    if (t4 > t4Max) { t4Max = t4; }
    if (t5 > t5Max) { t5Max = t5; }
    if (t8 > t8Max) { t8Max = t8; }
    if (t9 > t9Max) { t9Max = t9; }

    if (t4Count < maxCount && t4 < (t4Max*diff)) {
        t4Count++;
    } else if (t4Count > 0 && t4 > (t4Max*diff)) {
        t4Count--;
    }
    if (t5Count < maxCount && t5 < (t5Max*diff)) {
        t5Count++;
    } else if (t5Count > 0 && t5 > (t5Max*diff)) {
        t5Count--;
    }
    if (t8Count < maxCount && t8 < (t8Max*diff)) {
        t8Count++;
    } else if (t8Count > 0 && t8 > (t8Max*diff)) {
        t8Count--;
    }
    if (t9Count < maxCount && t9 < (t9Max*diff)) {
        t9Count++;
    } else if (t9Count > 0 && t9 > (t9Max*diff)) {
        t9Count--;
    }

    int numButtons = 0;
    if (t8Count > trigVal && t9Count > trigVal) {
        readings.b = true;
        numButtons++;
    }
    if (t4Count > trigVal && t8Count > trigVal) {
        readings.a = true;
        numButtons++;
    }
    if (t4Count > trigVal && t5Count > trigVal) {
        readings.up = true;
        numButtons++;
    }
    if (t5Count > trigVal && t9Count > trigVal) {
        readings.down = true;
        numButtons++;
    }
    if (t5Count > trigVal && t8Count > trigVal) {
        readings.left = true;
        numButtons++;
    }
    if (t4Count > trigVal && t9Count > trigVal) {
        readings.right = true;
        numButtons++;
    }
    if (numButtons > 1) {
        resetReadings();
    }
    if (numButtons >= 1) {
        readings.touched = true;
    }
    return readings;
}

void trim(char * s) {
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);
}

void getNameViaDPAD(char* name, int maxLength) {
    int pos = 0;
    int posOffset = 0;
    int textOffset = 0;

    int maxWidth = maxLength * fontWidth;
    bool processedTouch = false;
    while (!readings.a) {
        counter++;

        display.clear();
        display.setFont(Roboto_Light_15);
        display.drawStringMaxWidth(0, 0, 128, "Enter Name:");
        display.setFont(Roboto_Mono_Light_13);
        display.drawStringMaxWidth(textOffset, 24, maxWidth, name);
        if ((counter/100)%2) {
            display.drawStringMaxWidth(posOffset, 28, 128, "_");
        }
        display.setFont(Roboto_Light_11);
        display.drawStringMaxWidth(24, 50, 128, "( B: Delete, A: Enter )");
        display.display();

        getReadings();
        if (readings.touched && processedTouch) {
            continue;
        }
        if (!readings.touched) {
            processedTouch = false;
            continue;
        }

        if (readings.b && name[pos] == 0) {
            readings.left = true;
        }
        if (readings.left && pos>0) {
            pos--;
            if ((posOffset-fontWidth) < 0) {
                textOffset += fontWidth;
            } else {
                posOffset -= fontWidth;
            }
        }
        if (readings.right && pos<(maxLength-2)) {
            if (name[pos] == 0) {
                name[pos] = ' ';
            }
            pos++;
            if ((posOffset+fontWidth) > maxPosOffset) {
                textOffset -= fontWidth;
            } else {
                posOffset += fontWidth;
            }
        }
        if (readings.up) {
            if (!printable[name[pos]]) {
                if (pos==0 || name[pos-1]==' ') name[pos] = 'A';
                else name[pos] = 'a';
            } else {
                while (!printable[++name[pos]]) {};
            }
        }
        if (readings.down) {
            if (!printable[name[pos]]) {
                if (pos==0 || name[pos-1]==' ') name[pos] = 'Z';
                else name[pos] = 'z';
            } else {
                while (!printable[--name[pos]]) {};
            }
        }
        if (readings.b) {
            for (int i=pos; i<(maxLength-1); i++) {
                name[i] = name[i+1];
            }
        }
        processedTouch = true;
    }
    trim(name);
}


void setupPrintable() {
    printable[32] = true; //space
    printable[33] = true; //!
    printable[34] = true; //"
    printable[35] = true; //#
    printable[36] = true; //$
    printable[37] = true; //%
    printable[38] = true; //&
    printable[39] = true; //'
    printable[40] = true; //(
    printable[41] = true; //)
    printable[42] = true; //*
    printable[43] = true; //+
    printable[44] = true; //,
    printable[45] = true; //-
    printable[46] = true; //.
    printable[47] = true; ///

    printable[48] = true; //0
    printable[49] = true; //1
    printable[50] = true; //2
    printable[51] = true; //3
    printable[52] = true; //4
    printable[53] = true; //5
    printable[54] = true; //6
    printable[55] = true; //7
    printable[56] = true; //8
    printable[57] = true; //9

    printable[58] = true; //:
    printable[59] = true; //;
    printable[60] = true; //<
    printable[61] = true; //=
    printable[62] = true; //>
    printable[63] = true; //?
    printable[64] = true; //@

    printable[65] = true; //A
    printable[66] = true; //B
    printable[67] = true; //C
    printable[68] = true; //D
    printable[69] = true; //E
    printable[70] = true; //F
    printable[71] = true; //G
    printable[72] = true; //H
    printable[73] = true; //I
    printable[74] = true; //J
    printable[75] = true; //K
    printable[76] = true; //L
    printable[77] = true; //M
    printable[78] = true; //N
    printable[79] = true; //O
    printable[80] = true; //P
    printable[81] = true; //Q
    printable[82] = true; //R
    printable[83] = true; //S
    printable[84] = true; //T
    printable[85] = true; //U
    printable[86] = true; //V
    printable[87] = true; //W
    printable[88] = true; //X
    printable[89] = true; //Y
    printable[90] = true; //Z

    printable[97] = true; //a
    printable[98] = true; //b
    printable[99] = true; //c
    printable[100] = true; //d
    printable[101] = true; //e
    printable[102] = true; //f
    printable[103] = true; //g
    printable[104] = true; //h
    printable[105] = true; //i
    printable[106] = true; //j
    printable[107] = true; //k
    printable[108] = true; //l
    printable[109] = true; //m
    printable[110] = true; //n
    printable[111] = true; //o
    printable[112] = true; //p
    printable[113] = true; //q
    printable[114] = true; //r
    printable[115] = true; //s
    printable[116] = true; //t
    printable[117] = true; //u
    printable[118] = true; //v
    printable[119] = true; //w
    printable[120] = true; //x
    printable[121] = true; //y
    printable[122] = true; //z
}