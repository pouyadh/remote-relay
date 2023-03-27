const _xtal = 16000000;
const c = {
  td_min_us: 400,
  td_max_us: 4000,
  tp2ph_rate_min: 20,
  tp2ph_rate_max: 40,
  get tp_max_s() {return (this.td_max_us * 8) / 10 ** 6;},
  get tim1_prescale_min() {return (this.tp_max_s * 2 * _xtal) / 65536;},
  get tim1_prescale_reg_val() {return Math.floor(Math.log(this.tim1_prescale_min) / Math.log(2) + 1);},
  get tim1_prescale() {return 2 ** Math.floor(Math.log(this.tim1_prescale_min) / Math.log(2) + 1);},
  get tim1_frequency() {return _xtal / this.tim1_prescale;},
  get tim1_cycles_per_us() {return this.tim1_frequency / 10 ** 6;},
  get tim1_cycles_per_ms() {return this.tim1_frequency / 10 ** 3;},
  get tunit_min() {return (this.td_min_us * this.tim1_cycles_per_us) / 4;},
  get tunit_max() {return (this.td_max_us * this.tim1_cycles_per_us) / 4;},
  get td_min() {return this.tunit_min * 4;},
  get td_max() {return this.tunit_max * 4;},
  get t0h_min() {return this.tunit_min * 1;},
  get t0h_max() {return this.tunit_max * 1;},
  get t0l_min() {return this.tunit_min * 3;},
  get t0l_max() {return this.tunit_max * 3;},
  get t1h_min() {return this.tunit_min * 3;},
  get t1h_max() {return this.tunit_max * 3;},
  get t1l_min() {return this.tunit_min * 1;},
  get t1l_max() {return this.tunit_max * 1;},
  get tp_min() {return this.tunit_min * 32;},
  get tp_max() {return this.tunit_max * 32;},
  get tph_min() {return this.tunit_min * 1;},
  get tph_max() {return this.tunit_max * 1;},
  get tpl_min() {return this.tunit_min * 31;},
  get tpl_max() {return this.tunit_max * 31;},
};

const defines = Object.keys(c).map(
  (k) => `#define REMOTE_${k.toLocaleUpperCase()} ${c[k]}`
);

const lines = [
  "#ifndef __REMOTE_CONSTS_H",
  "#define __REMOTE_CONSTS_H",
  "",
  ...defines,
  "",
  "#endif",
];

const generateFile = () => {
  const fs = require("fs");
  fs.writeFile("./remote-consts.h", lines.join("\n"), (err) => {
    if (err) {
      console.log(err);
    } else {
      console.log("Generated!");
    }
  });
};

generateFile();
//console.log(c);
