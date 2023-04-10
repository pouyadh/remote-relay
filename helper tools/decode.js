const fs = require("fs");
const color = (code, string) => `\x1b[${code}m${string}\x1b[0m`;

const codeType = {
  EV1527: "EV1527",
  HT6P20: "HT6P20",
};

fs.readdirSync("./").forEach((item) => {
  if (!item.includes(".")) {
    console.log(`---> ${item}`);
    aynalize("./" + item + "/digital.csv");
  }
});

function aynalize(file) {
  const content = fs.readFileSync(file, {
    encoding: "ascii",
  });
  const data = content
    .split("\n")
    .map((item) => item.replace("\r", ""))
    .map((item) => item.split(",").map((item) => +item));

  data.shift();
  if (data[0][1] == 0) data.shift();

  const data2 = data.map((item, index) => {
    if (index == 0) return item;
    return [
      +((data[index][0] - data[index - 1][0]) * 10 ** 6).toFixed(),
      item[1],
    ];
  });

  const codes = [];

  // for (let i = 1; i < data.length - 2; i += 2) {
  //   const TH = +((data[i + 1][0] - data[i][0]) * 1000000).toFixed();
  //   const TL = +((data[i + 2][0] - data[i + 1][0]) * 1000000).toFixed();
  //   const TD = +((data[i + 2][0] - data[i][0]) * 1000000).toFixed();
  //   cycleHandler([TH, TL, TD]);
  // }

  let th = 0;
  let tl = 0;
  let thl = 0;
  let edge = 0;

  const ev1527 = {
    td_min: 900,
    td_max: 3200,
    tp_min: 900 * 8,
    tp_max: 3200 * 8,
    preamble: 0,
    bitCnt: 0,
    code: 0,
    codeBit: "",
    thlr: 0,
    OCp: 0,
    tds: [],
  };
  function ev1527Pipe() {
    if (edge == 1) {
      if (ev1527.preamble == 0) {
        if (thl > ev1527.tp_min && thl < ev1527.tp_max) {
          ev1527.thlr = (thl / th).toFixed();
          ev1527.preamble = 1;
          ev1527.bitCnt = 0;
          ev1527.code = 0;
          ev1527.codeBit = "";
          ev1527.OCp = (thl / 32).toFixed();
          ev1527.tds = [];
        }
      } else if (ev1527.preamble == 1) {
        if (thl > ev1527.td_min && thl < ev1527.td_max) {
          ev1527.tds.push((thl / ev1527.OCp).toFixed());
          ev1527.code *= 2;
          if (th > tl) ev1527.code++;
          ev1527.codeBit += th > tl ? "1" : "0";
          if (++ev1527.bitCnt == 24) {
            ev1527.preamble = 2;
          }
        } else {
          ev1527.preamble = 0;
        }
      } else {
        if (thl > ev1527.tp_min && thl < ev1527.tp_max) {
          ev1527.preamble = 1;
          ev1527.bitCnt = 0;
          if (!codes.includes(ev1527.code)) {
            codes.push(ev1527.code);
            console.log(
              `${color(33, "EV1527")}-> ${color(
                36,
                ev1527.code.toString(16).toUpperCase().padStart(6, "0")
              )}`,
              `${ev1527.codeBit.slice(0, 20)} ${ev1527.codeBit.slice(20, 24)}`,
              ev1527.thlr,
              ev1527.OCp,
              ev1527.tds
            );
          }

          ev1527.code = 0;
          ev1527.codeBit = "";
        } else {
          ev1527.preamble = 0;
        }
      }
    }
  }
  const ht6p20 = {
    td_min: 900,
    td_max: 3200,
    tp_min: 900 * 8,
    tp_max: 3200 * 8,
    preamble: 0,
    bitCnt: 0,
    code: 0,
    codeBit: "",
    thlr: 0,
    OCp: 0,
    tds: [],
  };
  function ht6p20Pipe() {
    if (edge == 0) {
      if (ht6p20.preamble == 0) {
        if (thl > ht6p20.tp_min && thl < ht6p20.tp_max) {
          ht6p20.thlr = (thl / th).toFixed();
          ht6p20.preamble = 1;
          ht6p20.bitCnt = 0;
          ht6p20.code = 0;
          ht6p20.codeBit = "";
          ht6p20.OCp = (thl / 24).toFixed();
          ht6p20.tds = [];
        }
      } else if (ht6p20.preamble == 1) {
        if (thl > ht6p20.td_min && thl < ht6p20.td_max) {
          ht6p20.tds.push((thl / ht6p20.OCp).toFixed());
          ht6p20.code *= 2;
          if (tl > th) ht6p20.code++;
          ht6p20.codeBit += tl > th ? "1" : "0";
          if (++ht6p20.bitCnt == 28) {
            ht6p20.preamble = 2;
            ht6p20.code >>= 4;
          }
        } else {
          ht6p20.preamble = 0;
        }
      } else {
        if (thl > ht6p20.tp_min && thl < ht6p20.tp_max) {
          ht6p20.preamble = 1;
          ht6p20.bitCnt = 0;
          if (!codes.includes(ht6p20.code)) {
            codes.push(ht6p20.code);
            let tmp =
              ht6p20.codeBit.slice(0, 20) +
              " " +
              ht6p20.codeBit.slice(20, 24) +
              " " +
              ht6p20.codeBit.slice(24, 28);
            console.log(
              `${color(35, "HT6P20")}-> ${color(
                36,
                ht6p20.code.toString(16).toUpperCase().padStart(6, "0")
              )}`,
              tmp,
              ht6p20.thlr,
              ht6p20.OCp,
              ht6p20.tds
            );
          }

          ht6p20.code = 0;
          ht6p20.codeBit = "";
        } else {
          ht6p20.preamble = 0;
        }
      }
    }
  }
  data2.forEach((item) => capture(item));
  function capture([_dur, _edge]) {
    if (_edge) tl = _dur;
    else th = _dur;
    edge = _edge;
    thl = th + tl;
    if (thl > 65535) console.log("overflow");
    ev1527Pipe();
    ht6p20Pipe();
  }
}
