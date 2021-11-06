TT2Bin
======

This little program converts a tab-separated Truth Table (as you can copy-and-paste from Logisim)
and converts it into a binary file. It is intended to be used to burn the truth table data into
an EEPROM to use as a poor-man's CPLD. 

Using an EEPROM you can get (typically) 16+ inputs and 8 or 16 outputs and every possible combination
of inputs can result in a unique combination of outputs.

Note that this only works for simple gate arrays with no feedback, latches, flipflops, buffers, etc.

Usage:
======

```bash
$ tt2bin [options] input.txt output.bin
```

The options fall into three categories:

* Address mappings (inputs)
* Data mappings (outputs)
* ROM Selection

```bash
--rom=<code>
```

This option selects which kind of ROM chip you are targetting. Currently supported chips:

* 39FS010A
* 39FS020A
* 39FS040

```bash
--a0=col ... --a19=col
```

These options map EEPROM address pins to columns named in the truth table file.

```bash
--d0=col ... -d7=col
```

These options map EEPROM data pins to columns named in the truth table file.
