<template>
  <div class="geek-logger flex flex-col h-[calc(100vh-12rem)] bg-slate-950 rounded-3xl border border-white/10 shadow-2xl overflow-hidden font-mono uppercase">
    <!-- Toolbar -->
    <div class="flex flex-wrap items-center justify-between p-4 bg-slate-900 border-b border-white/10 gap-4">
      <div class="flex items-center gap-4">
        <h3 class="text-white font-black tracking-widest flex items-center gap-2">
          <span :class="wsConnected ? 'bg-emerald-500 shadow-[0_0_10px_#10b981]' : 'bg-red-500 animate-pulse'" class="w-2 h-2 rounded-full"></span>
          GEEK LOGGER
        </h3>
        <div class="h-4 w-[1px] bg-white/10 mx-2 hidden sm:block"></div>
        <div class="flex items-center gap-2">
          <input v-model="filter" placeholder="FILTER..." class="bg-slate-800 border border-white/5 rounded px-3 py-1 text-xs text-blue-300 focus:outline-none focus:border-blue-500 w-32 sm:w-40" />
          <select v-model="minLevel" class="bg-slate-800 border border-white/5 rounded px-2 py-1 text-xs text-slate-400 focus:outline-none">
            <option value="ALL">ALL LEVELS</option>
            <option value="INFO">INFO</option>
            <option value="WARN">WARN</option>
            <option value="ERROR">ERROR</option>
            <option value="GHOST">GHOST</option>
          </select>
        </div>
      </div>
      <div class="flex items-center gap-3">
        <label class="hidden sm:flex items-center gap-2 cursor-pointer text-[10px] text-slate-500">
          <input type="checkbox" v-model="autoScroll" class="accent-blue-500" />
          AUTO-SCROLL
        </label>
        <button @click="clearLogs" class="text-slate-400 hover:text-white text-xs px-3 py-1 rounded bg-white/5 hover:bg-white/10 border border-white/5 transition-all">CLEAR</button>
        <button @click="toggleConnection" :class="wsConnected ? 'bg-red-500/20 text-red-400 hover:bg-red-500/30' : 'bg-emerald-500/20 text-emerald-400 hover:bg-emerald-500/30'" class="text-xs px-4 py-1 rounded border border-white/5 font-bold transition-all">
          {{ wsConnected ? 'SHUTDOWN' : 'ACTIVATE' }}
        </button>
      </div>
    </div>

    <!-- Terminal Output -->
    <div ref="terminalRef" class="flex-1 overflow-y-auto p-4 space-y-0.5 custom-scrollbar">
      <div v-for="(log, index) in filteredLogs" :key="index" class="text-[11px] leading-relaxed break-all animate-log-in">
        <span class="text-slate-600 mr-2">[{{ log.time }}]</span>
        <span :class="getLevelColor(log.level)" class="mr-2 px-1 rounded-sm text-[9px] font-bold">{{ log.level }}</span>
        <span :class="getContentColor(log.content)" class="whitespace-pre-wrap">{{ log.content }}</span>
      </div>
      <div v-if="logs.length === 0" class="h-full flex flex-col items-center justify-center opacity-20 select-none">
        <font-awesome-icon icon="terminal" class="text-6xl mb-4" />
        <p class="tracking-widest">AWAITING_STREAM_INPUT...</p>
      </div>
    </div>

    <!-- Footer Stats -->
    <div class="p-2 px-4 bg-slate-900 border-t border-white/5 flex justify-between text-[9px] text-slate-600">
      <div class="flex gap-4">
        <span>TOTAL_ENTRIES: {{ logs.length }}</span>
        <span>FILTERED: {{ filteredLogs.length }}</span>
      </div>
      <span>SYS_TIME: {{ currentTime }}</span>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted, watch, nextTick } from 'vue'

const logs = ref([])
const wsConnected = ref(false)
const filter = ref('')
const minLevel = ref('ALL')
const autoScroll = ref(true)
const terminalRef = ref(null)
const currentTime = ref(new Date().toLocaleTimeString())

let ws = null

const filteredLogs = computed(() => {
  return logs.value.filter(l => {
    const matchesFilter = filter.value ? l.content.toLowerCase().includes(filter.value.toLowerCase()) : true
    const matchesLevel = minLevel.value === 'ALL' || l.level === minLevel.value
    return matchesFilter && matchesLevel
  })
})

const getLevelColor = (level) => ({
  INFO: 'bg-blue-500/20 text-blue-400',
  WARN: 'bg-amber-500/20 text-amber-400',
  ERROR: 'bg-red-500/20 text-red-400',
  DEBUG: 'bg-purple-500/20 text-purple-400',
  GHOST: 'bg-white/10 text-white shadow-[0_0_10px_rgba(255,255,255,0.3)]',
  SYSTEM: 'bg-indigo-500/20 text-indigo-400'
}[level] || 'text-slate-400')

const getContentColor = (content) => {
  if (content.includes('SUCCESS') || content.includes('ESTABLISHED')) return 'text-emerald-400'
  if (content.includes('FAILED') || content.includes('ERROR') || content.includes('DISCONNECTED')) return 'text-red-400'
  if (content.includes('TRIGGERED') || content.includes('REBOOT')) return 'text-amber-400 font-bold underline'
  if (content.includes('GHOST_CMD')) return 'text-white italic'
  return 'text-slate-300'
}

const connectWS = () => {
  const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:'
  const host = window.location.host
  ws = new WebSocket(`${protocol}//${host}/api/ws/log`)

  ws.onopen = () => {
    wsConnected.value = true
    addLocalLog('SYSTEM', 'WS_LOG_STREAM_ESTABLISHED')
  }

  ws.onmessage = (event) => {
    try {
      const raw = event.data
      let level = 'INFO'
      let content = raw
      
      const match = raw.match(/^\[(.*?)\] (.*)/)
      if (match) {
        level = match[1]
        content = match[2]
      }
      
      logs.value.push({
        time: new Date().toLocaleTimeString(),
        level: level.toUpperCase(),
        content
      })
      
      if (logs.value.length > 200) logs.value.shift()
    } catch (e) {
      console.error('WS parse error', e)
    }
  }

  ws.onclose = () => {
    wsConnected.value = false
    addLocalLog('ERROR', 'WS_DISCONNECTED_RETRYING...')
  }
}

const toggleConnection = () => {
  if (wsConnected.value) {
    ws.close()
    wsConnected.value = false
  } else {
    connectWS()
  }
}

const addLocalLog = (level, content) => {
  logs.value.push({
    time: new Date().toLocaleTimeString(),
    level,
    content
  })
}

const clearLogs = () => {
  logs.value = []
}

watch(filteredLogs, () => {
  if (autoScroll.value) {
    nextTick(() => {
      if (terminalRef.value) {
        terminalRef.value.scrollTop = terminalRef.value.scrollHeight
      }
    })
  }
}, { deep: true })

let timeTimer = setInterval(() => {
  currentTime.value = new Date().toLocaleTimeString()
}, 1000)

onMounted(connectWS)
onUnmounted(() => {
  if (ws) ws.close()
  clearInterval(timeTimer)
})
</script>

<style scoped>
.custom-scrollbar::-webkit-scrollbar {
  width: 4px;
}
.custom-scrollbar::-webkit-scrollbar-track {
  background: transparent;
}
.custom-scrollbar::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.05);
  border-radius: 2px;
}
.custom-scrollbar::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.1);
}

.animate-log-in {
  animation: slide-in 0.15s ease-out;
}
@keyframes slide-in {
  from { opacity: 0; transform: translateX(-10px); }
  to { opacity: 1; transform: translateX(0); }
}
</style>
