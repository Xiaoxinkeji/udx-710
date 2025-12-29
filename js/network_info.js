/**
 * @name 网络详情
 * @version 1.0.0
 * @author 小新科技
 * @description 显示详细的网络连接信息，包括信号强度、频段、运营商等
 */

window.PLUGIN = {
  template: `
    <plugin-card title="网络详情" icon="fa-broadcast-tower">
      <div class="p-4 space-y-4">
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div class="bg-slate-100 dark:bg-white/5 p-4 rounded-xl">
            <h3 class="font-bold mb-3 text-slate-700 dark:text-white">信号状态</h3>
            <div class="space-y-2 text-sm">
              <div class="flex justify-between">
                <span class="text-slate-500 dark:text-white/60">信号强度</span>
                <span class="font-mono font-medium">{{ info.signal_strength || 'N/A' }}</span>
              </div>
              <div class="flex justify-between">
                <span class="text-slate-500 dark:text-white/60">网络类型</span>
                <span class="font-mono font-medium">{{ info.network_type || 'N/A' }}</span>
              </div>
              <div class="flex justify-between">
                <span class="text-slate-500 dark:text-white/60">频段</span>
                <span class="font-mono font-medium">{{ info.network_band || 'N/A' }}</span>
              </div>
            </div>
          </div>
          
          <div class="bg-slate-100 dark:bg-white/5 p-4 rounded-xl">
            <h3 class="font-bold mb-3 text-slate-700 dark:text-white">网络详情</h3>
            <div class="space-y-2 text-sm">
              <div class="flex justify-between">
                <span class="text-slate-500 dark:text-white/60">运营商</span>
                <span class="font-medium">{{ info.carrier || '未知' }}</span>
              </div>
              <div class="flex justify-between">
                <span class="text-slate-500 dark:text-white/60">IMEI</span>
                <span class="font-mono text-xs">{{ info.imei || 'N/A' }}</span>
              </div>
              <div class="flex justify-between">
                <span class="text-slate-500 dark:text-white/60">ICCID</span>
                <span class="font-mono text-xs">{{ info.iccid ? info.iccid.substring(0, 10) + '...' : 'N/A' }}</span>
              </div>
            </div>
          </div>
        </div>
        
        <div class="flex justify-end">
          <button @click="refresh" :disabled="loading" 
            class="px-4 py-2 bg-blue-500 hover:bg-blue-600 disabled:opacity-50 text-white rounded-lg transition-all flex items-center gap-2">
            <i :class="loading ? 'fa-spinner animate-spin' : 'fa-sync-alt'" class="fas"></i>
            刷新
          </button>
        </div>
      </div>
    </plugin-card>
  `,
  data() {
    return {
      info: {},
      loading: false
    };
  },
  methods: {
    async refresh() {
      this.loading = true;
      try {
        const res = await this.$api.get('/api/info');
        if (res) {
          this.$data.info = res;
        }
      } catch (e) {
        this.$api.toast('获取信息失败', 'error');
      } finally {
        this.loading = false;
      }
    }
  },
  mounted() {
    this.refresh();
  }
};
