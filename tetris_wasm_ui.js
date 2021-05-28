
// check tetris.h
const NONE    = 0;
const QUIT    = 1;
const REDRAW  = 2;
const PAUSE   = 3;
const BOTTOM  = 4;
const DOWN    = 5;
const RIGHT   = 6;
const LEFT    = 7;
const ROTATE1 = 8;
const ROTATE2 = 9;

const W       = 10;
const H       = 20;
const T       = 4;
const DELAY   = 10;

let w;
let h;
let data;
let data_len;
let score, rows, shapes, gameover, drawf, msg, now;

async function delay(time) {
    return new Promise((resolve) => setTimeout(resolve, time));
}

async function run_game() {
    const response = await fetch("tetris_wasm.wasm");
    const file = await response.arrayBuffer();
    const wasm = await WebAssembly.instantiate(file);
    let { memory, setup, loop, get_data_len } = wasm.instance.exports;

    w = W;
    h = H;
    data_len = get_data_len(w, h);
    data = new Int32Array(memory.buffer, 0, data_len);

    score    = data_len - 7;
    shapes   = data_len - 6;
    rows     = data_len - 5;
    gameover = data_len - 4;
    drawf    = data_len - 3;
    msg      = data_len - 2;
    now      = data_len - 1;

    let _setup = setup;
    let _loop = loop;
    setup = () => { data[now] = Date.now(); _setup(w, h, data); };
    loop  = () => { data[now] = Date.now(); _loop(); };

    data[msg] = NONE;
    setup();

    for (;;) {
        loop();
        if (data[msg] != NONE) {
            data[msg] = NONE;
        }

        if (data[drawf]) { // draw flag

            for (let j = 0; j < h; ++j) {
                for (let i = 0; i < w; ++i) {
                    set_cell(i, j, data[w * j + i]);
                }
            }

            // TODO: why j=0 is messed up?
            for (let j = 1; j < T; ++j) {
                for (let i = 0; i < T; ++i) {
                    set_cell(i, j, data[(w * h) + T * j + i], true);
                }
            }

            document.getElementById("score").innerText  = data[score];
            document.getElementById("rows").innerText   = data[rows];
            document.getElementById("shapes").innerText = data[shapes];

            if (data[gameover]) {
                let c = document.getElementById("content");
                let o = document.getElementById("game-over");
                c.style.visibility = "collapse";
                o.innerHTML = "GAME OVER!<br>";
                o.innerHTML += "Shapes: " + data[shapes] + "<br>";
                o.innerHTML += "Rows: "   + data[rows]   + "<br>";
                o.innerHTML += "Score: "  + data[score]  + "<br>";
                await delay(3000);
                o.innerHTML = "";
                c.style.visibility = "visible";
                break;
            } else {
                await delay(DELAY);
            }

            data[drawf] = 0;
        } else {
            await delay(DELAY);
        }
    }
}

function set_cell(i, j, val, next) {
    next = next ? "-next" : "";
    document.getElementById(`cell${next}-${i},${j}`).style.background =
        val == 0 ? "black" : "white";
}

async function main() {
    document.addEventListener("keydown", event => {
        switch (event.key) {
        case "ArrowRight": data[msg] = RIGHT;   break;
        case "ArrowLeft":  data[msg] = LEFT;    break;
        case "ArrowUp":    data[msg] = ROTATE1; break;
        case "ArrowDown":  data[msg] = DOWN;    break;
        case " ":          data[msg] = BOTTOM;  break;
        case "p":          data[msg] = PAUSE;   break;
        }
    });

    let table, tr, td;

    table = document.getElementById("board");
    for (let j = 0; j < H; ++j) {
        tr = document.createElement("tr");
        table.appendChild(tr);
        for (let i = 0; i < W; ++i) {
            td = document.createElement("td");
            td.id = `cell-${i},${j}`;
            tr.appendChild(td);
        }
    }

    table = document.getElementById("board-next");
    for (let j = 0; j < T; ++j) {
        tr = document.createElement("tr");
        table.appendChild(tr);
        for (let i = 0; i < T; ++i) {
            td = document.createElement("td");
            td.id = `cell-next-${i},${j}`;
            tr.appendChild(td);
        }
    }

    for (;;) await run_game();
}

main();
