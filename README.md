# GMSL_TOOL

**GMSL_TOOL** is an open-source utility for controlling GMSL (Gigabit Multimedia Serial Link) serializers and deserializers (SerDes) from user space via I2C.  
It is designed for platforms using GMSL2/3-compatible devices and is particularly suited for embedded Linux systems (such as aarch64-based devices).

---

## Features

- Control and configure various GMSL serializers and deserializers
- Set MIPI CSI-2 lane configurations and mappings
- Collect link statistics
- Run with or without initialization
- Auto-scan I2C slave addresses or set manually

---

## Usage

```bash
./gmsl_tool [options]
```

Options:
```text
  -i, --i2c <busnumber>   Specify the i2c bus number [default 4]
  -m, --maprx <hex>       Specify MIPI RX pins mapping for serializer
  -p, --polarityrx <hex>  Specify MIPI RX pins polarity for serializer
  -l, --lanesrx <val>     Specify MIPI RX lanes for serializer [default 4]
  -k, --maptx <hex>       Specify MIPI TX pins mapping for deserializer
  -o, --polaritytx <hex>  Specify MIPI TX pins polarity for deserializer
  -r, --lanestx <val>     Specify MIPI TX lanes for serializer [default 4]
  -t, --ratetx <val>      Specify MIPI TX transfer rate
  -s, --stats             Statistics only
  -n, --noinit            Without init
  -a, --ssa <address>     Specify serializer I2C slave address [default - autoscan]
  -b, --dsa <address>     Specify deserializer I2C slave address [default - autoscan]
  -h, --help              Show this help message and exit
```

## Compilation

```bash
CROSS_COMPILE=<path_to_compiler> make
```

Example:
```bash
CROSS_COMPILE=aarch64-none-linux-gnu- make
```

To clean:
```bash
make clean
```