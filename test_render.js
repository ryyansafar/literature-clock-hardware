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
    let count = 0;
    for(let i=0; i<targetBuffer.length; i++) {
        const c = targetBuffer[i];
        if (c[0] !== 0 || c[1] !== 0 || c[2] !== 0) count++;
    }
    console.log("Lit pixels:", count);
}
setTimeout(test, 500);
`;
code = code.replace("window.onload = init;", "");
code = code.replace("document.getElementById", "(() => ({ getContext: () => ({ setTransform:()=>{}, fillRect:()=>{}, clearRect:()=>{} }), dataset: {}, style: {}, addEventListener: ()=>{} }))");
code = code.replace("window.addEventListener", "(() => {})");
code = code.replace("requestAnimationFrame", "(() => {})");
fs.writeFileSync('temp_test.js', code);
node temp_test.js
