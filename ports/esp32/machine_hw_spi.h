

#include "py/runtime.h"
#include "py/stream.h"
#include "py/mphal.h"
#include "extmod/machine_spi.h"
#include "modmachine.h"

#include "driver/spi_master.h"

#define MP_HW_SPI_MAX_XFER_BYTES (4092)
#define MP_HW_SPI_MAX_XFER_BITS (MP_HW_SPI_MAX_XFER_BYTES * 8) // Has to be an even multiple of 8

typedef struct _machine_hw_spi_obj_t {
    mp_obj_base_t base;
    spi_host_device_t host;
    uint32_t baudrate;
    uint8_t polarity;
    uint8_t phase;
    uint8_t bits;
    uint8_t firstbit;
    int8_t sck;
    int8_t mosi;
    int8_t miso;
    spi_device_handle_t spi;
    enum {
        MACHINE_HW_SPI_STATE_NONE,
        MACHINE_HW_SPI_STATE_INIT,
        MACHINE_HW_SPI_STATE_DEINIT
    } state;
} machine_hw_spi_obj_t;

// Static objects mapping to HSPI and VSPI hardware peripherals
STATIC machine_hw_spi_obj_t machine_hw_spi_obj[2];

STATIC void machine_hw_spi_deinit_internal(machine_hw_spi_obj_t *self);

STATIC void machine_hw_spi_init_internal(
    machine_hw_spi_obj_t    *self,
    int8_t                  host,
    int32_t                 baudrate,
    int8_t                  polarity,
    int8_t                  phase,
    int8_t                  bits,
    int8_t                  firstbit,
    int8_t                  sck,
    int8_t                  mosi,
    int8_t                  miso);

STATIC void machine_hw_spi_deinit(mp_obj_base_t *self_in);
STATIC void machine_hw_spi_transfer(mp_obj_base_t *self_in, size_t len, const uint8_t *src, uint8_t *dest);

STATIC void machine_hw_spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind);
STATIC void machine_hw_spi_init(mp_obj_base_t *self_in, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);
mp_obj_t machine_hw_spi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args);
