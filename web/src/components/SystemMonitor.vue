<script setup>
import { inject, computed, ref, onMounted, onUnmounted, nextTick } from 'vue'
import { useI18n } from 'vue-i18n'
import { clearCache, getCurrentBand } from '../composables/useApi'
import { useToast } from '../composables/useToast'
import { useConfirm } from '../composables/useConfirm'

const { t } = useI18n()
const { success, error } = useToast()
const { confirm } = useConfirm()

const systemInfo = inject('systemInfo')
const loading = inject('loading')
const clearingCache = ref(false)

// IMEI/ICCID 显示控制
const showImei = ref(false)
const showIccid = ref(false)

// 数据脱敏函数
function maskValue(value, showFull) {
  if (!value || value === 'N/A') return 'N/A'
  if (showFull) return value
  if (value.length <= 8) return value
  return value.slice(0, 4) + '****' + value.slice(-4)
}

// 当前连接频段信息
const currentBand = ref(null)
const bandLoading = ref(false)

// 获取当前频段信息
async function fetchCurrentBand() {
  if (bandLoading.value) return // 防止重复请求
  bandLoading.value = true
  try {
    const res = await getCurrentBand()
    if (res && res.Code === 0 && res.Data) {
      currentBand.value = res.Data
    } else if (res && res.data) {
      // 兼容不同的响应格式
      currentBand.value = res.data
    }
  } catch (err) {
    console.error('获取频段信息失败:', err)
  }
  bandLoading.value = false
}

// 定时刷新频段信息
let bandTimer = null
onMounted(async () => {
  await nextTick()
  fetchCurrentBand()
  bandTimer = setInterval(fetchCurrentBand, 10000)
})

onUnmounted(() => {
  if (bandTimer) clearInterval(bandTimer)
})

// 信号强度等级计算（返回1-4）
function getSignalLevel(rsrp) {
  if (!rsrp || rsrp === 0) return 0
  if (rsrp >= -80) return 4   // 优秀
  if (rsrp >= -90) return 3   // 良好  
  if (rsrp >= -100) return 2  // 一般
  return 1                     // 差
}

// 信号强度颜色
function getSignalColor(rsrp) {
  if (!rsrp || rsrp === 0) return { text: 'text-gray-400', bg: 'bg-gray-400' }
  if (rsrp >= -80) return { text: 'text-green-400', bg: 'bg-green-400' }
  if (rsrp >= -90) return { text: 'text-yellow-400', bg: 'bg-yellow-400' }
  if (rsrp >= -100) return { text: 'text-orange-400', bg: 'bg-orange-400' }
  return { text: 'text-red-400', bg: 'bg-red-400' }
}

// 判断是否有网络连接（通过RSRP是否有有效值来判断）
const hasNetwork = computed(() => {
  return currentBand.value && currentBand.value.rsrp && currentBand.value.rsrp !== 0
})

// 格式化频段值（无网络时显示N/A）
function formatBandValue(value) {
  if (!hasNetwork.value) return 'N/A'
  return value ?? 'N/A'
}

// 格式化系统信息值（无网络时显示N/A，用于Modem信息板块）
function formatSystemValue(value) {
  if (!hasNetwork.value) return 'N/A'
  return value ?? 'N/A'
}

// 计算运行时间
const uptime = computed(() => {
  const s = systemInfo.value?.uptime || 0
  const days = Math.floor(s / 86400)
  const hours = Math.floor((s % 86400) / 3600)
  const minutes = Math.floor((s % 3600) / 60)
  let str = ''
  if (days > 0) str += t('common.days', { n: days }) + ' '
  if (hours > 0 || days > 0) str += t('common.hours', { n: hours }) + ' '
  str += t('common.minutes', { n: minutes })
  return str
})

// 计算内存使用率
const memoryPercent = computed(() => {
  const total = systemInfo.value?.total_ram || 1
  const free = systemInfo.value?.free_ram || 0
  return Math.round(((total - free) / total) * 100)
})

const usedMemory = computed(() => {
  const total = systemInfo.value?.total_ram || 0
  const free = systemInfo.value?.free_ram || 0
  return total - free
})

// 计算CPU使用率
const cpuPercent = computed(() => {
  const cpu = systemInfo.value?.cpu_usage || 0
  return Math.round(cpu * 10) / 10
})

// 格式化速率
function formatRate(kbps) {
  if (!kbps || kbps === 0) return 'N/A'
  if (kbps >= 1000000) return (kbps / 1000000).toFixed(1) + ' Gbps'
  if (kbps >= 1000) return (kbps / 1000).toFixed(0) + ' Mbps'
  return kbps + ' kbps'
}

// 清除缓存
async function handleClearCache() {
  if (clearingCache.value) return
  if (!await confirm({ title: t('monitor.clearCache'), message: t('monitor.confirmClearCache') })) return
  clearingCache.value = true
  try {
    const result = await clearCache()
    if (result.status === 'success') success(t('monitor.cacheCleared'))
    else throw new Error(result.message || t('monitor.cacheClearFailed'))
  } catch (err) {
    error(t('monitor.cacheClearFailed') + ': ' + err.message)
  } finally {
    clearingCache.value = false
  }
}
</script>

<template>
  <div class="space-y-6">
    <!-- 当前连接频段 - 炫酷卡片 -->
    <div class="relative overflow-hidden rounded-3xl bg-gradient-to-br from-indigo-50/80 via-purple-50/60 to-pink-50/40 dark:from-indigo-600/20 dark:via-purple-600/20 dark:to-pink-600/20 border border-slate-200/60 dark:border-white/10 p-6 shadow-xl shadow-indigo-100/50 dark:shadow-black/20 hover:shadow-2xl hover:shadow-indigo-200/60 dark:hover:shadow-black/30 transition-all duration-500">
      <!-- 动态背景装饰 -->
      <div class="absolute top-0 right-0 w-96 h-96 bg-purple-500/10 rounded-full blur-3xl -translate-y-1/2 translate-x-1/2 animate-pulse"></div>
      <div class="absolute bottom-0 left-0 w-64 h-64 bg-cyan-500/10 rounded-full blur-3xl translate-y-1/2 -translate-x-1/2"></div>
      
      <div class="relative">
        <!-- 动态波浪背景 (SVG 动画) -->
        <div class="absolute inset-0 -m-6 opacity-30 pointer-events-none overflow-hidden rounded-3xl">
          <svg class="absolute bottom-0 left-0 w-[200%] h-32 text-indigo-400/20 fill-current animate-wave-slow" viewBox="0 0 1200 120" preserveAspectRatio="none">
            <path d="M0,60 C150,110 300,10 450,60 C600,110 750,10 900,60 C1050,110 1200,10 1350,60 L1350,120 L0,120 Z"></path>
          </svg>
          <svg class="absolute bottom-0 left-0 w-[200%] h-24 text-purple-400/30 fill-current animate-wave-fast" viewBox="0 0 1200 120" preserveAspectRatio="none">
            <path d="M0,50 C150,0 300,100 450,50 C600,0 750,100 900,50 C1050,0 1200,100 1350,50 L1350,120 L0,120 Z" opacity="0.5"></path>
          </svg>
        </div>

        <div class="flex items-center justify-between mb-6">
          <div class="flex items-center space-x-3">
            <div class="w-12 h-12 rounded-2xl bg-gradient-to-br from-purple-500 to-pink-500 flex items-center justify-center shadow-lg shadow-purple-500/30">
              <i class="fas fa-satellite-dish text-white text-xl"></i>
            </div>
            <div>
              <h3 class="text-slate-800 dark:text-white font-bold text-sm sm:text-lg">{{ t('monitor.title') }}</h3>
              <p class="text-slate-600 dark:text-white/50 text-sm">{{ t('monitor.subtitle') }}</p>
            </div>
          </div>
          <div class="flex items-center space-x-4">
            <!-- RSRP信号强度指示 -->
            <div v-if="currentBand" class="flex items-center space-x-2 px-3 py-2 bg-white/80 dark:bg-white/5 rounded-xl border border-slate-200 dark:border-white/10">
              <div class="flex items-end space-x-0.5 h-5">
                <!-- 4格信号：根据level决定高亮数量 -->
                <div class="w-1.5 h-2 rounded-full" :class="getSignalLevel(currentBand.rsrp) >= 1 ? getSignalColor(currentBand.rsrp).bg : 'bg-gray-300 dark:bg-gray-600'"></div>
                <div class="w-1.5 h-3 rounded-full" :class="getSignalLevel(currentBand.rsrp) >= 2 ? getSignalColor(currentBand.rsrp).bg : 'bg-gray-300 dark:bg-gray-600'"></div>
                <div class="w-1.5 h-4 rounded-full" :class="getSignalLevel(currentBand.rsrp) >= 3 ? getSignalColor(currentBand.rsrp).bg : 'bg-gray-300 dark:bg-gray-600'"></div>
                <div class="w-1.5 h-5 rounded-full" :class="getSignalLevel(currentBand.rsrp) >= 4 ? getSignalColor(currentBand.rsrp).bg : 'bg-gray-300 dark:bg-gray-600'"></div>
              </div>
              <span class="text-sm font-medium" :class="getSignalColor(currentBand.rsrp).text">
                {{ currentBand.rsrp ? currentBand.rsrp.toFixed(0) + ' dBm' : 'N/A' }}
              </span>
            </div>
            <!-- 刷新按钮 -->
            <button @click="fetchCurrentBand" :disabled="bandLoading" 
              class="group px-4 py-2 bg-white/80 dark:bg-white/10 backdrop-blur text-slate-700 dark:text-white/80 text-sm rounded-xl hover:bg-white dark:hover:bg-white/20 transition-all border border-slate-300 dark:border-white/10">
              <font-awesome-icon v-if="bandLoading" icon="spinner" spin class="mr-2" />
              <font-awesome-icon v-else icon="sync-alt" class="mr-2 group-hover:rotate-180 transition-transform duration-500" />
              {{ t('common.refresh') }}
            </button>
          </div>
        </div>
  
      <!-- 频段信息网格：移动端2列，平板4列，桌面7列 -->
      <div v-if="currentBand" class="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-7 gap-1.5 sm:gap-2 md:gap-3">
          <!-- 网络类型 -->
          <div class="group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-white/80 dark:bg-white/5 backdrop-blur rounded-xl sm:rounded-2xl border border-slate-200 dark:border-white/10 hover:border-blue-500/50 transition-all hover:scale-105">
            <div class="absolute inset-0 bg-gradient-to-br from-blue-500/0 to-cyan-500/0 group-hover:from-blue-500/10 group-hover:to-cyan-500/10 transition-all"></div>
            <div class="relative text-center">
              <div class="w-8 h-8 sm:w-10 sm:h-10 md:w-12 md:h-12 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-lg sm:rounded-xl bg-gradient-to-br from-blue-500 to-cyan-400 flex items-center justify-center shadow-lg shadow-blue-500/30">
                <i class="fas fa-tower-cell text-white text-xs sm:text-sm md:text-lg"></i>
              </div>
              <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.networkType') }}</p>
              <p class="text-slate-800 dark:text-white font-bold text-sm sm:text-base md:text-lg">
                {{ formatBandValue(currentBand.network_type) }}
                <span v-if="currentBand?.network_type?.includes('5G')">🚀</span>
                <span v-else-if="currentBand?.network_type?.includes('4G') || currentBand?.network_type?.includes('LTE')">🚗</span>
                <span v-else-if="currentBand?.network_type">🚲</span>
              </p>
            </div>
          </div>
          
          <!-- 频段 -->
          <div class="group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-white/80 dark:bg-white/5 backdrop-blur rounded-xl sm:rounded-2xl border border-slate-200 dark:border-white/10 hover:border-purple-500/50 transition-all hover:scale-105">
            <div class="absolute inset-0 bg-gradient-to-br from-purple-500/0 to-pink-500/0 group-hover:from-purple-500/10 group-hover:to-pink-500/10 transition-all"></div>
            <div class="relative text-center">
              <div class="w-8 h-8 sm:w-10 sm:h-10 md:w-12 md:h-12 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-lg sm:rounded-xl bg-gradient-to-br from-purple-500 to-pink-400 flex items-center justify-center shadow-lg shadow-purple-500/30">
                <i class="fas fa-broadcast-tower text-white text-xs sm:text-sm md:text-lg"></i>
              </div>
              <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.band') }}</p>
              <p class="text-purple-700 dark:text-purple-400 font-bold text-sm sm:text-base md:text-lg">{{ formatBandValue(currentBand.band) }}</p>
            </div>
          </div>
          
          <!-- 频点 -->
          <div class="group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-white/80 dark:bg-white/5 backdrop-blur rounded-xl sm:rounded-2xl border border-slate-200 dark:border-white/10 hover:border-amber-500/50 transition-all hover:scale-105">
            <div class="absolute inset-0 bg-gradient-to-br from-amber-500/0 to-orange-500/0 group-hover:from-amber-500/10 group-hover:to-orange-500/10 transition-all"></div>
            <div class="relative text-center">
              <div class="w-8 h-8 sm:w-10 sm:h-10 md:w-12 md:h-12 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-lg sm:rounded-xl bg-gradient-to-br from-amber-500 to-orange-400 flex items-center justify-center shadow-lg shadow-amber-500/30">
                <i class="fas fa-wave-square text-white text-xs sm:text-sm md:text-lg"></i>
              </div>
              <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.arfcn') }}</p>
              <p class="text-slate-800 dark:text-white font-mono font-bold text-sm sm:text-base md:text-lg">{{ formatBandValue(currentBand.arfcn) }}</p>
            </div>
          </div>
          
          <!-- PCI -->
          <div class="group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-white/80 dark:bg-white/5 backdrop-blur rounded-xl sm:rounded-2xl border border-slate-200 dark:border-white/10 hover:border-green-500/50 transition-all hover:scale-105">
            <div class="absolute inset-0 bg-gradient-to-br from-green-500/0 to-emerald-500/0 group-hover:from-green-500/10 group-hover:to-emerald-500/10 transition-all"></div>
            <div class="relative text-center">
              <div class="w-8 h-8 sm:w-10 sm:h-10 md:w-12 md:h-12 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-lg sm:rounded-xl bg-gradient-to-br from-green-500 to-emerald-400 flex items-center justify-center shadow-lg shadow-green-500/30">
                <i class="fas fa-hashtag text-white text-xs sm:text-sm md:text-lg"></i>
              </div>
              <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.pci') }}</p>
              <p class="text-slate-800 dark:text-white font-bold text-sm sm:text-base md:text-lg">{{ formatBandValue(currentBand.pci) }}</p>
            </div>
          </div>

          <!-- RSRP - 竖屏隐藏，横屏显示 -->
          <div class="hidden landscape:block group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-white/80 dark:bg-white/5 backdrop-blur rounded-xl sm:rounded-2xl border border-slate-200 dark:border-white/10 hover:border-rose-500/50 transition-all hover:scale-105">
            <div class="absolute inset-0 bg-gradient-to-br from-rose-500/0 to-pink-500/0 group-hover:from-rose-500/10 group-hover:to-pink-500/10 transition-all"></div>
            <div class="relative text-center">
              <div class="w-8 h-8 sm:w-10 sm:h-10 md:w-12 md:h-12 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-lg sm:rounded-xl bg-gradient-to-br from-rose-500 to-pink-400 flex items-center justify-center shadow-lg shadow-rose-500/30">
                <i class="fas fa-signal text-white text-xs sm:text-sm md:text-lg"></i>
              </div>
              <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.rsrp') }}</p>
              <p class="text-rose-700 dark:text-rose-400 font-bold text-sm sm:text-base md:text-lg">
                {{ currentBand.rsrp ? currentBand.rsrp.toFixed(1) + ' dBm' : 'N/A' }}
              </p>
            </div>
          </div>
          
          <!-- RSRQ -->
          <div class="group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-white/80 dark:bg-white/5 backdrop-blur rounded-xl sm:rounded-2xl border border-slate-200 dark:border-white/10 hover:border-cyan-500/50 transition-all hover:scale-105">
            <div class="absolute inset-0 bg-gradient-to-br from-cyan-500/0 to-blue-500/0 group-hover:from-cyan-500/10 group-hover:to-blue-500/10 transition-all"></div>
            <div class="relative text-center">
              <div class="w-8 h-8 sm:w-10 sm:h-10 md:w-12 md:h-12 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-lg sm:rounded-xl bg-gradient-to-br from-cyan-500 to-blue-400 flex items-center justify-center shadow-lg shadow-cyan-500/30">
                <i class="fas fa-chart-line text-white text-xs sm:text-sm md:text-lg"></i>
              </div>
              <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.rsrq') }}</p>
              <p class="text-cyan-700 dark:text-cyan-400 font-bold text-sm sm:text-base md:text-lg">{{ currentBand.rsrq ? currentBand.rsrq.toFixed(1) + ' dB' : 'N/A' }}</p>
            </div>
          </div>
          
          <!-- SINR -->
          <div class="group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-white/80 dark:bg-white/5 backdrop-blur rounded-xl sm:rounded-2xl border border-slate-200 dark:border-white/10 hover:border-indigo-500/50 transition-all hover:scale-105">
            <div class="absolute inset-0 bg-gradient-to-br from-indigo-500/0 to-purple-500/0 group-hover:from-indigo-500/10 group-hover:to-purple-500/10 transition-all"></div>
            <div class="relative text-center">
              <div class="w-8 h-8 sm:w-10 sm:h-10 md:w-12 md:h-12 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-lg sm:rounded-xl bg-gradient-to-br from-indigo-500 to-purple-400 flex items-center justify-center shadow-lg shadow-indigo-500/30">
                <i class="fas fa-chart-area text-white text-xs sm:text-sm md:text-lg"></i>
              </div>
              <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.sinr') }}</p>
              <p class="text-indigo-700 dark:text-indigo-400 font-bold text-sm sm:text-base md:text-lg">{{ currentBand.sinr ? currentBand.sinr.toFixed(1) + ' dB' : 'N/A' }}</p>
            </div>
          </div>
        </div>
        
        <!-- 加载状态 -->
        <div v-else-if="bandLoading" class="text-center py-12">
          <div class="relative inline-block">
            <div class="w-20 h-20 rounded-full border-4 border-purple-500/30 border-t-purple-500 animate-spin"></div>
            <div class="absolute inset-0 flex items-center justify-center">
              <i class="fas fa-satellite-dish text-purple-500 dark:text-purple-400 text-2xl"></i>
            </div>
          </div>
          <p class="text-slate-600 dark:text-white/50 mt-4">{{ t('monitor.loadingBand') }}</p>
        </div>
        <!-- 无数据状态 -->
        <div v-else class="text-center py-12">
          <div class="w-16 h-16 mx-auto mb-4 rounded-2xl bg-slate-100 dark:bg-white/10 flex items-center justify-center">
            <i class="fas fa-satellite-dish text-slate-400 dark:text-white/40 text-2xl"></i>
          </div>
          <p class="text-slate-500 dark:text-white/50">{{ t('monitor.noBandInfo') }}</p>
          <button @click="fetchCurrentBand" class="mt-3 px-4 py-2 bg-purple-500/20 text-purple-600 dark:text-purple-400 rounded-xl hover:bg-purple-500/30 transition-all text-sm">
            <i class="fas fa-sync-alt mr-2"></i>{{ t('monitor.retryFetch') }}
          </button>
        </div>
      </div>
    </div>

    <!-- 顶部状态卡片 - 带动画，移动端优化 -->
    <div class="grid grid-cols-2 md:grid-cols-4 gap-2 sm:gap-3 md:gap-4">
      <div class="group relative overflow-hidden rounded-xl sm:rounded-2xl bg-gradient-to-br from-blue-50/90 to-cyan-50/70 dark:from-blue-500/20 dark:to-cyan-500/20 border border-slate-200/60 dark:border-white/10 p-3 sm:p-4 md:p-5 shadow-lg shadow-blue-100/40 dark:shadow-black/10 hover:shadow-xl hover:shadow-blue-200/50 dark:hover:shadow-black/20 hover:-translate-y-1 transition-all duration-300">
        <div class="absolute -top-4 -right-4 w-16 h-16 bg-blue-500/20 rounded-full blur-xl group-hover:scale-150 transition-transform"></div>
        <div class="relative flex items-center space-x-2 sm:space-x-3 md:space-x-4">
          <div class="w-10 h-10 sm:w-11 sm:h-11 md:w-12 md:h-12 rounded-lg sm:rounded-xl bg-gradient-to-br from-blue-500 to-cyan-400 flex items-center justify-center shadow-lg shadow-blue-500/30 flex-shrink-0">
            <i class="fas fa-server text-white text-sm sm:text-base"></i>
          </div>
          <div class="min-w-0 flex-1">
            <p class="text-slate-600 dark:text-white/50 text-xs">{{ t('monitor.hostname') }}</p>
            <p class="text-slate-800 dark:text-white font-bold text-sm sm:text-base truncate">{{ systemInfo?.hostname || '...' }}</p>
          </div>
        </div>
      </div>
      
      <div class="group relative overflow-hidden rounded-xl sm:rounded-2xl bg-gradient-to-br from-purple-50/90 to-pink-50/70 dark:from-purple-500/20 dark:to-pink-500/20 border border-slate-200/60 dark:border-white/10 p-3 sm:p-4 md:p-5 shadow-lg shadow-purple-100/40 dark:shadow-black/10 hover:shadow-xl hover:shadow-purple-200/50 dark:hover:shadow-black/20 hover:-translate-y-1 transition-all duration-300">
        <div class="absolute -top-4 -right-4 w-16 h-16 bg-purple-500/20 rounded-full blur-xl group-hover:scale-150 transition-transform"></div>
        <div class="relative flex items-center space-x-2 sm:space-x-3 md:space-x-4">
          <div class="w-10 h-10 sm:w-11 sm:h-11 md:w-12 md:h-12 rounded-lg sm:rounded-xl bg-gradient-to-br from-purple-500 to-pink-400 flex items-center justify-center shadow-lg shadow-purple-500/30 flex-shrink-0">
            <i class="fas fa-microchip text-white text-sm sm:text-base"></i>
          </div>
          <div class="min-w-0 flex-1">
            <p class="text-slate-600 dark:text-white/50 text-xs">{{ t('monitor.architecture') }}</p>
            <p class="text-slate-800 dark:text-white font-bold text-sm sm:text-base">{{ systemInfo?.machine || '...' }}</p>
          </div>
        </div>
      </div>
      
      <div class="group relative overflow-hidden rounded-xl sm:rounded-2xl bg-gradient-to-br from-green-50/90 to-emerald-50/70 dark:from-green-500/20 dark:to-emerald-500/20 border border-slate-200/60 dark:border-white/10 p-3 sm:p-4 md:p-5 shadow-lg shadow-green-100/40 dark:shadow-black/10 hover:shadow-xl hover:shadow-green-200/50 dark:hover:shadow-black/20 hover:-translate-y-1 transition-all duration-300">
        <div class="absolute -top-4 -right-4 w-16 h-16 bg-green-500/20 rounded-full blur-xl group-hover:scale-150 transition-transform"></div>
        <div class="relative flex items-center space-x-2 sm:space-x-3 md:space-x-4">
          <div class="w-10 h-10 sm:w-11 sm:h-11 md:w-12 md:h-12 rounded-lg sm:rounded-xl bg-gradient-to-br from-green-500 to-emerald-400 flex items-center justify-center shadow-lg shadow-green-500/30 flex-shrink-0">
            <i class="fas fa-clock text-white text-sm sm:text-base"></i>
          </div>
          <div class="min-w-0 flex-1">
            <p class="text-slate-600 dark:text-white/50 text-xs">{{ t('monitor.uptime') }}</p>
            <p class="text-slate-800 dark:text-white font-bold text-xs sm:text-sm truncate">{{ uptime || '...' }}</p>
          </div>
        </div>
      </div>
      
      <div class="group relative overflow-hidden rounded-xl sm:rounded-2xl bg-gradient-to-br from-orange-50/90 to-amber-50/70 dark:from-orange-500/20 dark:to-amber-500/20 border border-slate-200/60 dark:border-white/10 p-3 sm:p-4 md:p-5 shadow-lg shadow-orange-100/40 dark:shadow-black/10 hover:shadow-xl hover:shadow-orange-200/50 dark:hover:shadow-black/20 hover:-translate-y-1 transition-all duration-300">
        <div class="absolute -top-4 -right-4 w-16 h-16 bg-orange-500/20 rounded-full blur-xl group-hover:scale-150 transition-transform"></div>
        <div class="relative flex items-center space-x-2 sm:space-x-3 md:space-x-4">
          <div class="w-10 h-10 sm:w-11 sm:h-11 md:w-12 md:h-12 rounded-lg sm:rounded-xl bg-gradient-to-br from-orange-500 to-amber-400 flex items-center justify-center shadow-lg shadow-orange-500/30 flex-shrink-0">
            <i class="fas fa-temperature-half text-white text-sm sm:text-base"></i>
          </div>
          <div class="min-w-0 flex-1">
            <p class="text-slate-600 dark:text-white/50 text-xs">{{ t('monitor.temperature') }}</p>
            <p class="text-slate-800 dark:text-white font-bold text-sm sm:text-base">{{ systemInfo?.thermal_temp || '...' }}</p>
          </div>
        </div>
      </div>
    </div>


    <!-- 主要信息区 -->
    <div class="grid grid-cols-1 lg:grid-cols-2 gap-6">
      <!-- 内存使用 - 炫酷环形图 -->
      <div class="relative overflow-hidden rounded-3xl bg-white/95 dark:bg-white/5 backdrop-blur-xl border border-slate-200/60 dark:border-white/10 p-6 shadow-xl shadow-slate-200/40 dark:shadow-black/20 hover:shadow-2xl hover:shadow-slate-300/50 dark:hover:shadow-black/30 transition-all duration-500">
        <div class="absolute top-0 right-0 w-40 h-40 bg-cyan-500/10 rounded-full blur-3xl"></div>
        
        <div class="relative">
          <div class="flex items-center justify-between mb-6">
            <div class="flex items-center space-x-3">
              <div class="w-10 h-10 rounded-xl bg-gradient-to-br from-cyan-500 to-blue-500 flex items-center justify-center">
                <i class="fas fa-memory text-white"></i>
              </div>
              <h3 class="text-slate-900 dark:text-white font-bold">{{ t('monitor.memoryUsage') }}</h3>
            </div>
            <button @click="handleClearCache" :disabled="clearingCache"
              class="px-4 py-2 bg-gradient-to-r from-yellow-500/20 to-amber-500/20 text-yellow-600 dark:text-yellow-400 text-sm rounded-xl hover:from-yellow-500/30 hover:to-amber-500/30 transition-all border border-yellow-500/30">
              <i :class="clearingCache ? 'fas fa-spinner animate-spin' : 'fas fa-broom'" class="mr-2"></i>
              {{ clearingCache ? t('monitor.clearing') : t('monitor.clearCache') }}
            </button>
          </div>
          
          <div class="flex flex-row items-center justify-center gap-3 sm:gap-6 md:gap-8">
            <!-- 环形进度 -->
            <div class="relative w-20 h-20 sm:w-28 sm:h-28 md:w-36 md:h-36 flex-shrink-0 flex items-center justify-center">
              <svg class="w-full h-full transform -rotate-90" viewBox="0 0 144 144">
                <circle cx="72" cy="72" r="60" stroke="currentColor" stroke-width="14" fill="none" class="text-white/10"/>
                <circle cx="72" cy="72" r="60" stroke="url(#memGrad)" stroke-width="14" fill="none" stroke-linecap="round"
                  :stroke-dasharray="377" :stroke-dashoffset="377 * (1 - memoryPercent / 100)" class="transition-all duration-1000"/>
                <defs>
                  <linearGradient id="memGrad" x1="0%" y1="0%" x2="100%" y2="0%">
                    <stop offset="0%" stop-color="#06b6d4"/><stop offset="50%" stop-color="#3b82f6"/><stop offset="100%" stop-color="#8b5cf6"/>
                  </linearGradient>
                </defs>
              </svg>
              <div class="absolute inset-0 flex flex-col items-center justify-center">
                <span class="text-slate-900 dark:text-white text-base sm:text-2xl md:text-3xl font-bold">{{ memoryPercent }}%</span>
                <span class="text-slate-500 dark:text-white/50 text-[8px] sm:text-xs">{{ t('monitor.used') }}</span>
              </div>
            </div>
            
            <div class="flex-1 space-y-1.5 sm:space-y-3">
              <div class="p-2 sm:p-3 bg-gradient-to-r from-slate-100 dark:from-white/5 to-transparent rounded-lg sm:rounded-xl border-l-4 border-cyan-500">
                <div class="flex justify-between"><span class="text-slate-600 dark:text-white/60 text-xs sm:text-sm">{{ t('monitor.used') }}</span><span class="text-slate-900 dark:text-white font-bold text-xs sm:text-base">{{ usedMemory }} MB</span></div>
              </div>
              <div class="p-2 sm:p-3 bg-gradient-to-r from-slate-100 dark:from-white/5 to-transparent rounded-lg sm:rounded-xl border-l-4 border-green-500">
                <div class="flex justify-between"><span class="text-slate-600 dark:text-white/60 text-xs sm:text-sm">{{ t('monitor.available') }}</span><span class="text-green-600 dark:text-green-400 font-bold text-xs sm:text-base">{{ systemInfo?.free_ram || 0 }} MB</span></div>
              </div>
              <div class="p-2 sm:p-3 bg-gradient-to-r from-slate-100 dark:from-white/5 to-transparent rounded-lg sm:rounded-xl border-l-4 border-yellow-500">
                <div class="flex justify-between"><span class="text-slate-600 dark:text-white/60 text-xs sm:text-sm">{{ t('monitor.cached') }}</span><span class="text-yellow-600 dark:text-yellow-400 font-bold text-xs sm:text-base">{{ systemInfo?.cached_ram || 0 }} MB</span></div>
              </div>
            </div>
          </div>
          
          <!-- CPU使用率 -->
          <div class="mt-6 pt-6 border-t border-slate-200 dark:border-white/10">
            <div class="flex items-center justify-between mb-4">
              <div class="flex items-center space-x-3">
                <div class="w-8 h-8 rounded-lg bg-gradient-to-br from-orange-500 to-red-500 flex items-center justify-center">
                  <i class="fas fa-microchip text-white text-sm"></i>
                </div>
                <span class="text-slate-900 dark:text-white font-medium">{{ t('monitor.cpuUsage') }}</span>
              </div>
              <span class="text-orange-600 dark:text-orange-400 font-bold text-xl">{{ cpuPercent }}%</span>
            </div>
            <div class="relative h-3 bg-slate-200 dark:bg-white/10 rounded-full overflow-hidden">
              <div class="absolute inset-y-0 left-0 bg-gradient-to-r from-orange-500 via-red-500 to-pink-500 rounded-full transition-all duration-1000"
                :style="{ width: cpuPercent + '%' }">
                <div class="absolute inset-0 bg-gradient-to-r from-white/0 via-white/30 to-white/0 animate-pulse"></div>
              </div>
            </div>
            <div class="flex justify-between text-xs text-slate-400 dark:text-white/40 mt-2">
              <span>0%</span>
              <span>50%</span>
              <span>100%</span>
            </div>
          </div>
        </div>
      </div>


      <!-- Modem信息 -->
      <div class="relative overflow-hidden rounded-3xl bg-white/95 dark:bg-white/5 backdrop-blur-xl border border-slate-200/60 dark:border-white/10 p-6 shadow-xl shadow-slate-200/40 dark:shadow-black/20 hover:shadow-2xl hover:shadow-slate-300/50 dark:hover:shadow-black/30 transition-all duration-500">
        <div class="absolute bottom-0 left-0 w-40 h-40 bg-purple-500/10 rounded-full blur-3xl"></div>
        
        <div class="relative">
          <div class="flex items-center space-x-3 mb-6">
            <div class="w-10 h-10 rounded-xl bg-gradient-to-br from-purple-500 to-pink-500 flex items-center justify-center">
              <i class="fas fa-broadcast-tower text-white"></i>
            </div>
            <h3 class="text-slate-900 dark:text-white font-bold">{{ t('monitor.modemInfo') }}</h3>
          </div>
          
          <div class="space-y-3">
            <div class="group p-3 bg-slate-100 dark:bg-white/5 rounded-xl hover:bg-slate-200 dark:hover:bg-white/10 transition-all flex justify-between items-center">
              <span class="text-slate-600 dark:text-white/60 text-sm"><i class="fas fa-building mr-2 text-purple-400"></i>{{ t('monitor.carrier') }}</span>
              <span class="text-slate-900 dark:text-white font-bold">{{ systemInfo?.carrier || 'N/A' }}</span>
            </div>
            <div class="group p-3 bg-slate-100 dark:bg-white/5 rounded-xl hover:bg-slate-200 dark:hover:bg-white/10 transition-all flex justify-between items-center">
              <span class="text-slate-600 dark:text-white/60 text-sm"><i class="fas fa-signal mr-2 text-cyan-400"></i>{{ t('monitor.signalStrength') }}</span>
              <div class="flex items-center space-x-2">
                <div class="flex space-x-0.5 items-end">
                  <div class="w-1.5 h-2 rounded-full" :class="systemInfo?.signal_strength ? 'bg-green-400' : 'bg-slate-300 dark:bg-white/20'"></div>
                  <div class="w-1.5 h-3 rounded-full" :class="systemInfo?.signal_strength ? 'bg-green-400' : 'bg-slate-300 dark:bg-white/20'"></div>
                  <div class="w-1.5 h-4 rounded-full" :class="systemInfo?.signal_strength ? 'bg-green-400' : 'bg-slate-300 dark:bg-white/20'"></div>
                  <div class="w-1.5 h-5 rounded-full" :class="systemInfo?.signal_strength ? 'bg-green-400' : 'bg-slate-300 dark:bg-white/20'"></div>
                </div>
                <span class="text-green-600 dark:text-green-400 font-bold">{{ systemInfo?.signal_strength ?? 'N/A' }}</span>
              </div>
            </div>
            <div class="group p-3 bg-slate-100 dark:bg-white/5 rounded-xl hover:bg-slate-200 dark:hover:bg-white/10 transition-all flex justify-between items-center">
              <span class="text-slate-600 dark:text-white/60 text-sm"><i class="fas fa-gauge-high mr-2 text-amber-400"></i>{{ t('monitor.qci') }}</span>
              <span class="text-amber-600 dark:text-amber-400 font-bold">{{ formatSystemValue(systemInfo?.qci) }}</span>
            </div>
            <div class="p-3 bg-gradient-to-r from-green-500/10 via-transparent to-blue-500/10 rounded-xl border border-slate-200 dark:border-white/10">
              <div class="flex justify-between items-center">
                <span class="text-slate-600 dark:text-white/60 text-sm"><i class="fas fa-gauge mr-2 text-slate-400 dark:text-white/40"></i>{{ t('monitor.signedRate') }}</span>
                <div class="flex items-center space-x-4">
                  <span class="text-green-600 dark:text-green-400 font-bold"><i class="fas fa-arrow-down text-xs mr-1"></i>{{ formatRate(systemInfo?.downlink_rate) }}</span>
                  <span class="text-blue-600 dark:text-blue-400 font-bold"><i class="fas fa-arrow-up text-xs mr-1"></i>{{ formatRate(systemInfo?.uplink_rate) }}</span>
                </div>
              </div>
            </div>
            <div @click="showImei = !showImei" class="group p-3 bg-slate-100 dark:bg-white/5 rounded-xl hover:bg-slate-200 dark:hover:bg-white/10 transition-all flex justify-between items-center cursor-pointer select-none">
              <span class="text-slate-600 dark:text-white/60 text-sm"><i class="fas fa-fingerprint mr-2 text-blue-400"></i>{{ t('monitor.imei') }}</span>
              <div class="flex items-center space-x-2">
                <span class="text-slate-900 dark:text-white font-mono text-sm transition-all duration-300">{{ maskValue(systemInfo?.imei, showImei) }}</span>
                <font-awesome-icon :icon="showImei ? 'eye' : 'eye-slash'" :class="showImei ? 'text-blue-400' : 'text-slate-400'" class="text-xs transition-all duration-300" />
              </div>
            </div>
            <div @click="showIccid = !showIccid" class="group p-3 bg-slate-100 dark:bg-white/5 rounded-xl hover:bg-slate-200 dark:hover:bg-white/10 transition-all flex justify-between items-center cursor-pointer select-none">
              <span class="text-slate-600 dark:text-white/60 text-sm"><i class="fas fa-sim-card mr-2 text-green-400"></i>{{ t('monitor.iccid') }}</span>
              <div class="flex items-center space-x-2">
                <span class="text-slate-900 dark:text-white font-mono text-sm transition-all duration-300">{{ maskValue(systemInfo?.iccid, showIccid) }}</span>
                <font-awesome-icon :icon="showIccid ? 'eye' : 'eye-slash'" :class="showIccid ? 'text-green-400' : 'text-slate-400'" class="text-xs transition-all duration-300" />
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- 系统状态 - 卡片网格 -->
    <div class="rounded-3xl bg-white/95 dark:bg-white/5 backdrop-blur-xl border border-slate-200/60 dark:border-white/10 p-6 shadow-xl shadow-slate-200/40 dark:shadow-black/20 hover:shadow-2xl hover:shadow-slate-300/50 dark:hover:shadow-black/30 transition-all duration-500">
      <div class="flex items-center space-x-3 mb-6">
        <div class="w-10 h-10 rounded-xl bg-gradient-to-br from-red-500 to-pink-500 flex items-center justify-center">
          <i class="fas fa-heartbeat text-white"></i>
        </div>
        <h3 class="text-slate-800 dark:text-white font-bold">{{ t('monitor.systemStatus') }}</h3>
      </div>
      
      <div class="grid grid-cols-2 md:grid-cols-4 gap-1.5 sm:gap-2 md:gap-3">
        <div class="group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-gradient-to-br from-orange-50 to-amber-50 dark:from-orange-500/10 dark:to-amber-500/10 rounded-xl sm:rounded-2xl border border-orange-200 dark:border-orange-500/20 hover:border-orange-500/50 transition-all hover:scale-105">
          <div class="absolute -top-2 -right-2 w-12 h-12 bg-orange-500/20 rounded-full blur-xl group-hover:scale-150 transition-transform"></div>
          <div class="relative text-center">
            <div class="w-10 h-10 sm:w-12 sm:h-12 md:w-14 md:h-14 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-xl sm:rounded-2xl bg-gradient-to-br from-orange-500 to-amber-400 flex items-center justify-center shadow-lg shadow-orange-500/30">
              <i class="fas fa-temperature-high text-white text-sm sm:text-base md:text-xl"></i>
            </div>
            <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.temperature') }}</p>
            <p class="text-slate-800 dark:text-white font-bold text-base sm:text-lg md:text-xl">
              {{ systemInfo?.thermal_temp || '-' }} 
              <span v-if="parseFloat(systemInfo?.thermal_temp) > 60">🔥</span>
              <span v-else-if="parseFloat(systemInfo?.thermal_temp) > 45">🌡️</span>
              <span v-else>❄️</span>
            </p>
          </div>
        </div>
        
        <div class="group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-gradient-to-br from-green-50 to-emerald-50 dark:from-green-500/10 dark:to-emerald-500/10 rounded-xl sm:rounded-2xl border border-green-200 dark:border-green-500/20 hover:border-green-500/50 transition-all hover:scale-105">
          <div class="absolute -top-2 -right-2 w-12 h-12 bg-green-500/20 rounded-full blur-xl group-hover:scale-150 transition-transform"></div>
          <div class="relative text-center">
            <div class="w-10 h-10 sm:w-12 sm:h-12 md:w-14 md:h-14 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-xl sm:rounded-2xl bg-gradient-to-br from-green-500 to-emerald-400 flex items-center justify-center shadow-lg shadow-green-500/30">
              <i class="fas fa-plug text-white text-sm sm:text-base md:text-xl"></i>
            </div>
            <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.power') }}</p>
            <p class="text-slate-800 dark:text-white font-bold text-base sm:text-lg md:text-xl">
              {{ systemInfo?.power_status || '-' }}
              <span>{{ systemInfo?.power_status?.includes('Charging') ? '⚡' : '🔌' }}</span>
            </p>
          </div>
        </div>
        
        <div class="group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-gradient-to-br from-blue-50 to-cyan-50 dark:from-blue-500/10 dark:to-cyan-500/10 rounded-xl sm:rounded-2xl border border-blue-200 dark:border-blue-500/20 hover:border-blue-500/50 transition-all hover:scale-105">
          <div class="absolute -top-2 -right-2 w-12 h-12 bg-blue-500/20 rounded-full blur-xl group-hover:scale-150 transition-transform"></div>
          <div class="relative text-center">
            <div class="w-10 h-10 sm:w-12 sm:h-12 md:w-14 md:h-14 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-xl sm:rounded-2xl bg-gradient-to-br from-blue-500 to-cyan-400 flex items-center justify-center shadow-lg shadow-blue-500/30">
              <i class="fas fa-heart-pulse text-white text-sm sm:text-base md:text-xl"></i>
            </div>
            <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.batteryHealth') }}</p>
            <p class="text-slate-800 dark:text-white font-bold text-base sm:text-lg md:text-xl">
              {{ systemInfo?.battery_health || '-' }}
              <span>{{ systemInfo?.battery_health === 'Good' ? '✅' : '⚠️' }}</span>
            </p>
          </div>
        </div>
        
        <div class="group relative overflow-hidden p-2 sm:p-3 md:p-4 bg-gradient-to-br from-purple-50 to-pink-50 dark:from-purple-500/10 dark:to-pink-500/10 rounded-xl sm:rounded-2xl border border-purple-200 dark:border-purple-500/20 hover:border-purple-500/50 transition-all hover:scale-105">
          <div class="absolute -top-2 -right-2 w-12 h-12 bg-purple-500/20 rounded-full blur-xl group-hover:scale-150 transition-transform"></div>
          <div class="relative text-center">
            <div class="w-10 h-10 sm:w-12 sm:h-12 md:w-14 md:h-14 mx-auto mb-1.5 sm:mb-2 md:mb-3 rounded-xl sm:rounded-2xl bg-gradient-to-br from-purple-500 to-pink-400 flex items-center justify-center shadow-lg shadow-purple-500/30">
              <i class="fas fa-battery-three-quarters text-white text-sm sm:text-base md:text-xl"></i>
            </div>
            <p class="text-slate-600 dark:text-white/50 text-[10px] sm:text-xs mb-0.5">{{ t('monitor.batteryCapacity') }}</p>
            <p class="text-slate-800 dark:text-white font-bold text-base sm:text-lg md:text-xl">
              {{ systemInfo?.battery_capacity || '-' }}% 
              <span>{{ systemInfo?.battery_capacity > 80 ? '🔋' : systemInfo?.battery_capacity > 20 ? '🪫' : '🆘' }}</span>
            </p>
          </div>
        </div>
      </div>
    </div>

 
   <!-- 系统详情 -->
    <div class="rounded-3xl bg-white dark:bg-white/5 backdrop-blur-xl border border-slate-200 dark:border-white/10 p-6">
      <div class="flex items-center space-x-3 mb-6">
        <div class="w-10 h-10 rounded-xl bg-gradient-to-br from-blue-500 to-indigo-500 flex items-center justify-center">
          <i class="fas fa-info-circle text-white"></i>
        </div>
        <h3 class="text-slate-800 dark:text-white font-bold">{{ t('monitor.systemDetails') }}</h3>
      </div>
      
      <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
        <div class="group p-4 bg-slate-100 dark:bg-white/5 rounded-2xl hover:bg-slate-200 dark:hover:bg-white/10 transition-all border border-slate-200 dark:border-white/5 hover:border-slate-300 dark:hover:border-white/20">
          <div class="flex items-center space-x-4">
            <div class="w-10 h-10 rounded-xl bg-gradient-to-br from-slate-600 to-slate-500 flex items-center justify-center">
              <i class="fas fa-desktop text-white"></i>
            </div>
            <div>
              <p class="text-slate-600 dark:text-white/50 text-xs">{{ t('monitor.systemName') }}</p>
              <p class="text-slate-800 dark:text-white font-medium">{{ systemInfo?.sysname || 'N/A' }}</p>
            </div>
          </div>
        </div>
        
        <div class="group p-4 bg-slate-100 dark:bg-white/5 rounded-2xl hover:bg-slate-200 dark:hover:bg-white/10 transition-all border border-slate-200 dark:border-white/5 hover:border-slate-300 dark:hover:border-white/20">
          <div class="flex items-center space-x-4">
            <div class="w-10 h-10 rounded-xl bg-gradient-to-br from-slate-600 to-slate-500 flex items-center justify-center">
              <i class="fas fa-code-branch text-white"></i>
            </div>
            <div>
              <p class="text-slate-600 dark:text-white/50 text-xs">{{ t('monitor.kernelVersion') }}</p>
              <p class="text-slate-800 dark:text-white font-medium text-sm">{{ systemInfo?.release || 'N/A' }}</p>
            </div>
          </div>
        </div>
        
        <div class="group p-4 bg-slate-100 dark:bg-white/5 rounded-2xl hover:bg-slate-200 dark:hover:bg-white/10 transition-all border border-slate-200 dark:border-white/5 hover:border-slate-300 dark:hover:border-white/20">
          <div class="flex items-center space-x-4">
            <div class="w-10 h-10 rounded-xl bg-gradient-to-br from-blue-600 to-cyan-500 flex items-center justify-center">
              <i class="fab fa-qq text-white"></i>
            </div>
            <div>
              <p class="text-slate-600 dark:text-white/50 text-xs">{{ t('monitor.qqGroup') }}</p>
              <p class="text-slate-800 dark:text-white font-mono text-sm">1029148488</p>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>
<style scoped>
@keyframes wave {
  0% { transform: translateX(0); }
  100% { transform: translateX(-50%); }
}
.animate-wave-slow {
  animation: wave 15s linear infinite;
}
.animate-wave-fast {
  animation: wave 10s linear infinite;
}
</style>
