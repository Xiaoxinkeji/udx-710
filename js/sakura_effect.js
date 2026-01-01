/**
 * @name 樱花飘落
 * @id sakura_effect
 * @version 1.0.0
 * @author 小新科技
 * @description 为你的面板添加唯美的樱花飘落视觉效果，高性能CSS实现。
 * @icon fa-snowflake
 * @color from-pink-400 to-rose-300
 */

window.PLUGIN = {
    template: `
    <div class="sakura-container">
      <div class="sakura-settings bg-white/50 dark:bg-black/20 p-4 rounded-xl backdrop-blur-sm border border-pink-200/50">
        <h3 class="text-pink-600 font-bold mb-2 flex items-center">
          <i class="fas fa-snowflake mr-2"></i>樱花飘落设置
        </h3>
        <p class="text-xs text-pink-500/70 mb-4">效果已自动开启，点击下方按钮调整</p>
        <div class="flex flex-wrap gap-2">
          <button @click="toggleEnabled" 
            :class="enabled ? 'bg-pink-500 text-white' : 'bg-slate-200 text-slate-500'"
            class="px-3 py-1.5 rounded-lg text-xs transition-all">
            {{ enabled ? '关闭效果' : '开启效果' }}
          </button>
          <button @click="refreshSakura" class="bg-white/80 dark:bg-white/10 text-pink-500 px-3 py-1.5 rounded-lg text-xs border border-pink-200">
            重置动画
          </button>
        </div>
      </div>
      
      <!-- 只有在开启时才渲染飘落元素 -->
      <div v-if="enabled" class="sakura-overlay">
        <div v-for="n in 30" :key="n" class="sakura" :style="getSakuraStyle(n)"></div>
      </div>
    </div>

    <style>
      .sakura-container {
        position: relative;
        z-index: 10;
      }
      .sakura-overlay {
        position: fixed;
        top: 0;
        left: 0;
        width: 100vw;
        height: 100vh;
        pointer-events: none;
        z-index: 9999;
        overflow: hidden;
      }
      .sakura {
        position: absolute;
        top: -10%;
        background: linear-gradient(to bottom right, #ffb7c5, #ff99aa);
        border-radius: 50% 0 50% 50%;
        opacity: 0.8;
        transform-origin: center;
        animation: sakura-fall linear infinite;
      }
      @keyframes sakura-fall {
        0% {
          transform: translateY(0) translateX(0) rotate(0deg) skewX(0);
          opacity: 0.8;
        }
        25% {
          transform: translateY(25vh) translateX(50px) rotate(90deg) skewX(20deg);
        }
        50% {
          transform: translateY(50vh) translateX(-50px) rotate(180deg) skewX(-20deg);
          opacity: 0.6;
        }
        75% {
          transform: translateY(75vh) translateX(50px) rotate(270deg) skewX(20deg);
        }
        100% {
          transform: translateY(110vh) translateX(-50px) rotate(360deg) skewX(-20deg);
          opacity: 0;
        }
      }
    </style>
  `,
    data() {
        return {
            enabled: true
        }
    },
    methods: {
        toggleEnabled() {
            this.$data.enabled = !this.$data.enabled;
            this.$api.toast(this.$data.enabled ? '樱花已盛开' : '樱花已凋落', 'success');
            this.$refresh();
        },
        refreshSakura() {
            this.$data.enabled = false;
            this.$refresh();
            setTimeout(() => {
                this.$data.enabled = true;
                this.$refresh();
                this.$api.toast('樱花已重置', 'info');
            }, 100);
        },
        getSakuraStyle(n) {
            const left = Math.random() * 100;
            const size = Math.random() * 10 + 5;
            const duration = Math.random() * 10 + 5;
            const delay = Math.random() * 10;
            return {
                left: left + '%',
                width: size + 'px',
                height: size + 'px',
                animationDuration: duration + 's',
                animationDelay: '-' + delay + 's'
            };
        }
    },
    mounted() {
        this.$api.toast('已加载樱花特效 (高性能版)', 'info');
    }
};
