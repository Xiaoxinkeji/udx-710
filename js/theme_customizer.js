/**
 * @name 主题定制
 * @id theme_customizer
 * @version 1.0.0
 * @author 小新科技
 * @description 自定义你的软件背景、主色调和元素透明度。让它独一无二。
 * @icon fa-palette
 * @color from-indigo-500 to-purple-600
 */

window.PLUGIN = {
    template: `
    <div class="theme-customizer space-y-4">
      <div class="bg-white/50 dark:bg-black/20 p-4 rounded-2xl border border-violet-200/50 backdrop-blur-md shadow-xl">
        <h3 class="text-violet-600 font-bold mb-4 flex items-center">
          <i class="fas fa-magic mr-2"></i>个性化设置
        </h3>
        
        <div class="space-y-4 text-sm">
          <!-- 背景图设置 -->
          <div class="space-y-2">
            <label class="block font-medium text-slate-700 dark:text-white/80">背景图片地址 (URL)</label>
            <div class="flex gap-2">
              <input v-model="bgUrl" type="text" placeholder="https://..." class="flex-1 bg-white/50 dark:bg-white/5 border border-slate-200 dark:border-white/10 rounded-lg px-3 py-1.5 focus:border-violet-400 outline-none">
              <button @click="applyBg" class="bg-violet-500 text-white px-4 py-1.5 rounded-lg">应用</button>
            </div>
            <p class="text-[10px] text-slate-500">提示: 留空启用默认背景。推荐使用高清壁纸外链。</p>
          </div>

          <!-- 主色调 -->
          <div class="space-y-2">
            <label class="block font-medium text-slate-700 dark:text-white/80">主色调 (Primary Color)</label>
            <div class="flex items-center gap-3">
              <input v-model="primaryColor" type="color" class="w-10 h-10 rounded cursor-pointer border-none bg-transparent">
              <span class="text-xs uppercase font-mono">{{ primaryColor }}</span>
              <button @click="applyTheme" class="ml-auto bg-slate-800 dark:bg-white text-white dark:text-slate-800 px-3 py-1.5 rounded-lg text-xs">应用主题</button>
            </div>
          </div>

          <!-- 透明度 -->
          <div class="space-y-2">
            <label class="block font-medium text-slate-700 dark:text-white/80">暗色模式卡片透明度 ({{ Math.round(opacity * 100) }}%)</label>
            <input v-model="opacity" type="range" min="0" max="1" step="0.05" class="w-full accent-violet-500">
          </div>

          <div class="pt-4 border-t border-slate-100 dark:border-white/10 flex justify-end gap-2">
            <button @click="resetDefaults" class="px-3 py-1.5 text-xs text-slate-500 hover:text-red-500 transition-colors">恢复默认</button>
            <button @click="saveSettings" class="px-4 py-1.5 bg-gradient-to-r from-violet-500 to-purple-500 text-white rounded-lg text-xs font-bold shadow-lg shadow-violet-500/20">永久保存</button>
          </div>
        </div>
      </div>
      
      <!-- 预览 -->
      <div class="p-4 rounded-xl border border-dashed border-violet-300 dark:border-violet-700 text-center">
        <p class="text-xs text-slate-400">设置将即时应用于当前窗口</p>
      </div>
    </div>
  `,
    data() {
        return {
            bgUrl: '',
            primaryColor: '#8b5cf6',
            opacity: 0.1
        }
    },
    methods: {
        async applyBg() {
            const url = this.$data.bgUrl;
            const app = document.getElementById('app');
            if (app) {
                if (url) {
                    app.style.backgroundImage = "url('" + url + "')";
                    app.style.backgroundSize = 'cover';
                    app.style.backgroundPosition = 'center';
                    app.style.backgroundAttachment = 'fixed';
                } else {
                    app.style.backgroundImage = '';
                }
            }
            this.$api.toast('背景应用成功');
        },
        async applyTheme() {
            const color = this.$data.primaryColor;
            const op = this.$data.opacity;

            const styleId = 'custom-theme-styles';
            let styleTag = document.getElementById(styleId);
            if (!styleTag) {
                styleTag = document.createElement('style');
                styleTag.id = styleId;
                document.head.appendChild(styleTag);
            }

            styleTag.innerHTML = `
        :root {
          --violet-500: ` + color + ` !important;
          --violet-600: ` + color + `dd !important;
        }
        .dark .bg-white\\\\/5 {
          background-color: rgba(255, 255, 255, ` + op + `) !important;
        }
      `;
            this.$api.toast('主题配色已生效');
        },
        async saveSettings() {
            const settings = {
                bgUrl: this.$data.bgUrl,
                primaryColor: this.$data.primaryColor,
                opacity: this.$data.opacity
            };
            await this.$api.storage.set('theme_settings', settings);
            this.$api.toast('配置已持久化保存', 'success');
        },
        async resetDefaults() {
            this.$data.bgUrl = '';
            this.$data.primaryColor = '#8b5cf6';
            this.$data.opacity = 0.1;

            const app = document.getElementById('app');
            if (app) app.style.backgroundImage = '';

            const styleTag = document.getElementById('custom-theme-styles');
            if (styleTag) styleTag.remove();

            await this.$api.storage.remove('theme_settings');
            this.$api.toast('已恢复默认设置', 'info');
            this.$refresh();
        }
    },
    async mounted() {
        const saved = await this.$api.storage.get('theme_settings');
        if (saved) {
            this.$data.bgUrl = saved.bgUrl || '';
            this.$data.primaryColor = saved.primaryColor || '#8b5cf6';
            this.$data.opacity = saved.opacity !== undefined ? saved.opacity : 0.1;

            if (this.applyBg) this.applyBg();
            if (this.applyTheme) this.applyTheme();

            this.$refresh();
        }
        this.$api.toast('主题定制插件就绪', 'info');
    }
};
