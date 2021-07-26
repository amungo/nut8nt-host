#include <iostream>
#include <fstream>

#define CHANNELS    4
#define BUF_SIZE    (1024 * 512)

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "Usage: " << argv[0] << " [options] in_filename out_filename" << endl;
        cout << "Options: -g - complex(float) file output" << endl;
        cout << "         -s - short(int16) file output" << endl;
        cout << "         -c - char(int8) file output" << endl;
        cout << "         -i - invert Q" << endl;
        return 0;
    }

    string inFileName, outFileName;
    bool complexFlag = false, shortFlag = false, charFlag = false, invertFlag = false;

    for (int i=1; i<argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'g':
                    complexFlag = true;
                    break;
                case 's':
                    shortFlag = true;
                    break;
                case 'c':
                    charFlag = true;
                    break;
                case 'i':
                    invertFlag = true;
                 break;
            }
        }
        else {
            if (inFileName.empty())
                inFileName = argv[i];
            else if (outFileName.empty())
                outFileName = argv[i];
        }
    }

    fstream file;
    file.open(inFileName, ios_base::in | ios_base::binary);

    if (!file.is_open()) {
        cout << "Open file " << inFileName << " error" << endl;
        return 0;
    }

    streampos fsize = 0;

    fsize = file.tellg();
    file.seekg( 0, ios::end );
    fsize = file.tellg() - fsize;
    file.seekg( 0, ios::beg );

    cout << "File size: " << fsize << endl;

    char* data = new char[BUF_SIZE];
    int32_t* data32 = (int32_t*)data;
    int16_t* data16 = (int16_t*)data;

    fstream file16[CHANNELS];
    fstream file8[CHANNELS];
    fstream fileGRC[CHANNELS];

    int16_t** sh16 = new int16_t*[CHANNELS];
    int8_t** sh8 = new int8_t*[CHANNELS];
    float** gr32 = new float*[CHANNELS];

    for (int i=0; i<CHANNELS; i++) {
        char fileName[128];

        if (shortFlag) {
            sh16[i] = new int16_t[BUF_SIZE];

            sprintf(fileName, "%s_channel_%d.int16", outFileName.data(), i);
            file16[i].open(fileName, ios_base::out | ios_base::binary);
        }
        if (charFlag) {
            sh8[i] = new int8_t[BUF_SIZE];

            sprintf(fileName, "%s_channel_%d.int8", outFileName.data(), i);
            file8[i].open(fileName, ios_base::out | ios_base::binary);
        }
        if (complexFlag) {
            gr32[i] = new float[BUF_SIZE];

            sprintf(fileName, "%s_channel_%d.grc", outFileName.data(), i);
            fileGRC[i].open(fileName, ios_base::out | ios_base::binary);
        }
    }



    unsigned long long last_prog = 0;

    for (unsigned long long i=0; i < fsize / BUF_SIZE; i ++) {
        file.read(data, BUF_SIZE);

        unsigned long long  j;
        for (j=0; j < BUF_SIZE / sizeof(uint32_t); j += CHANNELS) {
            for (int ch=0; ch < CHANNELS; ch++) {
                if (shortFlag) {
                    sh16[ch][j/2] = data16[(j+ch)*2];
                    sh16[ch][j/2+1] = data16[(j+ch)*2 + 1];
                }

                if (charFlag) {
                    sh8[ch][j/2] = (data16[(i+j+ch)*2] >> 8) & 0xFF;
                    sh8[ch][j/2+1] = (data16[(i+j+ch)*2 + 1] >> 8) & 0xFF;
                }

                if (complexFlag) {
                    gr32[ch][j/2] = (float)data16[(i+j+ch)*2];
                    gr32[ch][j/2+1] = (float)data16[(i+j+ch)*2 + 1];
                }
            }
        }

        for (int ch=0; ch < CHANNELS; ch++) {
            if (shortFlag)
                file16[ch].write((char*)sh16[ch], j*sizeof(uint16_t)/2);
            if (charFlag)
                file8[ch].write((char*)sh8[ch], j*sizeof(uint8_t)/2);
            if (complexFlag)
                fileGRC[ch].write((char*)gr32[ch], j*sizeof(float)/2);
        }

        unsigned long long prog = i * 100 / (fsize / BUF_SIZE);
        if (last_prog != prog) {
            cout << "\r                                 \r";
            cout << i * sizeof(uint32_t) << "/" << fsize << " (" << prog << "%)";
            flush(cout);
            last_prog = prog;
        }

    }

    cout <<endl;

    file.close();



    return 0;
}
