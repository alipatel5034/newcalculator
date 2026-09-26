let ModuleReady = false;
let ans = 0;
let currentMode = "comp";

const $ = id => document.getElementById(id);
const out = $("outputText");
const result = $("result");
const expression = $("expression");

function setResult(text) {
  result.textContent = String(text);
  out.textContent = String(text);
}
function num(id) { return Number($(id).value || 0); }
function callC(name, returnType, argTypes, args) {
  return Module.ccall(name, returnType, argTypes, args);
}

window.Module = window.Module || {};
const oldInit = Module.onRuntimeInitialized;
Module.onRuntimeInitialized = function() {
  ModuleReady = true;
  out.textContent = "C/WebAssembly engine ready.";
  if (oldInit) oldInit();
  renderEquationInputs();
  renderMatrixInputs();
  renderVectorInputs();
};

function requireWasm() {
  if (!ModuleReady) { setResult("Please wait: WebAssembly is still loading."); return false; }
  return true;
}

document.querySelectorAll(".mode").forEach(btn => {
  btn.addEventListener("click", () => {
    currentMode = btn.dataset.mode;
    document.querySelectorAll(".mode").forEach(b => b.classList.remove("active"));
    btn.classList.add("active");
    $("modeName").textContent = btn.textContent;
    document.querySelectorAll(".panel").forEach(p => p.classList.remove("active-panel"));
    $("panel-" + currentMode).classList.add("active-panel");
    expression.textContent = currentMode.toUpperCase();
  });
});

$("themeToggle").onclick = () => {
  document.body.classList.toggle("dark");
  $("themeToggle").textContent = document.body.classList.contains("dark") ? "☀" : "☾";
};

$("clearOutput").onclick = () => { out.textContent = ""; result.textContent = "0"; expression.textContent = "Ready"; };

document.querySelectorAll("[data-value]").forEach(btn => {
  btn.onclick = () => {
    if (currentMode !== "comp") return;
    $("compInput").value += btn.dataset.value;
    expression.textContent = $("compInput").value;
  };
});

document.querySelectorAll("[data-action]").forEach(btn => {
  btn.onclick = () => {
    if (!requireWasm()) return;
    const a = btn.dataset.action;
    const input = $("compInput");
    if (currentMode !== "comp" && !["clear","backspace"].includes(a)) return;
    if (a === "clear") { input.value = ""; expression.textContent = "Ready"; setResult(0); }
    if (a === "backspace") { input.value = input.value.slice(0,-1); expression.textContent = input.value || "Ready"; }
    if (a === "open") input.value += "(";
    if (a === "close") input.value += ")";
    if (a === "pi") input.value += Math.PI;
    const unary = {sin:1,cos:2,tan:3,asin:4,acos:5,atan:6,sqrt:7,log:8,ln:9,inv:10,square:11};
    if (unary[a]) {
      const x = Number(input.value);
      if (!Number.isFinite(x)) return setResult("Enter one number for this function.");
      expression.textContent = `${a}(${x})`;
      const r = callC("web_unary","string",["number","number"],[x,unary[a]]);
      ans = Number(r) || ans; setResult(r);
    }
    if (a === "ans") input.value += ans;
    if (a === "equals") calculateComp();
    expression.textContent = input.value || expression.textContent;
  };
});

function calculateComp() {
  if (!requireWasm()) return;
  const s = $("compInput").value.trim();
  const m = s.match(/^\s*(-?(?:\d+(?:\.\d*)?|\.\d+))\s*([+\-*/])\s*(-?(?:\d+(?:\.\d*)?|\.\d+))\s*$/);
  if (!m) return setResult("For COMP, enter a simple expression such as 25+5, 25*2, 25/5 or 2^3 using xʸ.");
  const a=Number(m[1]), b=Number(m[3]), op={"+":1,"-":2,"*":3,"/":4}[m[2]];
  const r=callC("web_basic","string",["number","number","number"],[a,op,b]);
  ans=Number(r)||ans; setResult(r);
}
$("compCalc").onclick=calculateComp;

$("complexCalc").onclick=()=>{
  if(!requireWasm())return;
  const op={add:1,sub:2,mul:3,div:4,magarg:5}[$("complexOp").value];
  const r=callC("web_complex","string",
    ["number","number","number","number","number"],
    [num("c1r"),num("c1i"),num("c2r"),num("c2i"),op]);
  setResult(r);
};

function renderEquationInputs(){
  const type=$("eqType").value, box=$("eqInputs");
  if(type==="2") box.innerHTML=`<div class="grid2">
    ${["a1","b1","c1","a2","b2","c2"].map(x=>`<label>${x}<input id="${x}" type="number" step="any"></label>`).join("")}</div>`;
  else if(type==="3") box.innerHTML=`<div class="grid2">
    ${["a1","b1","c1","d1","a2","b2","c2","d2","a3","b3","c3","d3"].map(x=>`<label>${x}<input id="${x}" type="number" step="any"></label>`).join("")}</div>`;
  else if(type==="quadratic") box.innerHTML=`<div class="grid2">${["a","b","c"].map(x=>`<label>${x}<input id="q${x}" type="number" step="any"></label>`).join("")}</div>`;
  else box.innerHTML=`<div class="grid2">${["a","b","c","d"].map(x=>`<label>${x}<input id="cu${x}" type="number" step="any"></label>`).join("")}</div>`;
}
$("eqType").onchange=renderEquationInputs;
$("equationCalc").onclick=()=>{
  if(!requireWasm())return;
  const t=$("eqType").value;
  let r;
  if(t==="2") r=callC("web_linear2","string",Array(6).fill("number"),["a1","b1","c1","a2","b2","c2"].map(num));
  else if(t==="3") r=callC("web_linear3","string",Array(12).fill("number"),["a1","b1","c1","d1","a2","b2","c2","d2","a3","b3","c3","d3"].map(num));
  else if(t==="quadratic") r=callC("web_quadratic","string",["number","number","number"],[Number(document.getElementById("qa").value||0),Number(document.getElementById("qb").value||0),Number(document.getElementById("qc").value||0)]);
  else r=callC("web_cubic","string",["number","number","number","number"],[Number(document.getElementById("cua").value||0),Number(document.getElementById("cub").value||0),Number(document.getElementById("cuc").value||0),Number(document.getElementById("cud").value||0)]);
  setResult(r);
};

$("baseCalc").onclick=()=>{
  if(!requireWasm())return;
  const r=callC("web_base","string",["number","number"],[Number($("baseNumber").value||0),Number($("baseType").value)]);
  setResult(r);
};

function matrixId(prefix,i){return `${prefix}${i}`}
function renderMatrixInputs(){
  const n=Number($("matrixSize").value), box=$("matrixInputs"), op=$("matrixOp").value;
  const cells=n*n;
  box.innerHTML=`<div class="matrix ${n===2?"r2":"r3"}">
    ${Array.from({length:cells},(_,i)=>`<input id="${matrixId("A",i)}" type="number" step="any">`).join("")}
  </div>`+
  ((op==="add"||op==="sub"||op==="mul")?`<div class="matrix ${n===2?"r2":"r3"}">
    ${Array.from({length:cells},(_,i)=>`<input id="${matrixId("B",i)}" type="number" step="any">`).join("")}
  </div>`:"");
}
$("matrixSize").onchange=renderMatrixInputs;
$("matrixOp").onchange=renderMatrixInputs;
$("matrixCalc").onclick=()=>{
  if(!requireWasm())return;
  const n=Number($("matrixSize").value), op={det:1,transpose:2,inverse:3,add:4,sub:5,mul:6}[$("matrixOp").value];
  const cells=n*n, A=Array.from({length:cells},(_,i)=>num(matrixId("A",i)));
  const B=Array.from({length:cells},(_,i)=>num(matrixId("B",i)));
  const aPtr=Module._malloc(cells*8), bPtr=Module._malloc(cells*8);
  Module.HEAPF64.set(A,aPtr/8); Module.HEAPF64.set(B,bPtr/8);
  const r=callC("web_matrix","string",["number","number","number","number"],[n,op,aPtr,bPtr]);
  Module._free(aPtr); Module._free(bPtr); setResult(r);
};

function renderVectorInputs(){
  const n=Number($("vectorType").value);
  $("vectorInputs").innerHTML=`<div class="vector-grid">
    <div><h3>Vector V1</h3>${["x","y",...(n===3?["z"]:[])].map(x=>`<input id="v1${x}" type="number" step="any" placeholder="${x.toUpperCase()}">`).join("")}</div>
    <div><h3>Vector V2</h3>${["x","y",...(n===3?["z"]:[])].map(x=>`<input id="v2${x}" type="number" step="any" placeholder="${x.toUpperCase()}">`).join("")}</div>
  </div>`;
}
$("vectorType").onchange=renderVectorInputs;
$("vectorCalc").onclick=()=>{
  if(!requireWasm())return;
  const n=Number($("vectorType").value),op={add:1,sub:2,dot:3,cross:4}[$("vectorOp").value];
  const r=callC("web_vector","string",
    ["number","number","number","number","number","number","number","number"],
    [n,op,num("v1x"),num("v1y"),num("v1z"),num("v2x"),num("v2y"),num("v2z")]);
  setResult(r);
};
