#include "hd44780.h"
#include "pio_hd44780.pio.h"
#include <hardware/dma.h>
#include <hardware/irq.h>
#include <pico/malloc.h>
#include <string.h>

#define HD44780_BUFFERSIZE 128

typedef uint16_t hd44780_ctrl_t;

static PIO hd44780_pio;
static int hd44780_sm;
static int hd44780_dmachannel;
static hd44780_ctrl_t hd44780_buffer[HD44780_BUFFERSIZE];
static volatile int hd44780_bufferwr = 0;
static volatile int hd44780_bufferrd = 0;
static volatile int hd44780_bufferrddma = 0;


static void hd44780_triggerdmatransaction();

static void hd44780_dmahandler()
{
    // clear the interrupt request
    dma_hw->ints0 = 1u << hd44780_dmachannel;

    hd44780_bufferrd = hd44780_bufferrddma;

    hd44780_triggerdmatransaction();
}

static hd44780_ctrl_t hd44780_makectrlwr(bool rs, uint8_t data)
{
    hd44780_ctrl_t ctrlword = (rs ? 0x8000 : 0) |
        (false ? 0x4000 : 0) | // RW
        (data << 6);

    return ctrlword;
}

static hd44780_ctrl_t hd44780_makectrlrd(bool rs, uint8_t data)
{
    hd44780_ctrl_t ctrlword = (rs ? 0x8000 : 0) |
        (true ? 0x4000 : 0) | // RW
        (data << 6);

    return ctrlword;
}

static int hd44780_getwritablesize()
{
    volatile int wr = hd44780_bufferwr;
    volatile int rd = hd44780_bufferrd;
    
    int remain;
    if (wr < rd)
    {
        remain = rd - wr;
    }
    else
    {
        remain = count_of(hd44780_buffer) - wr + rd;
    }

    return remain;
}

static void hd44780_triggerdmatransaction()
{
    volatile int wr = hd44780_bufferwr;
    volatile int rd = hd44780_bufferrd;

    assert(rd == hd44780_bufferrddma);

    int transferrable;
    if (rd <= wr)
    {
        transferrable = wr - rd;
    }
    else
    {
        transferrable = count_of(hd44780_buffer) - rd; // only to end of buffer - next part in next transaction
    }

    if (transferrable > 0)
    {
        dma_channel_set_read_addr(hd44780_dmachannel, hd44780_buffer + rd, false);

        rd += transferrable;
        if (rd == count_of(hd44780_buffer))
            rd = 0;

        hd44780_bufferrddma = rd;

        // trigger DMA transfer
        dma_channel_set_trans_count(hd44780_dmachannel, transferrable, true);
    }
}

static void hd44780_writectrls(const hd44780_ctrl_t *ptr, int len)
{
    while (len > 0)
    {
        volatile int wr = hd44780_bufferwr;
        volatile int rd = hd44780_bufferrd;

        int writable;
        if (wr < rd)
            writable = rd - wr - 1;
        else
        {
            writable = count_of(hd44780_buffer) - wr;
            if (rd == 0)
                writable -= 1;
        }

        int towrite = MIN(len, writable);
        if (towrite > 0)
        {
            memcpy(hd44780_buffer + wr, ptr, towrite * sizeof(hd44780_ctrl_t));

            wr += towrite;
            if (wr == count_of(hd44780_buffer))
                wr = 0;
            ptr += towrite;
            len -= towrite;
            
            hd44780_bufferwr = wr;

            irq_set_enabled(DMA_IRQ_0, false);
            
            if (hd44780_bufferrd == hd44780_bufferrddma)
                hd44780_triggerdmatransaction();

            irq_set_enabled(DMA_IRQ_0, true);
        }
        else
        {
            // not enough space - try it again
        }

    }
}

static void hd44780_writectrl(hd44780_ctrl_t ctrl)
{
    hd44780_writectrls(&ctrl, 1);
}

void hd44780_init(PIO pio, int sm, int pin, float freq)
{
    hd44780_pio = pio;
    hd44780_sm = sm;
    hd44780_dmachannel = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(hd44780_dmachannel);

    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, DREQ_PIO0_TX0);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);

    dma_channel_configure(
        hd44780_dmachannel, 
        &c,
        &pio->txf[sm],
        NULL,
        0, 
        false
    );

    dma_channel_set_irq0_enabled(hd44780_dmachannel, true);

    irq_set_exclusive_handler(DMA_IRQ_0, hd44780_dmahandler);
    irq_set_enabled(DMA_IRQ_0, true);

    uint offset = pio_add_program(pio, &hd44780_4bit_program);
    hd44780_program_init(pio, sm, offset, pin, freq);
}

void hd44780_term()
{
    dma_channel_unclaim(hd44780_dmachannel);
}

void hd44780_cleardisplay()
{
    uint8_t data = 0x01;
        
    hd44780_ctrl_t ctrl = hd44780_makectrlwr(false, data);
    hd44780_writectrl(ctrl);
}

void hd44780_returnhome()
{
    uint8_t data = 0x02;
        
    hd44780_ctrl_t ctrl = hd44780_makectrlwr(false, data);
    hd44780_writectrl(ctrl);
}

void hd44780_entrymodeset(bool id, bool s)
{
    uint8_t data = 0x04 |
    (id ? 0x02 : 0x00) | 
    (s ? 0x01 : 0x00);
        
    hd44780_ctrl_t ctrl = hd44780_makectrlwr(false, data);
    hd44780_writectrl(ctrl);
}

void hd44780_displaycontrol(bool d, bool c, bool b)
{
    uint8_t data = 0x08 | 
        (d ? 0x04 : 0x00) |
        (c ? 0x02 : 0x00) |
        (b ? 0x01 : 0x00);
        
    hd44780_ctrl_t ctrl = hd44780_makectrlwr(false, data);
    hd44780_writectrl(ctrl);
}

void hd44780_cursordisplayshift(bool sc, bool rl)
{
    uint8_t data = 0x10 | 
        (sc ? 0x08 : 0x00) |
        (rl ? 0x04 : 0x00);
        
    hd44780_ctrl_t ctrl = hd44780_makectrlwr(false, data);
    hd44780_writectrl(ctrl);
}

void hd44780_functionset(bool dl, bool n, bool f)
{
    uint8_t data = 0x20 | 
        (dl ? 0x10 : 0x00) |
        (n ? 0x08 : 0x00) |
        (f ? 0x04 : 0x00);
        
    hd44780_ctrl_t ctrl = hd44780_makectrlwr(false, data);
    hd44780_writectrl(ctrl);
}

void hd44780_setcgramaddr(uint8_t addr)
{
    assert(addr < 64);
    uint8_t data = 0x40 | addr;
        
    hd44780_ctrl_t ctrl = hd44780_makectrlwr(false, data);
    hd44780_writectrl(ctrl);
}

void hd44780_setddramaddr(uint8_t addr)
{
    assert(addr < 128);
    uint8_t data = 0x80 | addr;
        
    hd44780_ctrl_t ctrl = hd44780_makectrlwr(false, data);
    hd44780_writectrl(ctrl);
}

void hd44780_writebyte(uint8_t value)
{
    hd44780_writebytes(&value, 1);
}

void hd44780_writebytes(const uint8_t *addr, int len)
{
    for (int i = 0; i < len; i++)   
    {
        hd44780_ctrl_t ctrl = hd44780_makectrlwr(true, addr[i]);
        hd44780_writectrls(&ctrl, 1);
    }
}