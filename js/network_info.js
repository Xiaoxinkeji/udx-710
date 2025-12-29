/**
 * @name NetworkInfo
 * @description 显示详细的网络连接信息，包括信号强度、频段、运营商等
 * @version 1.0.0
 * @author Xiaoxin
 */

const template = `
<div class="p-4">
  <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
    <div class="bg-slate-100 dark:bg-white/5 p-4 rounded-xl">
      <h3 class="font-bold mb-2">信号状态</h3>
      <div class="space-y-2">
        <div class="flex justify-between">
          <span class="text-slate-500">RSRP</span>
          <span class="font-mono">{{ info.rsrp || 'N/A' }} dBm</span>
        </div>
        <div class="flex justify-between">
          <span class="text-slate-500">RSRQ</span>
          <span class="font-mono">{{ info.rsrq || 'N/A' }} dB</span>
        </div>
        <div class="flex justify-between">
          <span class="text-slate-500">SINR</span>
          <span class="font-mono">{{ info.sinr || 'N/A' }} dB</span>
        </div>
      </div>
    </div>
    
    <div class="bg-slate-100 dark:bg-white/5 p-4 rounded-xl">
      <h3 class="font-bold mb-2">网络详情</h3>
      <div class="space-y-2">
        <div class="flex justify-between">
          <span class="text-slate-500">运营商</span>
          <span>{{ info.carrier || '未知' }}</span>
        </div>
        <div class="flex justify-between">
          <span class="text-slate-500">网络类型</span>
          <span>{{ info.network_type || 'N/A' }}</span>
        </div>
        <div class="flex justify-between">
          <span class="text-slate-500">频段</span>
          <span>{{ info.band || 'N/A' }}</span>
        </div>
      </div>
    </div>
  </div>
  
  <div class="mt-4 flex justify-end">
    <button @click="refresh" class="px-4 py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600 transition-colors">
      <i class="fas fa-sync-alt mr-2"></i>刷新
    </button>
  </div>
</div>
`

export default {
    template,
    setup() {
        const info = Vue.ref({})

        const refresh = async () => {
            try {
                const res = await $api.get('/api/info')
                if (res) {
                    info.value = res
                }
            } catch (e) {
                $api.toast.error('获取信息失败')
            }
        }

        Vue.onMounted(refresh)

        return {
            info,
            refresh
        }
    }
}
