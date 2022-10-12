meta:
  id: siglent_bin
  endian: le
enums:
  unit:
    0: v
    1: a
    2: vv
    3: aa
    4: ou
    5: w
    6: sqrt_v
    7: sqrt_a
    8: integral_v
    9: integral_a
    10: dt_v
    11: dt_a
    12: dt_div
    13: hz
    14: s
    15: sa
    16: pts
    17: nul
    18: db
    19: dbv
    20: dba
    21: vpp
    22: vdc
    23: dbm
  magnitude:
    0: yocto
    1: zepto
    2: atto
    3: femto
    4: pico
    5: nano
    6: micro
    7: milli
    8: iu
    9: kilo
    10: mega
    11: giga
    12: tera
    13: peta
types:
  unit_value:
    seq:
      - id: value
        type: f8
      - id: magnitude
        type: u4
        enum: magnitude
      - id: unit
        type: u4
        enum: unit
seq:
  - id: chx_on
    type: u4
    repeat: expr
    repeat-expr: 4
  - id: analog_scales
    type: unit_value
    repeat: expr
    repeat-expr: 4
  - id: analog_offsets
    type: unit_value
    repeat: expr
    repeat-expr: 4
  - id: digital_on
    type: u4
  - id: digital_ch_on
    type: u4
    repeat: expr
    repeat-expr: 16
  - id: time_div
    type: unit_value
  - id: time_delay
    type: unit_value
  - id: analog_size
    type: u4
  - id: analog_sample_rate
    type: unit_value
  - id: digital_size
    type: u4
  - id: digital_sample_rate
    type: unit_value