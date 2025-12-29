/**
 * @name UFI Dashboard
 * @version 1.0.1
 * @author 小新科技
 * @description 沉浸式数据监控看板，展示信号、流量、硬件状态等详细信息。
 */

// 修正模板字符串转义问题
const DASHBOARD_HTML = `
<!DOCTYPE html>
<html lang="zh-CN">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
	<title>UFI-TOOLS-AI看板</title>
	<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
	<style>
:root { --bg: #1e1e1e; --text: #e7ebf5; --accent: #4aa8ff; --card-bg: rgba(255, 255, 255, 0.05); }
body { margin: 0; font-family: system-ui; color: var(--text); background: var(--bg); overflow: hidden; }
.container { height: 100vh; display: flex; flex-direction: column; padding: 20px; }
.header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
.back-btn { background: var(--card-bg); border: 1px solid rgba(255,255,255,0.1); color: #fff; padding: 8px 16px; border-radius: 8px; cursor: pointer; }
.grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 16px; flex: 1; overflow: auto; }
.card { background: var(--card-bg); border: 1px solid rgba(255,255,255,0.1); border-radius: 12px; padding: 16px; }
.kpi-row { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; margin-bottom: 10px; }
.kpi { background: rgba(0,0,0,0.2); padding: 10px; border-radius: 8px; text-align: center; }
.kpi-label { font-size: 12px; color: #888; }
.kpi-value { font-size: 18px; font-weight: bold; }
.progress-bar { height: 8px; background: rgba(0,0,0,0.3); border-radius: 4px; overflow: hidden; margin: 10px 0; }
.progress-fill { height: 100%; background: var(--accent); transition: width 0.3s; }
canvas { height: 200px !important; }
	</style>
</head>
<body>
	<div class="container">
		<header class="header">
			<button class="back-btn" onclick="window.parent.postMessage({type:'BACK_TO_UFITOOLS'},'*')">返回</button>
			<h2 style="margin:0">UFI 数据看板</h2>
		</header>
		<div class="grid">
			<div class="card" style="grid-column: span 2">
				<div class="kpi-row">
					<div class="kpi"><div class="kpi-label">网络</div><div id="net">-</div></div>
					<div class="kpi"><div class="kpi-label">信号</div><div id="sig">-</div></div>
					<div class="kpi"><div class="kpi-label">CPU</div><div id="cpu">-</div></div>
				</div>
                <div class="kpi-row">
					<div class="kpi"><div class="kpi-label">内存</div><div id="mem">-</div></div>
					<div class="kpi"><div class="kpi-label">温度</div><div id="temp">-</div></div>
					<div class="kpi"><div class="kpi-label">QCI</div><div id="qci">-</div></div>
				</div>
			</div>
			<div class="card">
				<div style="margin-bottom:10px">资源趋势</div>
				<canvas id="chart"></canvas>
			</div>
		</div>
	</div>
	<script>
(() => {
	let chart;
	window.addEventListener('message', e => {
		if (e.data.type === 'UFI_UPDATE') {
			const d = e.data.payload;
			document.getElementById('net').innerText = d.network_type || '-';
			document.getElementById('sig').innerText = d.signal_strength || '-';
			document.getElementById('cpu').innerText = (d.cpu_usage || 0).toFixed(1) + '%';
			document.getElementById('temp').innerText = (d.thermal_temp || 0).toFixed(1) + '°C';
            document.getElementById('qci').innerText = d.qci || '-';
            const mem = ((d.total_ram - d.free_ram) / d.total_ram * 100).toFixed(1);
            document.getElementById('mem').innerText = mem + '%';

			if (chart) {
				chart.data.labels.push(new Date().toLocaleTimeString());
				chart.data.datasets[0].data.push(d.cpu_usage);
				if (chart.data.labels.length > 20) { chart.data.labels.shift(); chart.data.datasets[0].data.shift(); }
				chart.update('none');
			}
		}
	});

	function init() {
		if (typeof Chart === 'undefined') return setTimeout(init, 100);
		chart = new Chart(document.getElementById('chart'), {
			type: 'line',
			data: { labels: [], datasets: [{ label: 'CPU %', data: [], borderColor: '#4aa8ff', backgroundColor: 'rgba(74,168,255,0.1)', fill: true, tension: 0.4 }] },
			options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { y: { beginAtZero: true, max: 100 } } }
		});
	}
	init();
})();
	</script>
</body>
</html>
`;

window.PLUGIN = {
	template: `
    <plugin-card title="数据看板" icon="fa-chart-line">
        <div class="p-6 text-center">
            <div class="mb-4 text-slate-400">
                <i class="fas fa-desktop text-5xl mb-3"></i>
                <p class="text-lg">全屏沉浸式数据看板</p>
            </div>
            <button @click="openDashboard" class="px-8 py-3 bg-gradient-to-r from-blue-600 to-cyan-500 text-white rounded-xl shadow-lg hover:shadow-cyan-500/20 active:scale-95 transition-all font-bold">
                <i class="fas fa-external-link-alt mr-2"></i>开启监控
            </button>
        </div>
    </plugin-card>
  `,
	setup() {
		const openDashboard = () => {
			const id = 'ufi-db-iframe';
			if (document.getElementById(id)) document.getElementById(id).remove();
			const iframe = document.createElement('iframe');
			iframe.id = id;
			Object.assign(iframe.style, { position: 'fixed', inset: '0', width: '100%', height: '100%', border: '0', zIndex: '9999', background: '#1e1e1e' });
			document.body.appendChild(iframe);
			iframe.contentDocument.open();
			iframe.contentDocument.write(DASHBOARD_HTML);
			iframe.contentDocument.close();

			const timer = setInterval(async () => {
				if (!document.getElementById(id)) return clearInterval(timer);
				try {
					const info = await $api.get('/api/info');
					iframe.contentWindow.postMessage({ type: 'UFI_UPDATE', payload: info }, '*');
				} catch (e) { }
			}, 1000);

			const onMsg = e => {
				if (e.data?.type === 'BACK_TO_UFITOOLS') {
					iframe.remove();
					clearInterval(timer);
					window.removeEventListener('message', onMsg);
				}
			};
			window.addEventListener('message', onMsg);
		};
		return { openDashboard };
	}
}
