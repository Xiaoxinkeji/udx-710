<template>
  <div class="automation-manager space-y-6">
    <!-- Header -->
    <div class="flex flex-col sm:flex-row justify-between items-start sm:items-center bg-slate-800/50 backdrop-blur-xl p-6 rounded-3xl border border-white/10 shadow-xl gap-4">
      <div>
        <h2 class="text-2xl font-black text-white tracking-widest uppercase">自动化流引擎</h2>
        <p class="text-slate-400 text-xs mt-1 uppercase tracking-tighter">IF-THEN Logic Automation Engine</p>
      </div>
      <button @click="showAddModal = true" class="w-full sm:w-auto bg-blue-600 hover:bg-blue-500 text-white px-6 py-3 rounded-xl font-bold transition-all shadow-lg shadow-blue-500/30 flex items-center justify-center gap-2">
        <font-awesome-icon icon="plus" />
        新建规则
      </button>
    </div>

    <!-- Rules List -->
    <div v-if="rules.length === 0" class="text-center py-20 bg-slate-800/20 rounded-3xl border border-dashed border-white/10">
      <font-awesome-icon icon="robot" class="text-6xl text-slate-700 mb-4" />
      <p class="text-slate-500">尚无激活的自动化规则</p>
    </div>
    
    <div v-else class="grid grid-cols-1 md:grid-cols-2 gap-4">
      <div v-for="rule in rules" :key="rule.id" 
           class="group bg-slate-800/40 backdrop-blur-md p-5 rounded-2xl border border-white/5 hover:border-blue-500/30 transition-all relative overflow-hidden">
        <div class="flex justify-between items-start mb-4">
          <div class="flex items-center gap-3">
            <div :class="rule.enabled ? 'bg-blue-500/20 text-blue-400' : 'bg-slate-700/50 text-slate-500'" class="w-10 h-10 rounded-lg flex items-center justify-center transition-colors">
              <font-awesome-icon :icon="getTriggerIcon(rule.trigger)" />
            </div>
            <div>
              <h4 class="text-white font-bold">{{ rule.name }}</h4>
              <span class="text-[10px] text-slate-500 uppercase font-mono">ID: {{ rule.id }}</span>
            </div>
          </div>
          <div class="flex gap-2">
            <button @click="toggleRule(rule)" class="text-slate-400 hover:text-white transition-colors p-2" :title="rule.enabled ? '禁用' : '启用'">
              <font-awesome-icon :icon="rule.enabled ? 'toggle-on' : 'toggle-off'" class="text-xl" :class="rule.enabled ? 'text-blue-500' : ''" />
            </button>
            <button @click="deleteRule(rule.id)" class="text-slate-400 hover:text-red-400 transition-colors p-2">
              <font-awesome-icon icon="trash-alt" />
            </button>
          </div>
        </div>

        <div class="flex items-center gap-2 text-sm">
          <span class="bg-slate-700/50 px-2 py-1 rounded text-blue-300 font-mono text-[10px]">IF</span>
          <span class="text-slate-300">{{ formatTriggerName(rule.trigger) }}</span>
          <span class="text-blue-400 font-bold font-mono">{{ rule.operator }}</span>
          <span class="text-emerald-400 font-bold font-mono">{{ rule.value }}</span>
        </div>

        <div class="mt-3 flex items-center gap-2 text-sm">
          <span class="bg-indigo-500/20 px-2 py-1 rounded text-indigo-300 font-mono text-[10px]">THEN</span>
          <span class="text-slate-300 truncate">{{ formatActionName(rule.action) }}</span>
        </div>

        <!-- Background Accent -->
        <div class="absolute -right-4 -bottom-4 opacity-[0.03] text-8xl pointer-events-none group-hover:opacity-[0.06] transition-opacity">
          <font-awesome-icon :icon="getTriggerIcon(rule.trigger)" />
        </div>
      </div>
    </div>

    <!-- Add/Edit Modal -->
    <div v-if="showAddModal" class="fixed inset-0 z-[100] flex items-center justify-center p-4 backdrop-blur-md bg-black/60">
      <div class="bg-slate-900 border border-white/10 w-full max-w-lg rounded-3xl shadow-2xl overflow-hidden animate-in fade-in zoom-in duration-300">
        <div class="p-6 border-b border-white/5 flex justify-between items-center bg-slate-800/30">
          <h3 class="text-xl font-bold text-white">配置自动化逻辑</h3>
          <button @click="showAddModal = false" class="text-slate-400 hover:text-white"><font-awesome-icon icon="times" /></button>
        </div>
        
        <div class="p-6 space-y-4 max-h-[70vh] overflow-y-auto">
          <div>
            <label class="block text-xs uppercase text-slate-500 mb-1 ml-1">规则名称</label>
            <input v-model="form.name" type="text" placeholder="例如：高温自动重启" class="w-full bg-slate-800 border border-white/10 rounded-xl px-4 py-4 text-white focus:border-blue-500 focus:outline-none transition-all" />
          </div>

          <div class="grid grid-cols-1 sm:grid-cols-3 gap-3">
            <div class="sm:col-span-1">
              <label class="block text-xs uppercase text-slate-500 mb-1 ml-1">监控指标</label>
              <select v-model="form.trigger" class="w-full bg-slate-800 border border-white/10 rounded-xl px-4 py-4 text-white focus:border-blue-500 focus:outline-none">
                <option value="temperature">核心温度</option>
                <option value="uptime">运行时间</option>
                <option value="mem_percent">内存占用</option>
              </select>
            </div>
            <div class="sm:col-span-1">
              <label class="block text-xs uppercase text-slate-500 mb-1 ml-1">判定条件</label>
              <select v-model="form.operator" class="w-full bg-slate-800 border border-white/10 rounded-xl px-4 py-4 text-white focus:border-blue-500 focus:outline-none">
                <option value=">">&gt; 大于</option>
                <option value="<">&lt; 小于</option>
                <option value="==">== 等于</option>
                <option value=">=">&gt;= 大于等于</option>
                <option value="<=">&lt;= 小于等于</option>
              </select>
            </div>
            <div class="sm:col-span-1">
              <label class="block text-xs uppercase text-slate-500 mb-1 ml-1">阈值</label>
              <input v-model.number="form.value" type="number" class="w-full bg-slate-800 border border-white/10 rounded-xl px-4 py-4 text-white focus:border-blue-500 focus:outline-none" />
            </div>
          </div>

          <div>
            <label class="block text-xs uppercase text-slate-500 mb-1 ml-1">执行动作</label>
            <select v-model="form.actionType" class="w-full bg-slate-800 border border-white/10 rounded-xl px-4 py-4 text-white focus:border-blue-500 focus:outline-none mb-3">
              <option value="reboot">重启系统 (Reboot)</option>
              <option value="shell">自定义指令 (Shell)</option>
            </select>
            <div v-if="form.actionType === 'shell'" class="relative">
              <span class="absolute left-4 top-1/2 -translate-y-1/2 text-slate-500 font-mono text-sm">#</span>
              <input v-model="form.shellCmd" type="text" placeholder="例如: sh /path/to/script.sh" class="w-full bg-slate-800 border border-white/10 rounded-xl pl-8 pr-4 py-4 text-white focus:border-blue-500 focus:outline-none font-mono text-sm" />
            </div>
          </div>
        </div>

        <div class="p-6 bg-slate-800/50 border-t border-white/5 flex gap-3">
          <button @click="showAddModal = false" class="flex-1 px-4 py-4 rounded-xl border border-white/10 text-slate-400 hover:bg-white/5 transition-all">取消</button>
          <button @click="handleSave" class="flex-1 px-4 py-4 rounded-xl bg-blue-600 text-white font-bold hover:bg-blue-500 transition-all shadow-lg shadow-blue-500/20 active:scale-95">同步并应用</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { getAutomationRules, saveAutomationRule, deleteAutomationRule } from '../composables/useApi'
import { useToast } from '../composables/useToast'
import { useConfirm } from '../composables/useConfirm'

const { success, error } = useToast()
const { confirm } = useConfirm()

const rules = ref([])
const showAddModal = ref(false)

const form = ref({
  id: 0,
  name: '',
  trigger: 'temperature',
  operator: '>',
  value: 75,
  actionType: 'reboot',
  shellCmd: '',
  enabled: 1
})

const fetchRules = async () => {
  try {
    const data = await getAutomationRules()
    if (Array.isArray(data)) rules.value = data
  } catch (e) {
    error('获取规则库失败')
  }
}

const handleSave = async () => {
  if (!form.value.name) return error('请输入规则名称')
  if (form.value.actionType === 'shell' && !form.value.shellCmd) return error('请输入指令内容')
  
  const rule = {
    ...form.value,
    action: form.value.actionType === 'reboot' ? 'reboot' : `shell:${form.value.shellCmd}`
  }
  
  try {
    const res = await saveAutomationRule(rule)
    if (res.status === 'ok') {
      success('规则已激活')
      showAddModal.value = false
      resetForm()
      await fetchRules()
    }
  } catch (e) {
    error('同步失败')
  }
}

const deleteRule = async (id) => {
  if (await confirm('确定要移除此自动化逻辑吗？移除后将不再监控相关指标。')) {
    try {
      const res = await deleteAutomationRule(id)
      if (res.status === 'ok') {
        success('已从引擎移除')
        await fetchRules()
      }
    } catch (e) {
      error('移除失败')
    }
  }
}

const toggleRule = async (rule) => {
  const updated = { ...rule, enabled: rule.enabled ? 0 : 1 }
  try {
    const res = await saveAutomationRule(updated)
    if (res.status === 'ok') {
      await fetchRules()
    }
  } catch (e) {
    error('状态更新失败')
  }
}

const resetForm = () => {
  form.value = { id: 0, name: '', trigger: 'temperature', operator: '>', value: 75, actionType: 'reboot', shellCmd: '', enabled: 1 }
}

const getTriggerIcon = (t) => ({
  temperature: 'thermometer-half',
  uptime: 'clock',
  mem_percent: 'memory'
}[t] || 'cog')

const formatTriggerName = (t) => ({
  temperature: '核心温度',
  uptime: '在线时长 (min)',
  mem_percent: '内存利用率 (%)'
}[t] || t)

const formatActionName = (a) => {
  if (a === 'reboot') return '触发系统冷重启'
  if (a.startsWith('shell:')) return `执行自定义指令: ${a.substring(6)}`
  return a
}

onMounted(fetchRules)
</script>

<style scoped>
.animate-in {
  animation: modal-in 0.4s cubic-bezier(0.34, 1.56, 0.64, 1);
}
@keyframes modal-in {
  from { opacity: 0; transform: scale(0.9) translateY(20px); }
  to { opacity: 1; transform: scale(1) translateY(0); }
}

/* Custom scrollbar for better UX in dark mode */
::-webkit-scrollbar {
  width: 6px;
}
::-webkit-scrollbar-track {
  background: transparent;
}
::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.1);
  border-radius: 10px;
}
::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.2);
}
</style>
