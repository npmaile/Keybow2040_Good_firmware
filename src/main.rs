#![no_std]
#![no_main]

use core::convert::Infallible;

use panic_halt as _;
use rp_pico::hal::prelude::*;
use rp_pico::{
    entry,
    hal::{
        clocks::{self},
        fugit::RateExtU32,
        gpio::{FunctionI2c, Pin, Pins, PullUp},
        i2c::I2C,
        pac,
        sio::Sio,
        usb, Watchdog,
    },
};

use embedded_hal::digital::InputPin;
use embedded_hal::i2c::I2c;

pub trait InPin {
    fn is_low() -> Result<bool, Infallible>;
}

#[entry]
fn main() -> ! {
    let mut pac = pac::Peripherals::take().unwrap();

    let core = pac::CorePeripherals::take().unwrap();

    let mut watchdog = Watchdog::new(pac.WATCHDOG);

    let clocks = clocks::init_clocks_and_plls(
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

    let mut delay = cortex_m::delay::Delay::new(core.SYST, clocks.system_clock.freq().to_Hz());

    let _usb = usb::UsbBus::new(
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

    let sda_pin: Pin<_, FunctionI2c, PullUp> = pins.gpio4.reconfigure();
    let scl_pin: Pin<_, FunctionI2c, PullUp> = pins.gpio5.reconfigure();

    let i2c = I2C::i2c0(
        pac.I2C0,
        sda_pin,
        scl_pin,
        400.kHz(),
        &mut pac.RESETS,
        &clocks.system_clock,
    );

    let lc = LightControllerInstance::new(i2c);
    let mut board = Board {
        keys: [
            Key {
                input_pin: pins.gpio18.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 120,
                    g: 88,
                    b: 104,
                },
            },
            Key {
                input_pin: pins.gpio14.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 136,
                    g: 40,
                    b: 72,
                },
            },
            Key {
                input_pin: pins.gpio10.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 112,
                    g: 80,
                    b: 96,
                },
            },
            Key {
                input_pin: pins.gpio6.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 128,
                    g: 32,
                    b: 64,
                },
            },
            Key {
                input_pin: pins.gpio19.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 121,
                    g: 89,
                    b: 105,
                },
            },
            Key {
                input_pin: pins.gpio15.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 137,
                    g: 41,
                    b: 73,
                },
            },
            Key {
                input_pin: pins.gpio11.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 113,
                    g: 81,
                    b: 97,
                },
            },
            Key {
                input_pin: pins.gpio7.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 129,
                    g: 33,
                    b: 65,
                },
            },
            Key {
                input_pin: pins.gpio20.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 122,
                    g: 90,
                    b: 106,
                },
            },
            Key {
                input_pin: pins.gpio16.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 138,
                    g: 25,
                    b: 74,
                },
            },
            Key {
                input_pin: pins.gpio12.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 114,
                    g: 82,
                    b: 98,
                },
            },
            Key {
                input_pin: pins.gpio8.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 130,
                    g: 17,
                    b: 66,
                },
            },
            Key {
                input_pin: pins.gpio21.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 123,
                    g: 91,
                    b: 107,
                },
            },
            Key {
                input_pin: pins.gpio17.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 139,
                    g: 26,
                    b: 75,
                },
            },
            Key {
                input_pin: pins.gpio13.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 115,
                    g: 83,
                    b: 99,
                },
            },
            Key {
                input_pin: pins.gpio9.into_pull_up_input().into_dyn_pin(),
                light: RGB {
                    r: 131,
                    g: 18,
                    b: 67,
                },
            },
        ],
        light_controller: lc,
    };

    loop {
        board.turn_on_with_button_press();
    }
}

impl<INPIN, LC> Board<INPIN, LC>
where
    INPIN: InputPin,
    LC: LightControllerTrait,
{
    fn turn_on_with_button_press(&mut self) {
        let _ = self
            .keys
            .iter_mut()
            .map(|k| {
                if k.input_pin.is_low().expect("balls") {
                    self.light_controller.set_pixel(k.light.r, 255);
                    self.light_controller.set_pixel(k.light.g, 255);
                    self.light_controller.set_pixel(k.light.b, 255);
                } else {
                    self.light_controller.set_pixel(k.light.r, 0);
                    self.light_controller.set_pixel(k.light.g, 0);
                    self.light_controller.set_pixel(k.light.b, 0);
                }
            })
            .count();
        self.light_controller.commit();
    }
    fn all_red(&mut self) {
        let _ = self
            .keys
            .iter_mut()
            .map(|k| {
                self.light_controller.set_pixel(k.light.r, 0);
                self.light_controller.set_pixel(k.light.g, 0);
                self.light_controller.set_pixel(k.light.b, 0);
            })
            .count();
        self.light_controller.commit();
    }
}

struct Board<INPIN: InputPin, LC: LightControllerTrait> {
    keys: [Key<INPIN>; 16],
    light_controller: LC,
}

struct Key<INPIN: InputPin> {
    input_pin: INPIN,
    light: RGB,
}

struct RGB {
    r: u8,
    g: u8,
    b: u8,
}

trait LightControllerTrait {
    fn set_pixel(&mut self, num: u8, pwm: u8);
    fn commit(&mut self);
}

struct LightControllerInstance<I2C> {
    i2c: I2C,
    address: u8,
    frame1: bool,
}

impl<I2C: I2c> LightControllerTrait for LightControllerInstance<I2C> {
    fn set_pixel(&mut self, num: u8, pwm: u8) {
        if self.frame1 {
            self.write_register(1, 0x24 + num, pwm);
        } else {
            self.write_register(0, 0x24 + num, pwm);
        }
    }

    fn commit(&mut self) {
        self.frame1 = !self.frame1;
        if self.frame1 {
            self.write_register(ISSI_BANK_FUNCTIONREG, ISSI_REG_PICTUREFRAME, 0x01);
        } else {
            self.write_register(ISSI_BANK_FUNCTIONREG, ISSI_REG_PICTUREFRAME, 0x00);
        }
    }
}

impl<I2C: I2c> LightControllerInstance<I2C> {
    fn new(i2c: I2C) -> LightControllerInstance<I2C> {
        let mut x = LightControllerInstance {
            i2c: i2c,
            address: LED_DRIVER_BUS_ADDRESS,
            frame1: false,
        };
        x.initialize();
        return x;
    }

    fn initialize(&mut self) {
        self.write_register(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x00);
        self.write_register(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x01);
        self.write_register(
            ISSI_BANK_FUNCTIONREG,
            ISSI_REG_CONFIG,
            ISSI_REG_CONFIG_PICTUREMODE,
        );
        self.write_register(ISSI_BANK_FUNCTIONREG, ISSI_REG_PICTUREFRAME, 0x00);

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

    fn write_register(&mut self, bank: u8, register: u8, data: u8) {
        self.select_bank(bank);
        self.i2c.write(self.address, &[register, data]).unwrap();
    }
}

pub trait I2CWriter {
    fn write();
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
