<template>
  <div class="space-y-8 animate-in">
    <!-- 头部摘要 -->
    <div class="relative overflow-hidden rounded-3xl bg-gradient-to-br from-indigo-600 to-violet-700 p-8 text-white shadow-2xl shadow-indigo-500/30">
      <div class="absolute top-0 right-0 w-64 h-64 bg-white/10 rounded-full blur-3xl -translate-y-1/2 translate-x-1/2"></div>
      <div class="relative flex flex-col md:flex-row md:items-center justify-between gap-6">
        <div>
          <h2 class="text-3xl font-black tracking-tight mb-2">{{ t('achievements.title') }}</h2>
          <p class="text-indigo-100/80">{{ t('achievements.subtitle') }}</p>
        </div>
        <div class="flex items-center gap-4 bg-white/10 backdrop-blur-md px-6 py-4 rounded-2xl border border-white/20">
          <div class="text-center">
            <div class="text-2xl font-black">{{ totalAchieved }} / {{ achievements.length }}</div>
            <div class="text-[10px] uppercase tracking-widest opacity-60">Completed</div>
          </div>
          <div class="w-px h-8 bg-white/20"></div>
          <div class="text-center">
            <div class="text-2xl font-black text-yellow-400">{{ totalProgress }}%</div>
            <div class="text-[10px] uppercase tracking-widest opacity-60">Overall</div>
          </div>
        </div>
      </div>
    </div>

    <!-- 成就网格 -->
    <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
      <div
        v-for="achievement in achievements"
        :key="achievement.id"
        class="group relative"
      >
        <!-- 装饰背景 -->
        <div 
          class="absolute inset-0 bg-gradient-to-br opacity-0 group-hover:opacity-100 transition-opacity duration-500 rounded-3xl blur-xl"
          :class="achievement.color"
        ></div>

        <div 
          class="relative h-full overflow-hidden rounded-3xl border border-slate-200 dark:border-white/10 p-6 flex flex-col transition-all duration-300 hover:-translate-y-2 hover:shadow-2xl"
          :class="[
            achievement.achieved 
              ? 'bg-white/90 dark:bg-white/10 backdrop-blur-xl' 
              : 'bg-slate-50/50 dark:bg-black/20 opacity-80 grayscale-[0.5]'
          ]"
        >
          <!-- 解锁标记 -->
          <div 
            v-if="achievement.achieved"
            class="absolute -top-12 -right-12 w-24 h-24 bg-yellow-400 rotate-45 flex items-end justify-center pb-1 shadow-lg"
          >
            <i class="fas fa-check text-white text-xs mb-1"></i>
          </div>

          <div class="flex items-start justify-between mb-6">
            <div 
              class="w-14 h-14 rounded-2xl flex items-center justify-center text-2xl shadow-inner transition-transform group-hover:scale-110 duration-500"
              :class="[achievement.achieved ? 'bg-gradient-to-br ' + achievement.color + ' text-white shadow-lg' : 'bg-slate-200 dark:bg-white/5 text-slate-400']"
            >
              <i :class="['fas', achievement.icon]"></i>
            </div>
            <span 
              class="text-[10px] font-black uppercase tracking-widest px-2 py-1 rounded-full border"
              :class="achievement.achieved ? 'border-yellow-500/50 text-yellow-600 dark:text-yellow-400' : 'border-slate-300 dark:border-white/10 text-slate-400'"
            >
              {{ achievement.achieved ? 'Unlocked' : 'Locked' }}
            </span>
          </div>

          <div class="flex-1">
            <h3 
              class="text-xl font-black mb-2 transition-colors"
              :class="achievement.achieved ? 'text-slate-900 dark:text-white' : 'text-slate-500'"
            >
              {{ achievement.title }}
            </h3>
            <p class="text-slate-500 dark:text-white/60 text-sm leading-relaxed mb-6 font-medium">
              {{ achievement.description }}
            </p>
          </div>

          <!-- 进度条 -->
          <div class="mt-auto">
            <div class="flex justify-between items-center text-[10px] font-black uppercase tracking-tighter mb-2" :class="achievement.achieved ? 'text-indigo-500' : 'text-slate-400'">
              <span>Progress</span>
              <span>{{ achievement.progress }}%</span>
            </div>
            <div class="h-2 w-full bg-slate-200 dark:bg-white/5 rounded-full overflow-hidden">
              <div 
                class="h-full rounded-full transition-all duration-1000 ease-out shadow-[0_0_10px_rgba(0,0,0,0.1)]"
                :class="[achievement.achieved ? 'bg-gradient-to-r ' + achievement.color : 'bg-slate-400']"
                :style="{ width: achievement.progress + '%' }"
              >
                <div class="w-full h-full bg-white/20 animate-pulse"></div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { inject, computed, ref, onMounted, onUnmounted } from 'vue'
import { useI18n } from 'vue-i18n'
import { getAchievements } from '../composables/useApi'

const { t } = useI18n()
const realAchievements = ref([])

async function fetchAchievements() {
  try {
    const res = await getAchievements()
    if (res && Array.isArray(res)) {
      realAchievements.value = res
    }
  } catch (err) {
    console.error('Failed to fetch achievements:', err)
  }
}

let timer = null
onMounted(() => {
  fetchAchievements()
  timer = setInterval(fetchAchievements, 10000)
})

onUnmounted(() => {
  if (timer) clearInterval(timer)
})

const achievementDefinitions = computed(() => [
  {
    id: 'uptime_novice',
    title: t('achievements.uptimeNovice'),
    description: t('achievements.uptimeNoviceDesc'),
    icon: 'fa-clock',
    color: 'from-blue-400 to-cyan-500'
  },
  {
    id: 'traffic_master',
    title: t('achievements.trafficMaster'),
    description: t('achievements.trafficMasterDesc'),
    icon: 'fa-rocket',
    color: 'from-purple-500 to-indigo-600'
  },
  {
    id: 'signal_hunter',
    title: t('achievements.signalHunter'),
    description: t('achievements.signalHunterDesc'),
    icon: 'fa-satellite-dish',
    color: 'from-amber-400 to-orange-600'
  }
])

const achievements = computed(() => {
  return achievementDefinitions.value.map(def => {
    const real = realAchievements.value.find(a => a.id === def.id) || {}
    return {
      ...def,
      achieved: real.achieved || false,
      progress: real.progress || 0
    }
  })
})

const totalAchieved = computed(() => achievements.value.filter(a => a.achieved).length)
const totalProgress = computed(() => {
  if (achievements.value.length === 0) return 0
  const sum = achievements.value.reduce((acc, a) => acc + a.progress, 0)
  return Math.round(sum / achievements.value.length)
})
</script>

<style scoped>
.animate-in {
  animation: fadeIn 0.7s ease-out fill-mode-forwards;
}

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(20px); }
  to { opacity: 1; transform: translateY(0); }
}
</style>
