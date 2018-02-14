#include <stdint.h>
#include "hal_general.h"
#include "hal_uart.h"
#include "uart.h"
#include "system.h"
#include "macros.h"

#define Set_U1Tx_PPS(pin) CAT5(RPOR, OUTPUT_PIN_TO_REG_NUM(pin), bits.RP, pin, R) = 3 // U1TX
#define Set_U1Rx_PPS(pin) RPINR18bits.U1RXR = pin

// defaults for PPS if not set in settings.h
#ifndef UART1_TX_PIN
#define UART1_TX_PIN 19
#endif
#ifndef UART1_RX_PIN
#define UART1_RX_PIN 17
#endif

#ifndef UART_INTERRUPT_PRIORITY
#define UART_INTERRUPT_PRIORITY 3
#endif

#ifndef PERIPHERAL_CLOCK
#define PERIPHERAL_CLOCK FCPU
#endif

// private function, could be changed to public if needed
static void SetBaud(uint8_t n, uint32_t baud);

void hal_UART_Init(uint8_t channel, uint32_t baud) {
    switch (channel) {
        case UART1_CH:
            Set_U1Tx_PPS(UART1_TX_PIN);       // UART1_TX_PIN should be defined in settings.h
                                              // e.g. #define UART1_TX_PIN 10
                                              // Options are 1-25
            Set_U1Rx_PPS(UART1_RX_PIN);       // UART1_RX_PIN should be defined in settings.h
                                              // Options are 1-25
            IPC2bits.U1RXIP = UART_INTERRUPT_PRIORITY;
			IPC3bits.U1TXIP = UART_INTERRUPT_PRIORITY;
            // interrupt when transmit buffer is empty, enable RX and TX, rx interrupt
            // when not empty
                    //FEDCBA9876543210
            U1STA = 0b1000000000000000;
            break;
    }
    hal_UART_Disable(channel);
    SetBaud(channel, baud);
    hal_UART_ClearRxIF(channel);
    hal_UART_EnableRxInterrupt(channel);
    hal_UART_Enable(channel);
    hal_UART_TxEnable(channel);
}

void hal_UART_Enable(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            U1MODEbits.UARTEN = 1;
            break;
        default:
            return;
    }
}

void hal_UART_Disable(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            U1MODEbits.UARTEN = 0;
            break;
        default:
            return;
    }
}

void hal_UART_TxEnable(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            U1STAbits.UTXEN = 1;
            break;
        default:
            return;
    }
}

void hal_UART_EnableRxInterrupt(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            IEC0bits.U1RXIE = 1;
            break;
        default:
            return;
    }
}

void hal_UART_EnableTxInterrupt(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            IEC0bits.U1TXIE = 1;
            break;
        default:
            return;
    }
}

void hal_UART_DisableRxInterrupt(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            IEC0bits.U1RXIE = 0;
            break;
        default:
            return;
    }
}

void hal_UART_DisableTxInterrupt(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            IEC0bits.U1TXIE = 0;
            break;
        default:
            return;
    }
}

void hal_UART_TxByte(uint8_t channel, uint8_t c) {
    switch (channel) {
        case UART1_CH:
            U1TXREG = c;
            break;
        default:
            return;
    }
}

uint8_t hal_UART_RxByte(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            return U1RXREG;
        default:
            return 0;
    }
}

void hal_UART_ClearTxIF(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            IFS0bits.U1TXIF = 0;
            break;
        default:
            return;
    }
}

void hal_UART_ClearRxIF(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            IFS0bits.U1RXIF = 0;
            break;
        default:
            return;
    }
}

uint8_t hal_UART_DataAvailable(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            return U1STAbits.URXDA;
        default:
            return 0;
    }
}

// non typical function
uint8_t hal_UART_DoneTransmitting(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            return U1STAbits.TRMT;
        default:
            return 0;
    }
}

uint8_t hal_UART_SpaceAvailable(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            return !U1STAbits.UTXBF;
        default:
            return 0;
    }
}

void SetBaud(uint8_t n, uint32_t baud) {
    int32_t error, error1, error2;
    uint16_t brg, brg1, brg2;
    uint32_t baud2, baud1;
    uint8_t brgh;
    brg1 = PERIPHERAL_CLOCK / (16 * baud) - 1;
    brg2 = brg1 + 1;
    baud1 = PERIPHERAL_CLOCK / (16 * (brg1 + 1));
    baud2 = PERIPHERAL_CLOCK / (16 * (brg2 + 1));
    if (baud1 > baud) error1 = baud1 - baud;
    else error1 = baud - baud1;
    if (baud2 > baud) error2 = baud2 - baud;
    else error2 = baud - baud2;
    if (error1 < error2) {
        error = error1;
        brg = brg1;
    } else {
        error = error2;
        brg = brg2;
    }
    if (error * 1000 / baud > 25) {
        brg1 = PERIPHERAL_CLOCK / (4 * baud) - 1;
        brg2 = brg1 + 1;
        baud1 = PERIPHERAL_CLOCK / (4 * (brg1 + 1));
        baud2 = PERIPHERAL_CLOCK / (4 * (brg2 + 1));
        if (baud1 > baud) error1 = baud1 - baud;
        else error1 = baud - baud1;
        if (baud2 > baud) error2 = baud2 - baud;
        else error2 = baud - baud2;
        if (error1 < error2) {
            brg = brg1;
        } else {
            brg = brg2;
        }
        brgh = 1;
    } else {
        brgh = 0;
    }

    switch (n) {
        case UART1_CH:
            U1MODEbits.BRGH = brgh;
            U1BRG = brg;
            break;
        default:
            break;
    }
}

uint8_t hal_UART_RxInterruptEnabled(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            return IEC0bits.U1RXIE;
        default:
            return 0;
    }
}

uint8_t hal_UART_TxInterruptEnabled(uint8_t channel) {
    switch (channel) {
        case UART1_CH:
            return IEC0bits.U1TXIE;
        default:
            return 0;
    }
}

void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void) {
    UART_Rx_Handler(UART1_CH);
    hal_UART_ClearRxIF(UART1_CH);
}

void __attribute__((interrupt, auto_psv)) _U1TXInterrupt(void) {
    UART_Tx_Handler(UART1_CH);
    hal_UART_ClearTxIF(UART1_CH);
}
