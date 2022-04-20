#include <axidma.h>

#include <complex>

#include "GNU_UDP_client.h"

bool run = true;

int ctrlC_count = 0;
void sig_int_handler(int signo) {
    printf("Stopping programm\n");

    run = false;

    ctrlC_count++;

    if (ctrlC_count == 2)
        exit(-1);
}


int main(int argc, char *argv[]) {
    printConfigs();

    signal(SIGINT, sig_int_handler);
    signal(SIGTERM, sig_int_handler);

    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " address" << std::endl;
        std::cout << " Channel range: [0;" << (CHANNELS_NUM - 1) << "]" << std::endl;
        std::cout << " port: 30137" << std::endl;

        return 0;
    }

    udp_client* client = new udp_client(argv[1], 30137);


    if (argc == 3) {
        uint8_t ch = argv[2][0] - '0';

        if (ch < 0 || ch >= CHANNELS_NUM) {
            std::cout << "Channel error! Range: [0;" << (CHANNELS_NUM - 1) << "]" << std::endl;
            return 0;
        }

        std::cout << "Channel: " << (int)ch << std::endl;

        udp_client* client = new udp_client(argv[1], 30137);


        DMAReader reader(DMA_BUF_SIZE, SUPER_BUFFER_SIZE, "dma_proxy_rx1");
        reader.start();

        std::complex<float> sendData[SAMPLES_NUM];

        while (run) {
            uint8_t* data = (uint8_t*)reader.read();

            if (data) {


                std::complex<short>* iq = (std::complex<short>*)(data);

                for (int i = 0; i < SAMPLES_NUM; i++) {
                    sendData[i] = iq[i * CHANNELS_NUM + ch];
                }


                client->send(((char*)sendData), sizeof(sendData));
            }
        }
        reader.finish();
        //Start.out(0);
    }

    return 0;
}

