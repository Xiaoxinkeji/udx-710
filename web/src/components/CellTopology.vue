<template>
  <div class="topology-container p-4 bg-slate-900/50 backdrop-blur-xl rounded-3xl border border-white/10 shadow-2xl relative overflow-hidden h-[500px]">
    <!-- Background Grid -->
    <div class="absolute inset-0 opacity-10 pointer-events-none">
      <div v-for="i in 5" :key="i" 
           :style="{ width: i*20 + '%', height: i*20 + '%', left: (50 - i*10) + '%', top: (50 - i*10) + '%' }"
           class="absolute border border-white rounded-full"></div>
      <div class="absolute w-full h-[1px] bg-white top-1/2 left-0"></div>
      <div class="absolute h-full w-[1px] bg-white left-1/2 top-0"></div>
    </div>

    <!-- Main Title -->
    <div class="mb-6 relative z-10">
      <h3 class="text-white text-xl font-black tracking-widest flex items-center gap-2">
        <span class="w-2 h-2 bg-blue-500 rounded-full animate-pulse"></span>
        蜂窝网络拓扑 (CELLULAR TOPOLOGY)
      </h3>
      <p class="text-slate-400 text-xs mt-1 uppercase">Electromagnetic environment visualization</p>
    </div>

    <!-- SVG Radar -->
    <svg viewBox="0 0 400 400" class="w-full h-full relative z-10">
      <!-- Connections to neighbors -->
      <line v-for="(cell, index) in neighbors" :key="'line-' + index"
            :x1="200" :y1="200"
            :x2="calculateX(cell, index)" :y2="calculateY(cell, index)"
            class="stroke-blue-400/30 line-animate"
            stroke-width="1"
            stroke-dasharray="5,5" />

      <!-- Main Cell (Serving) -->
      <g @mouseenter="hovered = 'main'" @mouseleave="hovered = null" transform="translate(200, 200)">
        <circle r="12" class="fill-blue-500 animate-ping opacity-20" />
        <circle r="8" class="fill-blue-600 shadow-xl" />
        <text y="-15" text-anchor="middle" class="fill-white text-[10px] font-bold uppercase tracking-tight">Serving Cell</text>
      </g>

      <!-- Neighbor Cells -->
      <g v-for="(cell, index) in neighbors" :key="'cell-' + index"
         :transform="`translate(${calculateX(cell, index)}, ${calculateY(cell, index)})`"
         @mouseenter="hovered = cell" @mouseleave="hovered = null">
        <circle r="6" :class="cell.rsrp > -100 ? 'fill-emerald-500' : 'fill-amber-500'" class="hover:scale-125 transition-transform cursor-crosshair" />
        <text y="15" text-anchor="middle" class="fill-slate-300 text-[8px] uppercase tracking-tighter">
          {{ cell.tech }} / {{ cell.rsrp }}dBm
        </text>
      </g>
    </svg>

    <!-- Detailed Tooltip -->
    <div v-if="hovered" class="absolute bottom-4 right-4 bg-slate-800/90 border border-white/20 p-3 rounded-xl backdrop-blur-md z-20 min-w-[200px] shadow-2xl">
      <div v-if="hovered === 'main'">
        <p class="text-blue-400 font-bold text-sm">主服务小区</p>
        <p class="text-white text-xs mt-1">当前数据承载主体</p>
      </div>
      <div v-else>
        <div class="flex justify-between items-center mb-1">
          <span class="bg-slate-700 px-2 py-0.5 rounded text-[10px] text-white uppercase">{{ hovered.tech }}</span>
          <span class="text-emerald-400 font-mono font-bold">{{ hovered.rsrp }} dBm</span>
        </div>
        <p class="text-white text-xs">小区 ID: <span class="text-slate-400">{{ hovered.cell_id }}</span></p>
        <p class="text-white text-xs">EARFCN: <span class="text-slate-400">{{ hovered.earfcn }}</span></p>
        <div class="mt-2 h-1 bg-slate-700 rounded-full overflow-hidden">
          <div class="h-full bg-emerald-500 transition-all duration-1000" :style="{ width: calculateSignalWidth(hovered.rsrp) + '%' }"></div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { getNeighborCells } from '../composables/useApi'

const neighbors = ref([])
const hovered = ref(null)
let timer = null

const calculateX = (cell, index) => {
  const angle = (index / (neighbors.value.length || 1)) * 2 * Math.PI
  // RSRP typically -140 to -40. Normalize to distance 50 to 180
  const dist = 50 + (Math.abs(-40 - cell.rsrp) / 100) * 130
  return 200 + Math.cos(angle) * dist
}

const calculateY = (cell, index) => {
  const angle = (index / (neighbors.value.length || 1)) * 2 * Math.PI
  const dist = 50 + (Math.abs(-40 - cell.rsrp) / 100) * 130
  return 200 + Math.sin(angle) * dist
}

const calculateSignalWidth = (rsrp) => {
  const w = ((rsrp + 140) / 100) * 100
  return Math.min(Math.max(w, 0), 100)
}

const fetchData = async () => {
  try {
    const data = await getNeighborCells()
    if (Array.isArray(data)) {
      neighbors.value = data
    }
  } catch (e) {
    console.error('Failed to fetch neighbors', e)
  }
}

onMounted(() => {
  fetchData()
  timer = setInterval(fetchData, 10000)
})

onUnmounted(() => {
  if (timer) clearInterval(timer)
})
</script>

<style scoped>
.line-animate {
  stroke-dashoffset: 100;
  animation: dash 10s linear infinite;
}
@keyframes dash {
  to {
    stroke-dashoffset: 0;
  }
}
</style>
