
// tetris_asm is auto generated
const tetris_codearray = new Uint8Array(tetris_asm.length);
for (let i in tetris_asm) tetris_codearray[i] = tetris_asm.charCodeAt(i);

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
const NAV_DEL = 100;

let w;
let h;
let data;
let data_len;
let score, rows, shapes, gameover, drawf, msg, now;

let btn_left_clicked  = false;
let btn_right_clicked = false;

async function delay(time) {
    return new Promise((resolve) => setTimeout(resolve, time));
}

async function run_game() {
    const wasm = await WebAssembly.instantiate(tetris_codearray);
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
        if (btn_left_clicked) {
            data[msg] = LEFT;
        } else if (btn_right_clicked) {
            data[msg] = RIGHT;
        }

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
                o.innerHTML = "<br>GAME OVER!<br><br>" +
                    "Shapes: " + data[shapes] + "<br>" +
                    "Rows: "   + data[rows]   + "<br>" +
                    "Score: "  + data[score]  + "<br>";
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
    if ("serviceWorker" in navigator) {
        try {
            await navigator.serviceWorker.register("sw.js");
            console.log("Worker registered");
        } catch (e) {
            console.log("Worker was not registered");
        }
    }

    window.onbeforeunload = () => "Are you sure you want to leave?";

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

    const btn_left  = document.getElementById("btn-left");
    const btn_right = document.getElementById("btn-right");

    btn_left.onmousedown   = () => data[msg] = LEFT;
    btn_right.onmousedown  = () => data[msg] = RIGHT;

    let touch_timer;
    btn_left.ontouchstart  = () => touch_timer = setTimeout(() => btn_left_clicked  = true, NAV_DEL);
    btn_right.ontouchstart = () => touch_timer = setTimeout(() => btn_right_clicked = true, NAV_DEL);
    btn_left.ontouchend    = () => { btn_left_clicked  = false; clearTimeout(touch_timer); };
    btn_right.ontouchend   = () => { btn_right_clicked = false; clearTimeout(touch_timer); };

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
