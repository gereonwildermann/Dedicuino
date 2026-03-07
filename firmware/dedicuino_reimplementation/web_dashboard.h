#pragma once

#if defined(ARDUINO_ARCH_ESP32)

static const char WEB_DASHBOARD_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>Dedicuino Live</title>
  <style>
    :root {
      --bg: #0b0f16;
      --card: rgba(255,255,255,0.08);
      --text: #f4f7ff;
      --muted: #a8b1c2;
      --accent: #ff7a18;
      --accent2: #ffb347;
      --ok: #4ade80;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: Inter, Segoe UI, Roboto, Arial, sans-serif;
      color: var(--text);
      background: radial-gradient(circle at 20% 0%, #1b2640 0%, #0b0f16 50%, #070a10 100%);
      min-height: 100vh;
      padding: 22px;
    }
    .wrap { max-width: 980px; margin: 0 auto; }
    .hero {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
      margin-bottom: 18px;
    }
    .title { font-size: 1.5rem; font-weight: 800; letter-spacing: .2px; }
    .state {
      font-weight: 700;
      border: 1px solid rgba(255,255,255,.2);
      border-radius: 999px;
      padding: 8px 12px;
      background: rgba(255,255,255,.06);
    }
    .grid {
      display: grid;
      gap: 12px;
      grid-template-columns: repeat(2, minmax(0,1fr));
    }
    .card {
      background: var(--card);
      border: 1px solid rgba(255,255,255,0.12);
      border-radius: 18px;
      padding: 14px;
      backdrop-filter: blur(6px);
      box-shadow: 0 10px 26px rgba(0,0,0,.22);
    }
    .label { color: var(--muted); font-size: .85rem; margin-bottom: 4px; }
    .value { font-size: 2rem; font-weight: 800; }
    .unit { font-size: 1rem; color: var(--muted); margin-left: 4px; }
    .row { display: flex; justify-content: space-between; margin-top: 8px; font-size: .92rem; color: var(--muted); }
    .chart {
      display: block;
      width: 100%;
      height: 120px;
      margin-top: 10px;
      border-radius: 10px;
      border: 1px solid rgba(255,255,255,.12);
      background: rgba(0,0,0,.20);
    }
    .footer { margin-top: 14px; color: var(--muted); font-size: .85rem; }
    .dot { display: inline-block; width: 8px; height: 8px; border-radius: 50%; margin-right: 6px; background: var(--ok); }
    @media (max-width: 760px) {
      .grid { grid-template-columns: 1fr; }
      .value { font-size: 1.8rem; }
    }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="hero">
      <div class="title">☕ Dedicuino Live Dashboard</div>
      <div class="state" id="state">IDLE</div>
    </div>

    <div class="grid">
      <div class="card">
        <div class="label">Timer</div>
        <div><span class="value" id="timer">00.0</span><span class="unit">s</span></div>
        <div class="row"><span>Uptime</span><span id="uptime">0 s</span></div>
      </div>

      <div class="card">
        <div class="label">Pressure</div>
        <div><span class="value" id="pressure">0.0</span><span class="unit">bar</span></div>
        <div class="row"><span>Avg</span><span id="pAvg">0.0 bar</span></div>
        <div class="row"><span>Max</span><span id="pMax">0.0 bar</span></div>
        <canvas id="pressureChart" class="chart"></canvas>
      </div>

      <div class="card">
        <div class="label">Temperature</div>
        <div><span class="value" id="temp">0.0</span><span class="unit">°C</span></div>
        <div class="row"><span>Avg</span><span id="tAvg">0.0 °C</span></div>
        <div class="row"><span>Max</span><span id="tMax">0.0 °C</span></div>
        <canvas id="tempChart" class="chart"></canvas>
      </div>
    </div>

    <div class="footer"><span class="dot"></span><span id="net">Connecting…</span></div>
  </div>

  <script>
    const byId = (id) => document.getElementById(id);
    const fmt = (v, d=1) => Number(v || 0).toFixed(d);
    const setText = (id, val) => { byId(id).textContent = val; };

    const MAX_POINTS = 120;
    const pressureHistory = [];
    const tempHistory = [];

    function pushValue(arr, value) {
      arr.push(Number(value || 0));
      if (arr.length > MAX_POINTS) arr.shift();
    }

    function drawSparkline(canvasId, values, color, yLabelFormatter) {
      const canvas = byId(canvasId);
      const ctx = canvas.getContext('2d');
      const cssW = canvas.clientWidth;
      const cssH = canvas.clientHeight;
      if (cssW <= 0 || cssH <= 0) return;

      if (canvas.width !== cssW || canvas.height !== cssH) {
        canvas.width = cssW;
        canvas.height = cssH;
      }

      ctx.clearRect(0, 0, cssW, cssH);

      const padL = 30;
      const padR = 6;
      const padT = 8;
      const padB = 18;
      const w = cssW - padL - padR;
      const h = cssH - padT - padB;

      if (w <= 0 || h <= 0) return;

      const minVRaw = values.length ? Math.min(...values) : 0;
      const maxVRaw = values.length ? Math.max(...values) : 1;
      const span = Math.max(0.5, maxVRaw - minVRaw);
      const minV = minVRaw - span * 0.15;
      const maxV = maxVRaw + span * 0.15;
      const range = Math.max(0.5, maxV - minV);

      ctx.strokeStyle = 'rgba(255,255,255,0.12)';
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(padL, padT);
      ctx.lineTo(padL, padT + h);
      ctx.lineTo(padL + w, padT + h);
      ctx.stroke();

      const ticks = 3;
      ctx.fillStyle = 'rgba(255,255,255,0.65)';
      ctx.font = '10px Segoe UI, Arial, sans-serif';
      for (let i = 0; i <= ticks; i++) {
        const t = i / ticks;
        const y = padT + h - t * h;
        const v = minV + t * range;
        ctx.strokeStyle = 'rgba(255,255,255,0.08)';
        ctx.beginPath();
        ctx.moveTo(padL, y);
        ctx.lineTo(padL + w, y);
        ctx.stroke();
        ctx.fillText(yLabelFormatter(v), 2, y + 3);
      }

      if (values.length < 2) return;

      ctx.strokeStyle = color;
      ctx.lineWidth = 2;
      ctx.beginPath();
      for (let i = 0; i < values.length; i++) {
        const x = padL + (i / (MAX_POINTS - 1)) * w;
        const y = padT + h - ((values[i] - minV) / range) * h;
        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }

    async function refresh() {
      try {
        const res = await fetch('/api/status', { cache: 'no-store' });
        const s = await res.json();

        setText('state', s.state);
        setText('timer', fmt(s.timerSeconds, 1).padStart(4, '0'));
        setText('pressure', fmt(s.pressureBar, 1));
        setText('temp', fmt(s.tempC, 1));

        setText('pAvg', `${fmt(s.pAvg,1)} bar`);
        setText('pMax', `${fmt(s.pMax,1)} bar`);
        setText('tAvg', `${fmt(s.tAvg,1)} °C`);
        setText('tMax', `${fmt(s.tMax,1)} °C`);

        pushValue(pressureHistory, s.pressureBar);
        pushValue(tempHistory, s.tempC);
        drawSparkline('pressureChart', pressureHistory, '#ff7a18', (v) => `${v.toFixed(1)}`);
        drawSparkline('tempChart', tempHistory, '#4ade80', (v) => `${v.toFixed(0)}`);

        setText('uptime', `${Math.round(s.uptimeSec)} s`);
        setText('net', `Wi-Fi RSSI: ${s.wifiRssi} dBm`);
      } catch (e) {
        setText('net', 'Device unreachable');
      }
    }

    refresh();
    setInterval(refresh, 500);
  </script>
</body>
</html>
)HTML";

#endif
