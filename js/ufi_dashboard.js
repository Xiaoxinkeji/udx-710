/**
 * @name UFI Dashboard
 * @version 1.0.0
 * @author User/Xiaoxin
 * @description 沉浸式数据监控看板，展示信号、流量、硬件状态等详细信息。
 */

// 原始看板 HTML 模板 (经过 CDN 适配和变量注入修正)
const DASHBOARD_HTML = \`
<!DOCTYPE html>
<html lang="zh-CN">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
	<title>UFI-TOOLS-AI看板</title>
	<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
	<style>
/* 基础变量 */
:root {
	--bg: #00000059;
	--card-bg: rgba(255, 255, 255, 0.06);
	--card-border: rgba(255, 255, 255, 0.12);
	--text: #e7ebf5;
	--muted: #a8b0c3;
	--accent: #4aa8ff;
	--good: #22c55e;
	--warn: #f59e0b;
	--bad: #ef4444;
	--glass-blur: 16px;
	--radius: 14px;
}
* { box-sizing: border-box; }

html, body {
	margin: 0;
	font-family: 'Inter', system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial, "Noto Sans", "Apple Color Emoji", "Segoe UI Emoji";
	color: var(--text);
	background: var(--bg);
    overflow-x: hidden;
}

.dashboard-container { min-height: 100vh; padding: 20px 16px 40px; }
.header { display: flex; align-items: center; justify-content: space-between; gap: 12px; margin: 8px auto 16px; max-width: 1400px; }
.header-left { display: flex; align-items: center; gap: 10px; }
.net-badge { padding: 4px 8px; border-radius: 8px; background: rgba(42,120,255,0.18); border: 1px solid var(--card-border); font-weight: 700; font-size: 12px; color: #dbe7ff; }
.title { font-weight: 700; letter-spacing: 0.3px; font-size: 20px; }
.header-right { display: flex; align-items: center; gap: 14px; }
.signal { display: inline-flex; gap: 2px; align-items: flex-end; height: 16px; }
.signal .bar { width: 3px; background: rgba(255,255,255,0.3); border-radius: 2px; display: inline-block; }
.signal .b1 { height: 4px; } .signal .b2 { height: 7px; } .signal .b3 { height: 10px; } .signal .b4 { height: 13px; }
.signal .on { background: #6ee7ff; box-shadow: 0 0 6px rgba(110,231,255,0.7); }
.battery { display: inline-flex; align-items: center; gap: 6px; }
.battery-body { position:relative;width: 28px; height: 14px; border: 1px solid var(--card-border); border-radius: 3px; position: relative; background: rgba(255,255,255,0.06);}
.battery-fill { height: 100%; width: 0%; background: linear-gradient(90deg,#22c55e,#4ade80); }
.battery-cap { width: 3px; height: 8px; background: var(--card-border); border-radius: 1px; margin-left: -6px; }
.battery-text { font-weight: 700; font-size: 12px; color: var(--muted); }

.grid { max-width: 1400px; margin: 0 auto; display: grid; grid-template-columns: repeat(12, 1fr); gap: 16px; }
.card { backdrop-filter: blur(var(--glass-blur)); -webkit-backdrop-filter: blur(var(--glass-blur)); border: 1px solid var(--card-border); border-radius: var(--radius); padding: 16px; box-shadow: 0 10px 30px rgba(0,0,0,0.3), inset 0 1px 0 rgba(255,255,255,0.04); }

.kpis { grid-column: span 12; display: grid; grid-template-columns: repeat(6, minmax(0, 1fr)); gap: 12px; }
#card-network > .card-header { grid-column: 1 / -1; }
.kpis.compact { grid-template-columns: repeat(6, 1fr); }
.kpi.wide { grid-column: span 2; }
.duplex { font-weight: 700; font-size: 18px; }
.duplex .rx { color: #93c5fd; } .duplex .tx { color: #86efac; } .duplex .sep { color: var(--muted); margin: 0 6px; }

.kpi { background: linear-gradient(180deg, rgba(255,255,255,0.06), rgba(255,255,255,0.03)); border: 1px solid var(--card-border); border-radius: 12px; padding: 12px; }
.kpi-label { color: var(--muted); font-size: 12px; margin-bottom: 6px; }
.kpi-value { font-size: 18px; font-weight: 700; }
.kpi-value.small { font-size: 14px; word-break: break-all; }

.chart-card { grid-column: span 6; }
.progress-card { grid-column: span 6; }

#card-temp-mem .progress-grid { display: grid; grid-template-columns: repeat(1, 1fr); gap: 30px; }
#card-temp-mem .progress-block { grid-template-columns: 120px 1fr 70px; }

#card-signal .progress-fill { background: linear-gradient(90deg,#60a5fa,#34d399); }
.level-strong { background: linear-gradient(90deg,#22c55e,#86efac) !important; }
.level-medium { background: linear-gradient(90deg,#f59e0b,#fbbf24) !important; }
.level-weak { background: linear-gradient(90deg,#ef4444,#f97316) !important; }

.card-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: 10px; }
.card-title { font-weight: 700; }
.card-extra { color: var(--muted); font-size: 12px; }
.btn-mask { border: 1px solid var(--card-border); background: rgba(255,255,255,0.06); color: var(--text); font-weight: 600; font-size: 12px; padding: 6px 10px; border-radius: 8px; cursor: pointer; transition: background 160ms ease, box-shadow 160ms ease; }
.btn-mask[aria-pressed="true"] { box-shadow: 0 0 0 2px rgba(102,227,255,0.25) inset; }
.btn-mask:hover { background: rgba(255,255,255,0.09); }

canvas { width: 100% !important; height: 288px !important; display: block; }
.chart-card canvas { image-rendering: -moz-crisp-edges; image-rendering: pixelated; }

.progress-block { display: grid; grid-template-columns: 120px 1fr 70px; gap: 10px; align-items: center; }
.progress-label { color: var(--muted); font-size: 13px; }
.progress-bar { width: 100%; height: 12px; border-radius: 999px; background: rgba(255,255,255,0.06); border: 1px solid var(--card-border); overflow: hidden; position: relative; }
.progress-fill { height: 100%; width: 0%; border-radius: 999px; background: linear-gradient(90deg, rgba(74,168,255,0.9), rgba(42,120,255,0.9)); box-shadow: 0 0 10px rgba(74,168,255,0.6); transition: width 300ms ease; }
.progress-value { font-weight: 700; font-size: 14px; text-align: right; }
.divider { height: 10px; }

@media (max-width: 1200px) {
	.chart-card { grid-column: span 12; }
	.progress-card { grid-column: span 12; }
	.kpis { grid-template-columns: repeat(3, 1fr); }
	#card-temp-mem .progress-grid { grid-template-columns: 1fr; }
}
@media (max-width: 720px) {
	.title { font-size: 16px; }
	.subtitle { font-size: 12px; }
	.kpis { grid-template-columns: repeat(2, 1fr); }
	canvas { height: 220px !important; }
	.progress-block { grid-template-columns: 88px 1fr 58px; }
}
#kpi-nr,#kpi-lte { font-size: 12px; }
.span-col-12 { grid-column: span 12; }
#val-rsrp,#val-sinr{font-size:12px}
#kpi-qci{font-size:12px}
#val-storage-int{font-size:12px}
.battery-body.charging::after {
	content: "⚡";
    position: absolute;
    top: 9px;
    left: 9px;
    font-size: 18px;
    transform: rotate(20deg) translate(-50%, -50%);
    pointer-events: none;
}
	</style>
</head>
<body>
	<div class="dashboard-container">
		<header class="header">
			<div class="header-left">
				<div class="btn-mask" id="back_to_ufitools" style="user-select:none; cursor:pointer;">返回</div>
				<div class="title">UFI-数据看板</div>
			</div>
			<div class="header-right">
				<div class="signal" id="icon-signal" aria-label="信号强度">
					<span class="bar b1"></span><span class="bar b2"></span><span class="bar b3"></span><span class="bar b4"></span>
				</div>
				<div class="battery" id="icon-battery" aria-label="电池">
					<div class="battery-body"><div class="battery-fill" id="battery-fill"></div></div>
					<div class="battery-cap"></div>
					<div class="battery-text" id="battery-text">-</div>
				</div>
			</div>
		</header>

		<main class="grid">
			<section class="card kpis compact" id="card-throughput">
				<section class="wide progress-card span-col-12" id="card-signal">
					<div class="card-header">
						<div class="card-title">无线信号质量</div>
						<div class="card-extra" id="signal-label">-</div>
					</div>
                    <div class="progress-block">
						<div class="progress-label">RSRP</div>
						<div class="progress-bar" aria-label="RSRP"><div class="progress-fill" id="bar-rsrp"></div></div>
						<div class="progress-value" id="val-rsrp">-</div>
					</div>
					<div class="divider"></div>
					<div class="progress-block">
						<div class="progress-label">SINR</div>
						<div class="progress-bar" aria-label="SINR"><div class="progress-fill" id="bar-sinr"></div></div>
						<div class="progress-value" id="val-sinr">-</div>
					</div>
				</section>
				<section class="kpis">
					<div class="kpi wide">
						<div class="kpi-label">实时上下行</div>
						<div class="duplex"><span class="rx" id="kpi-rx">-</span><span class="sep">/</span><span class="tx" id="kpi-tx">-</span></div>
					</div>
					<div class="kpi"><div class="kpi-label">QCI</div><div class="kpi-value" id="kpi-qci">-</div></div>
					<div class="kpi"><div class="kpi-label">当日流量</div><div class="kpi-value" id="kpi-daily">-</div></div>
					<div class="kpi"><div class="kpi-label">月流量</div><div class="kpi-value small" id="kpi-month">-</div></div>
					<div class="kpi"><div class="kpi-label">连接设备</div><div class="kpi-value" id="kpi-sta">-</div></div>
				</section>
			</section>

			<section class="card kpis" id="card-network">
				<div class="card-header">
					<div class="card-title">网络信息</div>
					<button class="btn-mask" id="btn-mask" type="button" aria-pressed="true">脱敏：开</button>
				</div>
				<div class="kpi"><div class="kpi-label">网络类型</div><div class="kpi-value" id="kpi-network-type">-</div></div>
				<div class="kpi"><div class="kpi-label">运营商</div><div class="kpi-value" id="kpi-provider">-</div></div>
				<div class="kpi"><div class="kpi-label">IPv6 地址</div><div class="kpi-value small" id="kpi-ipv6">-</div></div>
				<div class="kpi"><div class="kpi-label">LAN 网关</div><div class="kpi-value" id="kpi-lan">-</div></div>
				<div class="kpi"><div class="kpi-label">5G SNR</div><div class="kpi-value" id="kpi-snr">-</div></div>
				<div class="kpi"><div class="kpi-label">5G RSRP</div><div class="kpi-value" id="kpi-rsrp">-</div></div>
				<div class="kpi"><div class="kpi-label">ICCID</div><div class="kpi-value small" id="kpi-iccid">-</div></div>
				<div class="kpi"><div class="kpi-label">IMEI</div><div class="kpi-value small" id="kpi-imei">-</div></div>
				<div class="kpi"><div class="kpi-label">NR 基站</div><div class="kpi-value small" id="kpi-nr">-</div></div>
				<div class="kpi"><div class="kpi-label">LTE 基站</div><div class="kpi-value small" id="kpi-lte">-</div></div>
				<div class="kpi"><div class="kpi-label">UFI-TOOLS版本</div><div class="kpi-value small" id="kpi-version">-</div></div>
				<div class="kpi"><div class="kpi-label">设备型号</div><div class="kpi-value" id="kpi-model">-</div></div>
			</section>

			<section class="card progress-card" id="card-temp-mem">
				<div class="card-header"><div class="card-title">温度 / 内存</div><div class="card-extra" id="temp-mem-label">-</div></div>
				<div class="progress-grid">
					<div class="progress-block"><div class="progress-label">CPU 温度</div><div class="progress-bar" aria-label="CPU 温度"><div class="progress-fill" id="bar-temp-cpu"></div></div><div class="progress-value" id="val-temp-cpu">-</div></div>
					<div class="progress-block"><div class="progress-label">PA 温度</div><div class="progress-bar" aria-label="SoC 温度"><div class="progress-fill" id="bar-temp-soc"></div></div><div class="progress-value" id="val-temp-soc">-</div></div>
					<div class="progress-block"><div class="progress-label">内存使用</div><div class="progress-bar" aria-label="内存使用率"><div class="progress-fill" id="bar-mem"></div></div><div class="progress-value" id="val-mem">-</div></div>
					<div class="progress-block"><div class="progress-label">SWAP 使用</div><div class="progress-bar" aria-label="SWAP 使用率"><div class="progress-fill" id="bar-swap"></div></div><div class="progress-value" id="val-swap">-</div></div>
                    <!-- 存储信息暂时隐藏 -->
				</div>
			</section>

			<section class="card chart-card"><div class="card-header"><div class="card-title">CPU 总占用</div><div class="card-extra" id="cpu-total-label">-</div></div><canvas id="chartCpuTotal"></canvas></section>
			<section class="card chart-card"><div class="card-header"><div class="card-title">CPU 核心使用率</div><div class="card-extra" id="cpu-cores-label">-</div></div><canvas id="chartCpuCores"></canvas></section>
			<section class="card chart-card"><div class="card-header"><div class="card-title">CPU 频率 (MHz)</div><div class="card-extra" id="cpu-freq-label">-</div></div><canvas id="chartCpuFreq"></canvas></section>
		</main>
	</div>

	<script>
(() => {
	'use strict';
	console.log('[dashboard] injected bundle loaded');

    // 初始化数据
	window.UFI_DATA = {};
    
	// 等待 Chart.js + 监听父窗口数据 postMessage
	function onChartReady(cb) {
        if (typeof Chart !== 'undefined') return cb();
		const timer = setInterval(() => { if (typeof Chart !== 'undefined') { clearInterval(timer); cb(); } }, 50);
	}

 	window.addEventListener('message', (ev) => {
 		if (ev && ev.data && ev.data.type === 'UFI_UPDATE') {
 			try { 
                window.UFI_DATA = { ...window.UFI_DATA, ...ev.data.payload };
                // 触发更新
                if (window.tick) window.tick();
            } catch (e) {}
 		}
 	});

	onChartReady(() => {
		Chart.defaults.responsive = true;
		Chart.defaults.maintainAspectRatio = false;
		Chart.defaults.devicePixelRatio = () => window.devicePixelRatio || 1;

		const clamp = (v, min, max) => Math.max(min, Math.min(max, v));
		const toPercentText = (v) => \`\${v.toFixed(1)}%\`;
		const toMbps = (bytesPerSec) => \`\${(bytesPerSec * 8 / 1_000_000).toFixed(2)} Mbps\`;
		const toGB = (bytes) => (bytes / (1024 ** 3)).toFixed(2) + ' GB';
		const toHumanBytes = (bytes) => { if (!bytes) return '0 B'; const u=['B','KB','MB','GB','TB']; let v=bytes,i=0; while(v>=1024&&i<u.length-1){v/=1024;i++;} return \`\${v.toFixed(v<10?2:1)} \${u[i]}\`; };
		const celsiusFromMilli = (m) => m; // 后端已经返回摄氏度，不需要/1000
		const el = (id) => document.getElementById(id);

        const backBtn = el('back_to_ufitools')

        if(backBtn) {
            backBtn.onclick = ()=>{
                try {
                    window.parent.postMessage({ type: 'BACK_TO_UFITOOLS', value: 1 },'*' );
                } catch (e) { console.warn('postMessage bridge failed', e); }
            }
        }

		// 图表
		const cpuTotalChart = new Chart(document.getElementById('chartCpuTotal'), {
			type: 'line',
			data: { labels: [], datasets: [{ label: 'CPU 使用率', data: [], borderColor: 'rgba(74,168,255,1)', backgroundColor: 'rgba(74,168,255,0.15)', borderWidth: 2, tension: 0.25, fill: true, pointRadius: 0 }]},
			options: { animation: { duration: 400, easing: 'easeOutQuart' }, responsive: true, maintainAspectRatio: false,
				layout: { padding: { top: 8, right: 12, bottom: 16, left: 12 } },
				scales: { y: { beginAtZero: true, max: 100, grid: { color: 'rgba(255,255,255,0.08)' }, ticks: { color: '#a8b0c3', autoSkip: true, maxTicksLimit: 5,padding: 6, callback: v => v + '%' } },
				         x: { grid: { display: false }, ticks: { color: '#a8b0c3', autoSkip: true, maxTicksLimit: 6 } } },
				plugins: { legend: { display: false } }
			}
		});
		const cpuCoresChart = new Chart(document.getElementById('chartCpuCores'), {
			type: 'bar',
			data: { labels: [], datasets: [{ label: '核心使用率', data: [], backgroundColor: 'rgba(42,120,255,0.8)' }]},
			options: { animation: { duration: 400, easing: 'easeOutQuart' }, responsive: true, maintainAspectRatio: false,
				layout: { padding: { top: 8, right: 12, bottom: 16, left: 12 } },
				scales: { y: { beginAtZero: true, max: 100, grid: { color: 'rgba(255,255,255,0.08)' }, ticks: { color: '#a8b0c3', autoSkip: true, maxTicksLimit: 5,padding: 6, callback: v => v + '%' } },
				         x: { grid: { display: false }, ticks: { color: '#a8b0c3', autoSkip: true, maxTicksLimit: 8 } } },
				plugins: { legend: { display: false } }
			}
		});
		const cpuFreqChart = new Chart(document.getElementById('chartCpuFreq'), {
			type: 'bar',
			data: { labels: [], datasets: [{ label: '当前频率', data: [], backgroundColor: 'rgba(74,168,255,0.85)' }, { label: '最大频率', data: [], backgroundColor: 'rgba(168, 181, 255, 0.55)' }]},
			options: { animation: { duration: 400, easing: 'easeOutQuart' }, responsive: true, maintainAspectRatio: false,
				layout: { padding: { top: 8, right: 12, bottom: 16, left: 12 } },
				scales: { y: { beginAtZero: true, grid: { color: 'rgba(255,255,255,0.08)' }, ticks: { color: '#a8b0c3', autoSkip: true, maxTicksLimit: 6 } },
				         x: { stacked: false, grid: { display: false }, ticks: { color: '#a8b0c3', autoSkip: true, maxTicksLimit: 8 } } },
				plugins: { legend: { labels: { color: '#a8b0c3' } } }
			}
		});

		function fixCanvasResolution(chart) {
			const dpr = window.devicePixelRatio || 1;
			const canvas = chart.canvas, parent = canvas.parentNode, rect = parent.getBoundingClientRect();
			const w = Math.floor(rect.width), h = Math.floor(rect.height);
			canvas.style.width = w + 'px'; canvas.style.height = h + 'px';
			canvas.width = Math.floor(w * dpr); canvas.height = Math.floor(h * dpr);
			chart.resize();
		}
		function updateFonts(chart) {
			const w = chart.canvas.clientWidth || 300;
			const base = Math.max(10, Math.min(14, Math.round(w / 40)));
			const small = Math.max(9, base - 1);
			chart.options.scales.x.ticks.font = { size: small };
			chart.options.scales.y.ticks.font = { size: small };
			if (chart.options.plugins?.legend?.labels) chart.options.plugins.legend.labels.font = { size: base };
		}
		const ro = new ResizeObserver(() => [cpuTotalChart, cpuCoresChart, cpuFreqChart].forEach(c => { fixCanvasResolution(c); updateFonts(c); c.update('none'); }));
		ro.observe(document.querySelector('.grid'));

		// 脱敏
		const maskState = { on: true };
		const maskICCID = v => v ? v.replace(/(\\d{4})\\d+(\\d{4})/, '$1****$2') : '-';
		const maskIMEI  = v => v ? v.replace(/(\\d{3})\\d+(\\d{2})/, '$1****$2') : '-';
		const maskIPv6  = v => v ? v.replace(/(^.{4}).*(.{4}$)/, '$1:****:$2') : '-';
		const btnMask = document.getElementById('btn-mask');
		if (btnMask) {
			btnMask.setAttribute('aria-pressed', 'true');
			btnMask.addEventListener('click', () => {
				maskState.on = !maskState.on;
				btnMask.setAttribute('aria-pressed', maskState.on ? 'true' : 'false');
				btnMask.textContent = maskState.on ? '脱敏：开' : '脱敏：关';
				try { updateKpis(window.UFI_DATA || {}); } catch(e){}
			});
		}

		function el_dashboard(id){ return document.getElementById(id); }
		function updateKpis(data) {
			el_dashboard('kpi-network-type').textContent = data.network_type || '-';
			el_dashboard('kpi-provider').textContent = data.carrier ?? '-';
			el_dashboard('kpi-ipv6').textContent = '-'; // 暂不支持
			el_dashboard('kpi-lan').textContent = '-';
			el_dashboard('kpi-snr').textContent = '-'; // C代码暂无
			el_dashboard('kpi-rsrp').textContent = data.signal_strength ?? '-';
            el_dashboard('kpi-qci').textContent = data.qci ?? "-";
			el_dashboard('kpi-rx').textContent = toMbps(data.downlink_rate ?? 0);
			el_dashboard('kpi-tx').textContent = toMbps(data.uplink_rate ?? 0);
			el_dashboard('kpi-daily').textContent = '-';
			el_dashboard('kpi-sta').textContent = '-';
			el_dashboard('kpi-model').textContent = data.machine ?? '-';
			el_dashboard('kpi-version').textContent = data.version ?? '-';

			const iccid = data.iccid ?? '';
			const imei  = data.imei ?? '';

			const elICCID = el_dashboard('kpi-iccid'); if (elICCID) elICCID.textContent = maskState.on ? maskICCID(iccid) : (iccid || '-');
			const elIMEI  = el_dashboard('kpi-imei');  if (elIMEI)  elIMEI.textContent  = maskState.on ? maskIMEI(imei)   : (imei  || '-');

			// 电池
			const battPercent = Number(data.battery_capacity || 0);
			const battFill = el_dashboard('battery-fill'), battText = el_dashboard('battery-text');
			if (battFill) {
				battFill.style.width = \`\${Math.max(0, Math.min(100, battPercent))}%\`
			};
			if (battText) battText.textContent = battPercent ? \`\${battPercent}%\` : '-';
		}

		function barFill(id, p, mode){ const f = el_dashboard(id); if (!f) return; f.style.width = \`\${clamp(p,0,100)}%\`; if(mode==='temp'){ let c=p; let g='linear-gradient(90deg,#4aa8ff,#2a78ff)'; if(c>=80) g='linear-gradient(90deg,#ef4444,#f97316)'; else if(c>=65) g='linear-gradient(90deg,#f59e0b,#fbbf24)'; f.style.background=g; } }
		function updateTemps(data){
			const cpuC = data.thermal_temp || 0;
			const socC = cpuC; // 暂无 SoC 温度
			barFill('bar-temp-cpu', clamp(cpuC,0,100), 'temp'); barFill('bar-temp-soc', clamp(socC,0,100), 'temp');
			el_dashboard('val-temp-cpu').textContent = \`\${cpuC.toFixed(1)} °C\`; el_dashboard('val-temp-soc').textContent = '-';
			const usedB = Number(data.total_ram - data.free_ram)*1024; const totalB = Number(data.total_ram)*1024;
			el_dashboard('temp-mem-label').textContent = \`Temp \${cpuC.toFixed(1)} °C · MEM \${toGB(usedB)} / \${toGB(totalB)}\`;
		}
		function updateMemory(data){
            const total = Number(data.total_ram || 1);
            const free = Number(data.free_ram || 0);
            const used = total - free;
			const memPct = (used / total) * 100;
            
			barFill('bar-mem', memPct); 
			el_dashboard('val-mem').textContent = toPercentText(memPct); 
            // Swap 暂无
            el_dashboard('val-swap').textContent = '-';
		}
		function updateStorage(data){
             // 暂无存储信息
		}
		function updateCpuTotal(data){
			const now = new Date(), label = now.toLocaleTimeString();
			const usage = Number(data.cpu_usage || 0);
			el_dashboard('cpu-total-label').textContent = toPercentText(usage);
			const L = cpuTotalChart.data.labels, S = cpuTotalChart.data.datasets[0].data;
			L.push(label); S.push(clamp(usage,0,100)); if(L.length>60){L.shift(); S.shift();}
			cpuTotalChart.update();
		}
		function updateCpuCores(data){
            // 暂无核心数据
		}
		function updateCpuFreq(data){
            // 暂无频率数据
		}
		function updateSignalBars(data){
             // 信号处理暂略，使用默认值
		}

        // 导出 tick 以便外部触发
		window.tick = () => {
			const d = window.UFI_DATA || {};
			updateKpis(d); updateCpuTotal(d); updateCpuCores(d); updateCpuFreq(d);
			updateTemps(d); updateMemory(d); updateStorage(d); updateSignalBars(d);
		}

		try { window.tick(); } catch(e){ console.error(e); }
	});
})();
	</script>
</body>
</html>
\`;

window.PLUGIN = {
  template: \`
    <plugin-card title="数据看板" icon="fa-chart-line">
        <div class="p-4 text-center">
            <div class="mb-4 text-slate-400">
                <i class="fas fa-desktop text-4xl mb-2"></i>
                <p>沉浸式数据监控看板</p>
                <p class="text-xs mt-2 opacity-70">点击下方按钮开启全屏监控</p>
            </div>
            <button @click="openDashboard" class="px-6 py-2 bg-gradient-to-r from-blue-500 to-cyan-400 text-white rounded-lg hover:shadow-lg transition-all font-bold">
                <i class="fas fa-external-link-alt mr-2"></i>打开看板
            </button>
        </div>
    </plugin-card>
  \`,
  setup() {
    const openDashboard = () => {
        // ID
        const IFRAME_ID = 'ufi-dashboard-iframe';
        let iframe = document.getElementById(IFRAME_ID);
        if (iframe) iframe.remove();

        // 创建 iframe
        iframe = document.createElement('iframe');
        iframe.id = IFRAME_ID;
        iframe.style.position = 'fixed';
        iframe.style.inset = '0';
        iframe.style.width = '100%';
        iframe.style.height = '100%';
        iframe.style.border = '0';
        iframe.style.zIndex = '9999';
        iframe.style.backgroundColor = '#1e1e1e';
        
        // 挂载
        document.body.appendChild(iframe);
        
        // 写入内容
        iframe.contentDocument.open();
        iframe.contentDocument.write(DASHBOARD_HTML);
        iframe.contentDocument.close();

        // 定时数据同步
        const timer = setInterval(async () => {
            try {
                // 如果 iframe 被移除了，停止定时器
                if (!document.getElementById(IFRAME_ID)) {
                    clearInterval(timer);
                    return;
                }
                
                // 获取数据
                const info = await $api.get('/api/info');
                
                // 发送给 iframe
                iframe.contentWindow.postMessage({
                    type: 'UFI_UPDATE',
                    payload: info
                }, '*');
                
            } catch (e) {
                console.error("Dashboard sync error:", e);
            }
        }, 1000);

        // 监听退出消息
        const closeHandler = (event) => {
            if (event.data && event.data.type === 'BACK_TO_UFITOOLS') {
                iframe.remove();
                clearInterval(timer);
                window.removeEventListener('message', closeHandler);
            }
        };
        window.addEventListener('message', closeHandler);
    };

    return {
      openDashboard
    }
  }
}
