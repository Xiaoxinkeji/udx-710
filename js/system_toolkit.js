/**
 * 示例插件：系统工具箱
 * 展示如何使用 $api 运行 Shell 命令并显示结果
 */
window.PLUGIN = {
    template: `
    <plugin-card title="系统工具箱" icon="fa-tools">
      <div class="space-y-4">
        <div class="grid grid-cols-2 gap-2">
          <plugin-btn @click="getUptime" icon="fa-clock" label="运行时间" />
          <plugin-btn @click="getMemInfo" icon="fa-memory" label="内存详情" />
          <plugin-btn @click="getDiskInfo" icon="fa-hdd" label="存储空间" />
          <plugin-btn @click="getProcess" icon="fa-tasks" label="进程视图" />
        </div>
        
        <div v-if="loading" class="flex justify-center py-4">
          <i class="fas fa-spinner animate-spin text-emerald-500"></i>
        </div>
        
        <div v-if="result" class="bg-slate-900 rounded-lg p-3 font-mono text-xs text-emerald-400 overflow-x-auto whitespace-pre">
          {{ result }}
        </div>
      </div>
    </plugin-card>
  `,
    data() {
        return {
            result: '',
            loading: false
        }
    },
    methods: {
        async getUptime() {
            this.loading = true;
            this.result = await this.$api.shell('uptime');
            this.loading = false;
        },
        async getMemInfo() {
            this.loading = true;
            this.result = await this.$api.shell('free -m');
            this.loading = false;
        },
        async getDiskInfo() {
            this.loading = true;
            this.result = await this.$api.shell('df -h /');
            this.loading = false;
        },
        async getProcess() {
            this.loading = true;
            this.result = await this.$api.shell('top -b -n 1 | head -n 20');
            this.loading = false;
        }
    }
};
