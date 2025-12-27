window.PLUGIN = {
    template: `
    <div class="relative bg-black rounded-3xl overflow-hidden min-h-[400px] font-mono text-green-500 p-4">
      <canvas id="matrix-canvas" class="absolute inset-0 w-full h-full opacity-50"></canvas>
      
      <div class="relative z-10 flex flex-col h-full pointer-events-none">
        <div class="flex justify-between items-start">
          <div>
            <div class="text-2xl font-bold tracking-tighter animate-pulse mb-2">SYSTEM MONITOR v1.0</div>
            <div class="text-xs opacity-70">REAL-TIME DATA STREAM ACTIVE</div>
          </div>
          <plugin-status :text="statusText" :type="statusType" />
        </div>

        <div class="mt-8 grid grid-cols-2 gap-4">
          <div class="border border-green-500/30 bg-black/40 p-3 rounded-lg backdrop-blur-sm">
            <div class="text-[10px] uppercase opacity-50 mb-1">CPU Load</div>
            <div class="text-xl font-bold">{{ cpu }}%</div>
            <div class="w-full h-1 bg-green-900/50 mt-2 rounded-full overflow-hidden">
               <div class="h-full bg-green-500 transition-all duration-500" :style="{ width: cpu + '%' }"></div>
            </div>
          </div>
          <div class="border border-green-500/30 bg-black/40 p-3 rounded-lg backdrop-blur-sm">
            <div class="text-[10px] uppercase opacity-50 mb-1">Memory</div>
            <div class="text-xl font-bold">{{ mem }}%</div>
            <div class="w-full h-1 bg-green-900/50 mt-2 rounded-full overflow-hidden">
               <div class="h-full bg-green-500 transition-all duration-500" :style="{ width: mem + '%' }"></div>
            </div>
          </div>
        </div>

        <div class="mt-auto pt-8">
          <div class="text-[10px] uppercase opacity-50 mb-2">Network Traffic</div>
          <div class="text-xs space-y-1">
            <div class="flex justify-between">
              <span>RX_STREAM:</span>
              <span>{{ rx }}</span>
            </div>
            <div class="flex justify-between">
              <span>TX_STREAM:</span>
              <span>{{ tx }}</span>
            </div>
          </div>
        </div>
      </div>

      <div class="absolute bottom-4 right-4 z-20">
        <button class="px-4 py-2 border border-green-500/50 hover:bg-green-500 hover:text-black transition-all rounded-full text-xs" @click="refreshData">
          FORCE SYNC
        </button>
      </div>
    </div>
  `,
    data() {
        return {
            cpu: 0,
            mem: 0,
            rx: '0 B',
            tx: '0 B',
            statusText: 'SCANNING',
            statusType: 'info'
        }
    },
    methods: {
        async refreshData() {
            this.statusText = 'SYNCING'
            this.statusType = 'warning'

            // 使用 API 获取数据
            const infoStr = await this.$api.shell("top -bn1 | head -n 10")
            const cpuMatch = infoStr.match(/CPU:\s+(\d+)%/)
            if (cpuMatch) this.cpu = cpuMatch[1]

            const memMatch = infoStr.match(/Mem:\s+(\d+)K\s+used/)
            const memTotalMatch = infoStr.match(/Mem:\s+(\d+)K\s+total/)
            if (memMatch && memTotalMatch) {
                this.mem = Math.round((parseInt(memMatch[1]) / parseInt(memTotalMatch[1])) * 100)
            }

            const traffic = await this.$api.shell("cat /proc/net/dev | grep sipa_eth0")
            if (traffic) {
                const parts = traffic.trim().split(/\s+/)
                if (parts.length > 9) {
                    this.rx = this.formatBytes(parts[1])
                    this.tx = this.formatBytes(parts[9])
                }
            }

            this.statusText = 'STABLE'
            this.statusType = 'success'
        },
        formatBytes(bytes) {
            bytes = parseInt(bytes)
            if (isNaN(bytes) || bytes === 0) return '0 B'
            const k = 1024
            const sizes = ['B', 'KB', 'MB', 'GB', 'TB']
            const i = Math.floor(Math.log(bytes) / Math.log(k))
            return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i]
        },
        startMatrix() {
            const canvas = document.getElementById('matrix-canvas')
            if (!canvas) return
            const ctx = canvas.getContext('2d')

            canvas.width = canvas.offsetWidth
            canvas.height = canvas.offsetHeight

            const characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$+-*/=%'
            const fontSize = 14
            const columns = canvas.width / fontSize
            const drops = []

            for (let i = 0; i < columns; i++) {
                drops[i] = 1
            }

            const draw = () => {
                ctx.fillStyle = 'rgba(0, 0, 0, 0.1)'
                ctx.fillRect(0, 0, canvas.width, canvas.height)

                ctx.fillStyle = '#0F0'
                ctx.font = fontSize + 'px monospace'

                for (let i = 0; i < drops.length; i++) {
                    const text = characters.charAt(Math.floor(Math.random() * characters.length))
                    ctx.fillText(text, i * fontSize, drops[i] * fontSize)

                    if (drops[i] * fontSize > canvas.height && Math.random() > 0.975) {
                        drops[i] = 0
                    }
                    drops[i]++
                }
            }

            this.$api.$setInterval(draw, 33)
        }
    },
    mounted() {
        this.refreshData()
        this.$api.$setInterval(() => this.refreshData(), 5000)

        // 等待 DOM 渲染后启动矩阵效果
        setTimeout(() => this.startMatrix(), 100)
    }
}
