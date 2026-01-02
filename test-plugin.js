/**
 * 测试插件 - 用于验证插件系统
 * @version 1.0.0
 * @author 测试
 */
window.PLUGIN = {
  // ========== 元数据（必填） ==========
  name: '测试插件',
  version: '1.0.0',
  author: '测试作者',
  description: '这是一个测试插件，用于验证插件系统是否正常工作',

  // ========== 元数据（可选） ==========
  icon: 'fa-flask',
  color: 'from-purple-500 to-pink-400',

  // ========== 模板（必填） ==========
  template: `
    <div class="space-y-4">
      <plugin-card title="测试插件" icon="flask">
        <p class="text-gray-300 mb-4">{{ message }}</p>
        <plugin-btn type="primary" @click="testFunction">
          测试按钮
        </plugin-btn>
        <plugin-btn type="success" @click="showInfo">
          显示信息
        </plugin-btn>
      </plugin-card>
    </div>
  `,

  // ========== 数据（必填） ==========
  data() {
    return {
      message: '插件已加载，点击按钮测试功能',
      counter: 0
    }
  },

  // ========== 方法（必填） ==========
  methods: {
    async testFunction() {
      this.$data.counter++
      this.$data.message = `按钮被点击了 ${this.$data.counter} 次`
      this.$api.toast('测试成功！', 'success')
    },

    async showInfo() {
      const info = `插件名称: ${this.name}\n版本: ${this.version}\n作者: ${this.author}`
      this.$api.toast(info, 'info')
    }
  },

  // ========== 生命周期（可选） ==========
  mounted() {
    console.log('测试插件已挂载')
    this.$api.toast('测试插件加载成功', 'success')
  },

  destroyed() {
    console.log('测试插件已销毁')
  }
}
