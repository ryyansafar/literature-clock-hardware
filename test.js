const fs = require('fs');
let code = fs.readFileSync('script.js', 'utf8');
code += `
function test() {
    clearBuffer(targetBuffer);
    const entry = {
        quote: "They were hurrying west, trying to reach the river before sunset. The warming-related adjustments to Earth's orbit had shortened the winter days, so that now, in January, sunset was taking place at 4.23.",
        timeText: "4.23",
        author: "Jennifer Egan",
        title: "A Visit from the Goon Squad"
    };
    buildTargetAndMaps(entry);
    
    // Check top 3 lines (quotes area)
    let litQuotes = 0;
    for (let y = 0; y < 24; y++) {
        for (let x = 0; x < GRID_W; x++) {
            const idx = y * GRID_W + x;
            const c = targetBuffer[idx];
            if (c[0] !== 0 || c[1] !== 0 || c[2] !== 0) litQuotes++;
        }
    }
    console.log("Lit pixels in Quote Area (top 24 rows):", litQuotes);
    
    // Check rest
    let litRest = 0;
    for (let y = 24; y < GRID_H; y++) {
        for (let x = 0; x < GRID_W; x++) {
            const idx = y * GRID_W + x;
            const c = targetBuffer[idx];
            if (c[0] !== 0 || c[1] !== 0 || c[2] !== 0) litRest++;
        }
    }
    console.log("Lit pixels in Rest Area (bottom 24 rows):", litRest);
}
test();
`;

// Mocking browser globals
code = code.replace("window.onload = init;", "");
code = code.replace("const canvas = document.getElementById('ledMatrix');", "const canvas = { dataset: {}, style: {}, getContext: () => ({ setTransform:()=>{}, fillRect:()=>{} }) };");
code = code.replace("const ctx = canvas.getContext('2d');", "");
code = code.replace(/document\.getElementById.*?;/g, "null;");
code = code.replace(/window\.addEventListener.*?;/g, "");
code = code.replace(/document\.addEventListener.*?;/g, "");
code = code.replace(/requestAnimationFrame/g, "(() => {})");
code = code.replace(/cancelAnimationFrame/g, "(() => {})");
code = code.replace(/performance\.now/g, "(() => 0)");
code = code.replace("fetch(", "(() => Promise.resolve({ text: () => '' }))(");
code = code.replace("new Image()", "{}");

fs.writeFileSync('temp_test.js', code);
require('./temp_test.js');
