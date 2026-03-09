#![no_std]
#![no_main]

use embedded_hal::i2c::I2c;
use panic_halt as _;
use rp_pico::{
    entry,
    hal::{
        self,
        fugit::RateExtU32,
        gpio::{FunctionI2c, Pin, Pins, PullUp},
        i2c::I2C,
        pac,
        sio::Sio,
        usb, Clock,
    },
};

#[entry]
fn main() -> ! {
    let mut pac = pac::Peripherals::take().unwrap();
    let core = pac::CorePeripherals::take().unwrap();

    let mut watchdog = hal::Watchdog::new(pac.WATCHDOG);

    let clocks = hal::clocks::init_clocks_and_plls(
        rp_pico::XOSC_CRYSTAL_FREQ,
        pac.XOSC,
        pac.CLOCKS,
        pac.PLL_SYS,
        pac.PLL_USB,
        &mut pac.RESETS,
        &mut watchdog,
    )
    .ok()
    .unwrap();

    let usb = usb::UsbBus::new(
        pac.USBCTRL_REGS,
        pac.USBCTRL_DPRAM,
        clocks.usb_clock,
        true,
        &mut pac.RESETS,
    );

    let sio = Sio::new(pac.SIO);
    let pins = Pins::new(
        pac.IO_BANK0,
        pac.PADS_BANK0,
        sio.gpio_bank0,
        &mut pac.RESETS,
    );

    let sdapin: Pin<_, FunctionI2c, PullUp> = pins.gpio4.reconfigure();
    let sclpin: Pin<_, FunctionI2c, PullUp> = pins.gpio5.reconfigure();

    let i2c = I2C::i2c0(
        pac.I2C0,
        sdapin,
        sclpin,
        400.kHz(),
        &mut pac.RESETS,
        &clocks.system_clock,
    );

    let delay = cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz());

    let mut lc = LightController {
        i2c: i2c,
        address: LED_DRIVER_BUS_ADDRESS,
        d: delay,
    };

    lc.initialize();

    loop {
        for x in 0..16 {
            lc.set_key_rgb(x, 0xFF, 0xFF, 0xFF);
            lc.sleep(100);
        }
    }
}

/*
struct Screen<I2C> {
    i2c: I2C,
    address: u8,
}

impl<I2C> screen<I2C>
where
    I2C: I2c,
{
    fn initialize(&mut self) {}
}
*/

struct LightController<I2C> {
    i2c: I2C,
    address: u8,
    d: cortex_m::delay::Delay,
}

impl<I2C> LightController<I2C>
where
    I2C: I2c,
{
    fn initialize(&mut self) {
        self.write_register(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x00);
        self.d.delay_ms(100);
        self.write_register(
            ISSI_BANK_FUNCTIONREG,
            ISSI_REG_CONFIG,
            ISSI_REG_CONFIG_PICTUREMODE,
        );
        self.write_register(ISSI_BANK_FUNCTIONREG, ISSI_REG_PICTUREFRAME, 0x00);
        self.write_register(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x01);

        for frame in 0..8 {
            for col in 0..0x11 {
                self.write_register(frame, col, 0xFF);
            }
        }
    }

    fn select_bank(&mut self, bank: u8) {
        self.i2c
            .write(self.address, &[ISSI_COMMANDREGISTER, bank])
            .unwrap();
    }

    fn sleep(&mut self, ms: u32) {
        self.d.delay_ms(ms);
    }

    fn write_register(&mut self, bank: u8, register: u8, data: u8) {
        self.select_bank(bank);
        self.i2c.write(self.address, &[register, data]).unwrap();
    }

    fn set_pixel(&mut self, num: u8, pwm: u8) {
        self.write_register(0, 0x24 + num, pwm);
    }

    fn set_key_rgb(&mut self, pixel_index: usize, r: u8, g: u8, b: u8) {
        self.set_pixel(LIGHTLOOKUP[pixel_index][0], r);
        self.set_pixel(LIGHTLOOKUP[pixel_index][1], g);
        self.set_pixel(LIGHTLOOKUP[pixel_index][2], b);
    }
}

#[derive(Clone, Copy, Debug)]
pub enum Error<I2cError> {
    I2cError(I2cError),
    InvalidLocation(u8),
    InvalidFrame(u8),
}

impl<E> From<E> for Error<E> {
    fn from(error: E) -> Self {
        Error::I2cError(error)
    }
}

pub const LED_DRIVER_BUS_ADDRESS: u8 = 0x74;

pub const ISSI_REG_CONFIG: u8 = 0x00;
pub const ISSI_REG_CONFIG_PICTUREMODE: u8 = 0x00;
pub const ISSI_REG_CONFIG_AUTOPLAYMODE: u8 = 0x08;
pub const ISSI_REG_CONFIG_AUDIOPLAYMODE: u8 = 0x18;

pub const ISSI_CONF_PICTUREMOD: u8 = 0x00;
pub const ISSI_CONF_AUTOFRAMEMOD: u8 = 0x04;
pub const ISSI_CONF_AUDIOMOD: u8 = 0x08;

pub const ISSI_REG_PICTUREFRAME: u8 = 0x01;

pub const ISSI_REG_SHUTDOWN: u8 = 0x0A;
pub const ISSI_REG_AUDIOSYNC: u8 = 0x06;

pub const ISSI_COMMANDREGISTER: u8 = 0xFD;
pub const ISSI_BANK_FUNCTIONREG: u8 = 0x0B;

const LIGHTLOOKUP: [[u8; 3]; 16] = [
    [120, 88, 104], // 0, 0
    [136, 40, 72],  // 1, 0
    [112, 80, 96],  // 2, 0
    [128, 32, 64],  // 3, 0
    [121, 89, 105], // 0, 1
    [137, 41, 73],  // 1, 1
    [113, 81, 97],  // 2, 1
    [129, 33, 65],  // 3, 1
    [122, 90, 106], // 0, 2
    [138, 25, 74],  // 1, 2
    [114, 82, 98],  // 2, 2
    [130, 17, 66],  // 3, 2
    [123, 91, 107], // 0, 3
    [139, 26, 75],  // 1, 3
    [115, 83, 99],  // 2, 3
    [131, 18, 67],  // 3, 3
];
