#pragma once

#if defined(ARDUINO_ARCH_ESP32)

static const char WEB_DASHBOARD_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>Dedicuino</title>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400&family=JetBrains+Mono:wght@300;400&display=swap" rel="stylesheet">
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }

    :root {
      --bg:    #0e0e0e;
      --line:  rgba(255,255,255,0.07);
      --text:  #e8e8e8;
      --dim:   rgba(255,255,255,0.28);
      --amber: #d4883a;
      --teal:  #3ab8a8;
      --ok:    #4caf7d;
    }

    body {
      font-family: 'Inter', sans-serif;
      background: var(--bg);
      color: var(--text);
      min-height: 100vh;
    }

    .page {
      max-width: 680px;
      margin: 0 auto;
      padding: 48px 24px 64px;
    }

    /* header */
    .header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin-bottom: 52px;
    }

    .wordmark {
      font-family: 'JetBrains Mono', monospace;
      font-weight: 300;
      font-size: 0.78rem;
      letter-spacing: 0.22em;
      color: var(--dim);
      text-transform: uppercase;
    }

    .status {
      display: flex;
      align-items: center;
      gap: 8px;
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.68rem;
      letter-spacing: 0.18em;
      color: var(--dim);
      text-transform: uppercase;
    }

    .dot {
      width: 6px; height: 6px;
      border-radius: 50%;
      background: var(--ok);
      animation: blink 2.4s ease-in-out infinite;
      flex-shrink: 0;
    }

    @keyframes blink {
      0%,100% { opacity: 1; }
      50%      { opacity: 0.25; }
    }

    /* timer */
    .timer-block { margin-bottom: 52px; }

    .section-label {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.62rem;
      letter-spacing: 0.2em;
      color: var(--dim);
      text-transform: uppercase;
      margin-bottom: 8px;
    }

    .timer-value {
      font-family: 'JetBrains Mono', monospace;
      font-weight: 300;
      font-size: clamp(4rem, 13vw, 7rem);
      line-height: 1;
      letter-spacing: -0.02em;
      color: var(--text);
    }

    .timer-value .unit {
      font-size: 0.28em;
      letter-spacing: 0.12em;
      color: var(--dim);
      margin-left: 4px;
    }

    .bar-wrap {
      margin-top: 18px;
      height: 1px;
      background: var(--line);
    }

    .bar-fill {
      height: 100%;
      background: var(--dim);
      width: 0%;
      transition: width 0.45s linear;
    }

    /* metrics */
    .metrics {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 1px;
      background: var(--line);
      border: 1px solid var(--line);
    }

    .metric {
      background: var(--bg);
      padding: 24px 20px 20px;
    }

    .metric-value {
      font-family: 'JetBrains Mono', monospace;
      font-weight: 300;
      font-size: 2.7rem;
      line-height: 1;
      letter-spacing: -0.01em;
    }

    .metric-value.pressure { color: var(--amber); }
    .metric-value.temp     { color: var(--teal); }

    .metric-unit {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.8rem;
      color: var(--dim);
      margin-left: 3px;
    }

    .metric-stats {
      margin-top: 14px;
      display: flex;
      gap: 16px;
    }

    .mini-stat { display: flex; flex-direction: column; gap: 3px; }

    .mini-label {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.56rem;
      letter-spacing: 0.16em;
      color: rgba(255,255,255,0.18);
      text-transform: uppercase;
    }

    .mini-value {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.72rem;
      color: var(--dim);
    }

    canvas.chart {
      display: block;
      width: 100%;
      height: 68px;
      margin-top: 16px;
      cursor: pointer;
    }

    canvas.chart:hover { opacity: 0.8; }

    /* detail overlay */
    .overlay {
      display: none;
      position: fixed; inset: 0;
      background: rgba(0,0,0,0.88);
      z-index: 100;
      align-items: center;
      justify-content: center;
      padding: 24px;
      backdrop-filter: blur(4px);
    }

    .overlay.open { display: flex; }

    .overlay-box {
      width: 100%;
      max-width: 720px;
      background: #111;
      border: 1px solid var(--line);
      border-radius: 6px;
      padding: 28px 24px 24px;
      position: relative;
    }

    .overlay-header {
      display: flex;
      align-items: baseline;
      justify-content: space-between;
      margin-bottom: 20px;
    }

    .overlay-title {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.65rem;
      letter-spacing: 0.2em;
      color: var(--dim);
      text-transform: uppercase;
    }

    .overlay-close {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.65rem;
      letter-spacing: 0.15em;
      color: rgba(255,255,255,0.25);
      cursor: pointer;
      background: none;
      border: none;
      text-transform: uppercase;
      padding: 4px 0;
    }

    .overlay-close:hover { color: var(--text); }

    #detailCanvas {
      display: block;
      width: 100%;
      height: 260px;
      border-radius: 4px;
    }

    .overlay-hint {
      margin-top: 12px;
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.58rem;
      letter-spacing: 0.13em;
      color: rgba(255,255,255,0.15);
      text-align: right;
    }

    /* footer */
    .footer {
      margin-top: 28px;
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.62rem;
      letter-spacing: 0.15em;
      color: rgba(255,255,255,0.18);
    }

    @media (max-width: 480px) {
      .metrics { grid-template-columns: 1fr; }
      .timer-value { font-size: 4rem; }
    }
  </style>
</head>
<body>
<div class="page">

  <div class="header">
    <div class="wordmark">Dedicuino</div>
    <div class="status">
      <div class="dot" id="dot"></div>
      <span id="state">idle</span>
    </div>
  </div>

  <div class="timer-block">
    <div class="section-label">Shot timer</div>
    <div class="timer-value">
      <span id="timer">00.0</span><span class="unit">s</span>
    </div>
    <div class="bar-wrap"><div class="bar-fill" id="timerBar"></div></div>
  </div>

  <div class="metrics">
    <div class="metric">
      <div class="section-label">Pressure</div>
      <span class="metric-value pressure" id="pressure">0.0</span><span class="metric-unit">bar</span>
      <div class="metric-stats">
        <div class="mini-stat">
          <span class="mini-label">avg</span>
          <span class="mini-value" id="pAvg">—</span>
        </div>
        <div class="mini-stat">
          <span class="mini-label">peak</span>
          <span class="mini-value" id="pMax">—</span>
        </div>
      </div>
      <canvas id="pressureChart" class="chart"></canvas>
    </div>

    <div class="metric">
      <div class="section-label">Temperature</div>
      <span class="metric-value temp" id="temp">0.0</span><span class="metric-unit">°C</span>
      <div class="metric-stats">
        <div class="mini-stat">
          <span class="mini-label">avg</span>
          <span class="mini-value" id="tAvg">—</span>
        </div>
        <div class="mini-stat">
          <span class="mini-label">peak</span>
          <span class="mini-value" id="tMax">—</span>
        </div>
      </div>
      <canvas id="tempChart" class="chart"></canvas>
    </div>
  </div>

  <div class="footer" id="net">connecting…</div>

</div>

<!-- detail overlay -->
<div class="overlay" id="overlay">
  <div class="overlay-box">
    <div class="overlay-header">
      <span class="overlay-title" id="overlayTitle">last shot</span>
      <button class="overlay-close" id="overlayClose">close ×</button>
    </div>
    <canvas id="detailCanvas"></canvas>
    <div class="overlay-hint">last <span id="overlayPts">—</span> samples · tap outside to close</div>
  </div>
</div>
<script>
  const byId = id => document.getElementById(id);
  const fmt  = (v, d=1) => Number(v||0).toFixed(d);
  const set  = (id, v) => { byId(id).textContent = v; };

  const MAX_PTS = 120, SHOT_MAX = 60;
  const pH = [], tH = [];
  // snapshot of last completed shot
  let lastShot = { pressure: [], temp: [] };
  let prevState = '';

  function push(arr, v) {
    arr.push(Number(v||0));
    if (arr.length > MAX_PTS) arr.shift();
  }

  /* ── sparkline (small) ── */
  function draw(id, vals, color) {
    const c = byId(id);
    if (!c) return;
    const ctx = c.getContext('2d');
    const W = c.clientWidth, H = c.clientHeight;
    if (!W || !H) return;
    if (c.width !== W || c.height !== H) { c.width = W; c.height = H; }
    ctx.clearRect(0, 0, W, H);
    if (vals.length < 2) return;
    drawCurve(ctx, vals, W, H, color, 0, 0, 1.2, false);
  }

  /* ── shared curve renderer ── */
  function drawCurve(ctx, vals, W, H, color, padL, padB, lw, axes) {
    const usableW = W - padL - 6;
    const usableH = H - padB - 8;
    const mn = Math.min(...vals), mx = Math.max(...vals);
    const span = Math.max(0.5, mx - mn);
    const lo = mn - span*0.12, hi = mx + span*0.12;
    const rng = Math.max(0.5, hi - lo);
    const n = vals.length;

    const px = i => padL + (i / (MAX_PTS - 1)) * usableW;
    const py = v => 8 + usableH - ((v - lo) / rng) * usableH;

    if (axes) {
      ctx.font = '10px JetBrains Mono, monospace';
      ctx.fillStyle = 'rgba(255,255,255,0.25)';
      ctx.textAlign = 'right';
      const ticks = 5;
      for (let i = 0; i <= ticks; i++) {
        const t = i / ticks;
        const v = lo + t * rng;
        const y = py(v);
        ctx.strokeStyle = 'rgba(255,255,255,0.05)';
        ctx.lineWidth = 1;
        ctx.beginPath(); ctx.moveTo(padL, y); ctx.lineTo(W - 6, y); ctx.stroke();
        ctx.fillText(v.toFixed(1), padL - 4, y + 3);
      }
      // x-axis time labels
      ctx.textAlign = 'center';
      ctx.fillStyle = 'rgba(255,255,255,0.2)';
      const xTicks = 6;
      for (let i = 0; i <= xTicks; i++) {
        const frac = i / xTicks;
        const idx = Math.round(frac * (n - 1));
        const secAgo = ((n - 1 - idx) * 0.5).toFixed(0);
        const x = px(idx);
        ctx.fillText(`-${secAgo}s`, x, H - 4);
      }
    }

    const g = ctx.createLinearGradient(0, 8, 0, 8 + usableH);
    g.addColorStop(0, color.replace('rgb(','rgba(').replace(')',',0.18)'));
    g.addColorStop(1, 'rgba(0,0,0,0)');

    ctx.beginPath();
    vals.forEach((v, i) => i ? ctx.lineTo(px(i), py(v)) : ctx.moveTo(px(i), py(v)));
    ctx.lineTo(px(n-1), 8 + usableH);
    ctx.lineTo(px(0),   8 + usableH);
    ctx.closePath();
    ctx.fillStyle = g;
    ctx.fill();

    ctx.beginPath();
    vals.forEach((v, i) => i ? ctx.lineTo(px(i), py(v)) : ctx.moveTo(px(i), py(v)));
    ctx.strokeStyle = color;
    ctx.lineWidth = lw;
    ctx.globalAlpha = axes ? 0.9 : 0.7;
    ctx.stroke();
    ctx.globalAlpha = 1;

    // latest dot (detail only)
    if (axes && n > 0) {
      const lx = px(n-1), ly = py(vals[n-1]);
      ctx.beginPath(); ctx.arc(lx, ly, 3, 0, Math.PI*2);
      ctx.fillStyle = color; ctx.fill();
    }
  }

  /* ── overlay ── */
  let overlayData = [], overlayColor = '', overlayLabel = '';

  function openOverlay(data, color, label, unit) {
    overlayData  = [...data];
    overlayColor = color;
    overlayLabel = label;
    set('overlayTitle', `${label}  ·  last shot`);
    set('overlayPts', `${data.length}`);
    byId('overlay').classList.add('open');
    requestAnimationFrame(renderDetail);
  }

  function renderDetail() {
    const c = byId('detailCanvas');
    if (!c) return;
    const ctx = c.getContext('2d');
    const W = c.clientWidth, H = c.clientHeight;
    if (!W || !H) return;
    if (c.width !== W || c.height !== H) { c.width = W; c.height = H; }
    ctx.clearRect(0, 0, W, H);
    if (overlayData.length < 2) return;
    drawCurve(ctx, overlayData, W, H, overlayColor, 36, 22, 1.8, true);
  }

  byId('overlayClose').addEventListener('click', () => byId('overlay').classList.remove('open'));
  byId('overlay').addEventListener('click', e => { if (e.target === byId('overlay')) byId('overlay').classList.remove('open'); });
  window.addEventListener('resize', () => { if (byId('overlay').classList.contains('open')) renderDetail(); });

  byId('pressureChart').addEventListener('click', () => openOverlay(lastShot.pressure.length ? lastShot.pressure : pH, 'rgb(212,136,58)', 'Pressure'));
  byId('tempChart').addEventListener('click',     () => openOverlay(lastShot.temp.length     ? lastShot.temp     : tH, 'rgb(58,184,168)',  'Temperature'));

  /* ── poll ── */
  async function refresh() {
    try {
      const s = await fetch('/api/status', {cache:'no-store'}).then(r => r.json());

      const state = (s.state||'idle').toLowerCase();
      set('state', state);

      // capture shot snapshot when brewing ends
      if (prevState === 'brewing' && state !== 'brewing' && pH.length > 2) {
        lastShot.pressure = [...pH];
        lastShot.temp     = [...tH];
      }
      prevState = state;

      const t = Number(s.timerSeconds||0);
      set('timer', fmt(t,1));
      byId('timerBar').style.width = Math.min(100, t/SHOT_MAX*100) + '%';

      set('pressure', fmt(s.pressureBar,1));
      set('temp',     fmt(s.tempC,1));
      set('pAvg', fmt(s.pAvg,1));
      set('pMax', fmt(s.pMax,1));
      set('tAvg', fmt(s.tAvg,1));
      set('tMax', fmt(s.tMax,1));

      push(pH, s.pressureBar); push(tH, s.tempC);
      draw('pressureChart', pH, 'rgb(212,136,58)');
      draw('tempChart',     tH, 'rgb(58,184,168)');

      set('net', `rssi ${s.wifiRssi} dBm`);
      byId('dot').style.background = state === 'brewing' ? 'var(--amber)' : 'var(--ok)';
    } catch {
      set('net', 'unreachable');
      byId('dot').style.background = '#c0392b';
    }
  }

  refresh();
  setInterval(refresh, 500);
</script>
</body>
</html>
)HTML";

#endif
